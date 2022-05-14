import * as path from "https://deno.land/std@0.138.0/path/mod.ts";
import * as fs from "https://deno.land/std@0.138.0/fs/mod.ts";

const __filename = new URL('', import.meta.url).pathname;
const __dirname = path.resolve(new URL('.', import.meta.url).pathname);

const root_path = path.resolve(__dirname, "../");
const build_path = path.join(root_path, "/build_ninja");
const emscripten_path = path.join(root_path, "/emscripten");
// const tmp_path = path.join(path.resolve(root_path, "../"), "/.tmp");
const tmp_path = path.join(root_path, "/.tmp");
fs.emptyDirSync(tmp_path);

let execute_command = async function (p_command: string[], p_working_directory: string = "") {
    console.log(p_command);
    if (p_working_directory != "") {
        Deno.chdir(p_working_directory);
    }
    console.log(Deno.cwd());
    const l_command = Deno.run({ cmd: p_command });
    let l_status = await l_command.status();
    if (!l_status.success) {
        Deno.exit(1);
    }
};

let execute_command_with_output = async function (p_command: string[], p_working_directory: string = ""): Promise<string> {
    console.log(p_command);
    if (p_working_directory != "") {
        Deno.chdir(p_working_directory);
    }
    console.log(Deno.cwd());
    const l_command = Deno.run({ cmd: p_command, stdout: "piped" });
    let l_status = await l_command.status();
    if (!l_status.success) {
        Deno.exit(1);
    }

    return new TextDecoder().decode(await l_command.output());
};

class BuildConfig {
    BUILD_TYPE: string = "Debug";
    ENABLE_SAFETY_CHECKS: boolean = false;
    ENABLE_ADDRESS: boolean = false;
    ENABLE_UNDEFINED: boolean = false;
    WIN_HEADLESS: boolean = false;
    PLATFORM_WEBASSEMBLY: boolean = false;

    public constructor(init?: Partial<BuildConfig>) {
        Object.assign(this, init);
    };

    buildTypeParam(): string {
        return `CMAKE_BUILD_TYPE=${this.BUILD_TYPE}`;
    };

    enableSafetyChecksParam(): string {
        return `ENABLE_SAFETY_CHECKS=${this.ENABLE_SAFETY_CHECKS}`;
    };
    enableAddressParam(): string {
        return `ENABLE_ADDRESS=${this.ENABLE_ADDRESS}`;
    };
    enableUndefinedParam(): string {
        return `ENABLE_UNDEFINED=${this.ENABLE_UNDEFINED}`;
    };
    winHeadlessParam(): string {
        return `WIN_HEADLESS=${this.WIN_HEADLESS}`;
    };
    platformWebasseblyParam(): string {
        return `PLATFORM_WEBASSEMBLY=${this.PLATFORM_WEBASSEMBLY}`;
    };
};

let build_cmake_project = async function (p_target: string, p_config: BuildConfig) {
    await execute_command(["cmake", "-Bbuild_ninja", "-D", p_config.buildTypeParam(), "-D", "CMAKE_C_COMPILER=clang-10", "-D", "CMAKE_CXX_COMPILER=clang++-10", "-D", p_config.enableSafetyChecksParam(),
        "-D", p_config.enableAddressParam(), "-D", p_config.enableUndefinedParam(), "-D", p_config.winHeadlessParam(), "-D", p_config.platformWebasseblyParam(), "-H.", "-GNinja"], root_path);
    await execute_command(["cmake", "--build", ".", "--target", p_target], build_path);
};

let l_type = Deno.args[0];
if (l_type == "BUILD_TESTS") {
    fs.emptyDirSync(build_path);
    await build_cmake_project("TESTS", new BuildConfig({ ENABLE_ADDRESS: true, ENABLE_SAFETY_CHECKS: true, ENABLE_UNDEFINED: true, WIN_HEADLESS: true }));
    await execute_command(["./TESTS"], build_path);
}
else if (l_type == "BUILD_EMSCRIPTEN") {
    fs.emptyDirSync(build_path);
    await build_cmake_project("TEST_0", new BuildConfig({ BUILD_TYPE: "Release", PLATFORM_WEBASSEMBLY: true }));
    const ninja_build_out_dir = path.join(build_path, "/sandbox");

    await execute_command(["git", "clone", "https://github.com/ldalzotto/ldalzotto.github.io.git"], tmp_path);
    const github_page_path = path.join(tmp_path, "ldalzotto.github.io");

    const emscripten_out_dir = path.join(github_page_path, "/TEST_0");
    fs.emptyDirSync(emscripten_out_dir);
    //    fs.copySync(path.join(ninja_build_out_dir, "/TEST_0"), path.join(emscripten_out_dir, "main.js"));

    let l_js: string = await Deno.readTextFile(path.join(ninja_build_out_dir, "/TEST_0"));
    l_js = l_js.replace("TEST_0.wasm", "main.wasm");
    Deno.writeTextFile(path.join(emscripten_out_dir, "main.js"), l_js);

    fs.copySync(path.join(ninja_build_out_dir, "/TEST_0.wasm"), path.join(emscripten_out_dir, "main.wasm"));
    fs.copySync(path.join(emscripten_path, "/library.js"), path.join(emscripten_out_dir, "library.js"));
    fs.copySync(path.join(emscripten_path, "/main.html"), path.join(emscripten_out_dir, "main.html"));


    let l_last_commit_hash = execute_command_with_output(["git", "rev-parse", "HEAD"], root_path);
    await execute_command(["git", "config", "--global", "user.email", "\"loic.dalzotto@hotmail.fr\""], github_page_path);
    await execute_command(["git", "config", "--global", "user.name", "\"ldalzotto\""], github_page_path);
    await execute_command(["git", "add", ".", "-f"], github_page_path);
    await execute_command(["git", "commit", "-m", `Built from ${l_last_commit_hash}`], github_page_path);
    await execute_command(["git", "push", "origin", "master"], github_page_path);


    // "TEST_0.wasm"

    // fs.emptyDirSync(build_path);
}
else {
    console.log("NOTHING TO DO");
}