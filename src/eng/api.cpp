
#include <eng/engine.hpp>

extern "C" {
eng::engine *engine_allocate() { return eng::engine_allocate(); };
void engine_free(eng::engine *p_engine) { eng::engine_free(p_engine); };
void engine_update(eng::engine *p_engine) { p_engine->update(); };

eng::window_handle engine_window_open(eng::engine *p_engine, ui32 p_width,
                                      ui32 p_height) {
  return p_engine->window_open(p_width, p_height);
};
void engine_window_close(eng::engine *p_engine, eng::window_handle p_window) {
  p_engine->window_close(p_window);
};
}
