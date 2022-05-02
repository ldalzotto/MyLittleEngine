const engine_lib = Deno.dlopen("/media/loic/SSD/SoftwareProjects/Once/build/libAPI.so", {
    engine_allocate: {
        parameters:[],
        result: "pointer"
    },
    engine_free: {
        parameters: ["pointer"],
        result : "void"
    },
    engine_update: {
        parameters: ["pointer"],
        result: "void"
    },
    engine_window_open: {
        parameters: ["pointer", "u32", "u32"],
        result: "pointer"
    },
    engine_window_close: {
        parameters: ["pointer", "pointer"],
        result: "void"
    },
});

let l_engine : any = engine_lib.symbols.engine_allocate();
let l_window :any = engine_lib.symbols.engine_window_open(l_engine, 800,600);

for (let i =0; i < 5000; ++i)
{
    engine_lib.symbols.engine_update(l_engine);
}

engine_lib.symbols.engine_window_close(l_engine, l_window);
engine_lib.symbols.engine_free(l_engine);