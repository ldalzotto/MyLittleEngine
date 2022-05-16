#include <api/c_api.h>
#include <eng/engine.hpp>

inline bgfx_impl s_bgfx_impl = bgfx_impl();

extern "C" {

engine_handle engine_allocate(unsigned short width, unsigned short height) {
  eng::engine *l_engine = new eng::engine();
  l_engine->allocate(width, height);
  return engine_handle{.m_ptr = l_engine};
};

void engine_free(engine_handle p_engine) {
  eng::engine *l_engine = (eng::engine *)p_engine.m_ptr;
  l_engine->free();
  delete l_engine;
};

unsigned char engine_update(engine_handle p_engine) {
  eng::engine *l_engine = (eng::engine *)p_engine.m_ptr;
  return l_engine->update();
};
}

#include <sys/sys_impl.hpp>
#include <sys/win_impl.hpp>