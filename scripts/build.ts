import * as path from "https://deno.land/std@0.138.0/path/mod.ts";
import * as fs from "https://deno.land/std@0.138.0/fs/mod.ts";

const __filename = new URL('', import.meta.url).pathname;
const __dirname = path.resolve(new URL('.', import.meta.url).pathname);

const root_path = path.resolve(__dirname, "../");
const build_path = path.join(root_path, "/build_ninja");

let execute_command = async function (p_command: string[], p_working_directory: string = "") {
    console.log(p_command);
    if (p_working_directory != "") {
        Deno.chdir(p_working_directory);
    }
    const l_command = Deno.run({ cmd: p_command });
    let l_status = await l_command.status();
    if (!l_status.success) {
        Deno.exit(1);
    }
};

fs.emptyDirSync(build_path);
await execute_command(["cmake", "-Bbuild_ninja", "-D", "CMAKE_C_COMPILER=clang-10", "-D", "CMAKE_CXX_COMPILER=clang++-10", "-D", "ENABLE_SAFETY_CHECKS=true",
    "-D", "ENABLE_ADDRESS=true", "-D", "ENABLE_UNDEFINED=true", "-H.", "-GNinja"], root_path);
await execute_command(["cmake", "--build", ".", "--target", "TESTS"], build_path);
await execute_command(["./TESTS"], build_path);
