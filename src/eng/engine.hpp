#pragma once

#include <eng/input.hpp>
#include <eng/window.hpp>
#include <rast/rast.hpp>
#include <ren/ren.hpp>

namespace eng {

template <typename EngineImpl> struct engine_api {
  EngineImpl &thiz;
  engine_api(EngineImpl &p_thiz) : thiz(p_thiz){};

  FORCE_INLINE void allocate(ui16 p_window_width, ui16 p_window_height) {
    thiz.allocate(p_window_width, p_window_height);
  };
  FORCE_INLINE void free() { thiz.free(); };
  template <typename UpdateCallback>
  FORCE_INLINE void update(const UpdateCallback &p_update) {
    thiz.update(p_update);
  };
  FORCE_INLINE typename EngineImpl::ren_impl_t &renderer() {
    return thiz.m_renderer;
  };
  FORCE_INLINE typename EngineImpl::rast_impl_t &rasterizer() {
    return thiz.m_rasterizer;
  };
  FORCE_INLINE input::system &input() { return thiz.m_input_system; };
};

namespace details {
template <typename RenImpl, typename RastImpl> struct engine {

  using ren_impl_t = RenImpl;
  using rast_impl_t = RastImpl;

  window::system m_window_system;

  input::system m_input_system;

  ren_impl_t m_renderer;
  rast_impl_t m_rasterizer;

  window_handle m_window;

  void allocate(ui16 p_window_width, ui16 p_window_height) {
    m_window_system.allocate();
    m_input_system.allocate();

    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    l_rast.init();
    l_renderer.allocate();

    m_window = m_window_system.create_window(p_window_width, p_window_height);
    m_window_system.open_window(m_window);
  };

  void free() {

    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    m_window_system.close_window(m_window);

    m_window_system.free();
    m_input_system.free();
    l_renderer.free();
    l_rast.shutdown();
  };

  template <typename UpdateCallback>
  void update(const UpdateCallback &p_update) {
    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    m_window_system.fetch_events();
    m_input_system.update(m_window_system.input_system_events());

    p_update();

    l_renderer.frame(l_rast);
    l_rast.frame();
    m_window_system.draw_window(
        0, m_renderer.frame_view(ren::camera_handle{.m_idx = 0}, l_rast));
  };
};
}; // namespace details

}; // namespace eng

// TODO -> remove this
#include <ren/impl/ren_impl.hpp>