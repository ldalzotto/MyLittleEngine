import * as path from "https://deno.land/std@0.138.0/path/mod.ts";
import * as fs from "https://deno.land/std@0.138.0/fs/mod.ts";

const __filename = new URL('', import.meta.url).pathname;
const __dirname = path.resolve(new URL('.', import.meta.url).pathname);

const root_path = path.resolve(__dirname, "../");
const build_path = path.join(root_path, "/build_ninja");
const emscripten_path = path.join(root_path, "/emscripten");
const tmp_path = path.join(root_path, "/.tmp");

class CommandResult {
    m_success: boolean = false;
    m_stdout: string = "";
};

let execute_command_unchecked = async function (p_command: string[], p_working_directory: string = "") {
    console.log(p_command);
    if (p_working_directory != "") {
        Deno.chdir(p_working_directory);
    }
    console.log(Deno.cwd());
    const l_command = Deno.run({ cmd: p_command, stdout: "piped" });
    let l_result = new CommandResult();
    l_result.m_success = (await l_command.status()).success;
    l_result.m_stdout = new TextDecoder().decode(await l_command.output());
    return l_result;
};


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

let run_cmake_project = async function (p_project_name: string, p_path: string) {
    await execute_command([`./${p_project_name}`], p_path);
};

let get_commit_message = async function (p_sha: string): Promise<string> {
    return execute_command_with_output(["git", "log", "--format=%B", "-n", "1", p_sha], root_path);
};

let get_commit_PR_number = async function (p_sha: string): Promise<string> {
    let l_message: string = await get_commit_message(p_sha);
    const l_search_text: string = "Merge pull request #";
    let l_last_index = l_message.indexOf(l_search_text);
    if (l_last_index != -1) {
        l_last_index += l_search_text.length;
        let l_end_index = l_message.indexOf(" ", l_last_index);
        return l_message.slice(l_last_index, l_end_index);
    }
    return "";
};

let build_emscripten = async function (p_project_name: string, p_subdirectories: string[], p_output_path: string, p_commit_sha: string) {
    fs.emptyDirSync(build_path);
    await build_cmake_project(p_project_name, new BuildConfig({ BUILD_TYPE: "Release", PLATFORM_WEBASSEMBLY: true }));
    const ninja_build_out_dir = path.join(build_path, "/" + p_subdirectories.join("/"));

    const emscripten_out_dir = path.join(p_output_path, `/${p_commit_sha}/`);
    fs.emptyDirSync(emscripten_out_dir);

    {
        let l_js: string = await Deno.readTextFile(path.join(ninja_build_out_dir, "/SANDBOX_MESH_VIZUALIZER"));
        l_js = l_js.replace(`${p_project_name}.wasm`, "main.wasm");
        Deno.writeTextFile(path.join(emscripten_out_dir, "main.js"), l_js);
    }


    fs.copySync(path.join(ninja_build_out_dir, `/${p_project_name}.wasm`), path.join(emscripten_out_dir, "main.wasm"));
    fs.copySync(path.join(emscripten_path, "/library.js"), path.join(emscripten_out_dir, "library.js"));

    {
        let l_pr_number = await get_commit_PR_number(p_commit_sha);
        let l_pr_url = "https://github.com/ldalzotto/MyLittleEngine/pull/" + l_pr_number;
        let l_html: string = await Deno.readTextFile(path.join(emscripten_path, "/main.html"));
        l_html = l_html.replace("${COMMIT_SHA}", p_commit_sha);
        l_html = l_html.replace("${COMMIT_URL}", `https://github.com/ldalzotto/MyLittleEngine/commit/${p_commit_sha}`);
        while (l_html.indexOf("${PULL_REQUEST_URL}") != -1) {
            l_html = l_html.replace("${PULL_REQUEST_URL}", l_pr_url);
        }
        Deno.writeTextFile(path.join(emscripten_out_dir, "main.html"), l_html);
    }
};


let l_type = Deno.args[0];
if (l_type == "BUILD_TESTS_DEBUG") {
    fs.emptyDirSync(tmp_path);
    fs.emptyDirSync(build_path);
    await build_cmake_project("TESTS", new BuildConfig({ ENABLE_ADDRESS: true, ENABLE_SAFETY_CHECKS: true, ENABLE_UNDEFINED: true, WIN_HEADLESS: true }));
    await run_cmake_project("TESTS", build_path);
}
else if (l_type == "BUILD_TESTS_RELEASE") {
    fs.emptyDirSync(tmp_path);
    fs.emptyDirSync(build_path);
    await build_cmake_project("TESTS", new BuildConfig({ BUILD_TYPE: "Release", ENABLE_ADDRESS: false, ENABLE_SAFETY_CHECKS: false, ENABLE_UNDEFINED: false, WIN_HEADLESS: true }));
    await run_cmake_project("TESTS", build_path);
}
else if (l_type == "BUILD_EMSCRIPTEN") {
    fs.emptyDirSync(tmp_path);
    fs.emptyDirSync(build_path);
    
    let l_last_commit_hash: string = await execute_command_with_output(["git", "rev-parse", "HEAD"], root_path);
    l_last_commit_hash = l_last_commit_hash.trim();

    await build_emscripten("SANDBOX_MESH_VIZUALIZER", ["sandbox"], build_path, l_last_commit_hash);
}
else if (l_type == "BUILD_PUBLISH_EMSCRIPTEN") {

    let l_last_commit_hash: string = await execute_command_with_output(["git", "rev-parse", "HEAD"], root_path);
    l_last_commit_hash = l_last_commit_hash.trim();

    fs.emptyDirSync(tmp_path);
    fs.emptyDirSync(build_path);
    await execute_command(["git", "clone", "https://github.com/ldalzotto/ldalzotto.github.io"], tmp_path);
    const github_page_path = path.join(tmp_path, "ldalzotto.github.io");
    await build_emscripten("SANDBOX_MESH_VIZUALIZER", ["sandbox"], github_page_path, l_last_commit_hash);

    let l_github_token: string = Deno.args[1];

    await execute_command(["git", "config", "--global", "user.email", "\"loic.dalzotto@hotmail.fr\""], github_page_path);
    await execute_command(["git", "config", "--global", "user.name", "\"ldalzotto\""], github_page_path);
    await execute_command(["git", "add", ".", "-f"], github_page_path);
    let l_commit_command: CommandResult = await execute_command_unchecked(["git", "commit", "-m", `Built from ${l_last_commit_hash}`], github_page_path);
    if (l_commit_command.m_success) {
        if (l_commit_command.m_stdout.indexOf("nothing to commit, working tree clean") == -1) {
            await execute_command(["git", "push", `https://${l_github_token}@github.com/ldalzotto/ldalzotto.github.io.git`], github_page_path);
        }
    }
}
else {
    console.log("NOTHING TO DO");
}