
#include <api/c_api.h>

#if PLATFORM_WEBASSEMBLY_PREPROCESS

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#else

#define EMSCRIPTEN_KEEPALIVE

#endif

inline static engine_handle s_engine;

extern "C" {
EMSCRIPTEN_KEEPALIVE
void initialize() { s_engine = engine_allocate(800, 800); };
EMSCRIPTEN_KEEPALIVE
unsigned char main_loop() { return engine_update(s_engine); };
EMSCRIPTEN_KEEPALIVE
void terminate() { engine_free(s_engine); };
}

#if !PLATFORM_WEBASSEMBLY_PREPROCESS
int main() {
  initialize();
  while (main_loop()) {
  };
  terminate();
};
#endif
