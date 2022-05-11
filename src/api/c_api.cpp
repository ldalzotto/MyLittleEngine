#include <api/c_api.h>
#include <eng/engine.hpp>

extern "C" {

engine_handle engine_allocate() {
  eng::engine *l_engine = new eng::engine();
  l_engine->allocate();
  return engine_handle{.m_ptr = l_engine};
};

void engine_free(engine_handle p_engine) {
  eng::engine *l_engine = (eng::engine *)p_engine.m_ptr;
  l_engine->free();
  delete l_engine;
};

void engine_update(engine_handle p_engine) {
  eng::engine *l_engine = (eng::engine *)p_engine.m_ptr;
  l_engine->update();
};

window_handle engine_widow_open(engine_handle p_engine, unsigned short width,
                                unsigned short height) {
  eng::engine *l_engine = (eng::engine *)p_engine.m_ptr;
  return window_handle{.m_ptr = l_engine->window_open(width, height).m_idx};
};

void engine_window_close(engine_handle p_engine, window_handle p_window) {
  eng::engine *l_engine = (eng::engine *)p_engine.m_ptr;
  l_engine->window_close(eng::window_handle{.m_idx = p_window.m_ptr});
};
}