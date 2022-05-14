
#include <api/c_api.h>
#if 1
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif
inline static engine_handle s_engine;

#if 1
extern "C" {
EMSCRIPTEN_KEEPALIVE
void initialize() { s_engine = engine_allocate(800, 800); };
EMSCRIPTEN_KEEPALIVE
unsigned char main_loop() { return engine_update(s_engine); };
EMSCRIPTEN_KEEPALIVE
void terminate() { engine_free(s_engine); };
}
#endif

#if 0
// void single_up
EM_BOOL one_iter(double time, void *userData) {
  // Can render to the screen here, etc.

  // Return true to keep the loop running.
  return engine_update(engine_handle{.m_ptr = userData});
}

int main() {
  engine_handle l_engine = engine_allocate(800, 800);

  emscripten_request_animation_frame_loop(one_iter, l_engine.m_ptr);
  /*
    while (engine_update(l_engine)) {

      emscripten_sleep(16);
    }
    */
  // engine_free(l_engine);
};
#endif

#if 0
int main() {
  engine_handle l_engine = engine_allocate(800,800);
  while (engine_update(l_engine)) {
  }
  engine_free(l_engine);
};
#endif