#pragma once

#include <eng/input.hpp>
#include <eng/time.hpp>
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
  FORCE_INLINE void update(fix32 p_delta, const UpdateCallback &p_update) {
    thiz.update(p_delta, p_update);
  };
  FORCE_INLINE typename EngineImpl::ren_impl_t &renderer() {
    return thiz.m_renderer;
  };
  FORCE_INLINE typename ren::ren_api<typename EngineImpl::ren_impl_t>
  renderer_api() {
    return ren::ren_api<typename EngineImpl::ren_impl_t>{renderer()};
  };
  FORCE_INLINE typename EngineImpl::rast_impl_t &rasterizer() {
    return thiz.m_rasterizer;
  };
  FORCE_INLINE rast_api<typename EngineImpl::rast_impl_t> rasterizer_api() {
    return rast_api<typename EngineImpl::rast_impl_t>{rasterizer()};
  };

  FORCE_INLINE input::system &input() { return thiz.m_input_system; };
  FORCE_INLINE window::system &window_system() { return thiz.m_window_system; };
  FORCE_INLINE time &time() { return thiz.m_time; };
};

namespace details {
template <typename RenImpl, typename RastImpl> struct engine {

  using ren_impl_t = RenImpl;
  using rast_impl_t = RastImpl;

  window::system m_window_system;

  input::system m_input_system;

  ren_impl_t m_renderer;
  rast_impl_t m_rasterizer;
  time m_time;

  window_handle m_window;

  void allocate(ui16 p_window_width, ui16 p_window_height) {
    m_window_system.allocate();
    m_input_system.allocate();

    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    l_rast.init();
    l_renderer.allocate();

    m_time.allocate();

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
  void update(fix32 p_delta, const UpdateCallback &p_update) {

    m_time.increment(p_delta);
    m_window_system.fetch_events();

    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    m_input_system.update(m_window_system.input_system_events());

    p_update();

    l_renderer.frame(l_rast);
    l_rast.frame();

    rast::image l_rendereed_frame =
        m_renderer.frame_view(ren::camera_handle{.m_idx = 0}, l_rast);
    m_window_system.draw_window(m_window, l_rendereed_frame);
  };
};
}; // namespace details

}; // namespace eng
