#pragma once

#include <eng/input.hpp>
#include <eng/window.hpp>
#include <rast/rast.hpp>
#include <ren/ren.hpp>

namespace eng {

template <typename EngineImpl> struct engine_api {
  using ren_api_t = ren::ren_api<typename EngineImpl::ren_impl_t>;
  using rast_api_t = rast_api<typename EngineImpl::rast_impl_t>;
  EngineImpl &thiz;
  engine_api(EngineImpl &p_thiz) : thiz(p_thiz){};

  FORCE_INLINE void allocate(ui16 p_window_width, ui16 p_window_height) {
    thiz.allocate(p_window_width, p_window_height);
  };
  FORCE_INLINE void free() { thiz.free(); };
  template <typename UpdateCallback>
  FORCE_INLINE ui8 update(const UpdateCallback &p_update) {
    return thiz.update(p_update);
  };
  FORCE_INLINE ren_api_t renderer() { return thiz.renderer(); };
  FORCE_INLINE rast_api_t rasterizer() { return thiz.rasterizer(); };
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

  FORCE_INLINE ren::ren_api<ren_impl_t> renderer() {
    return ren::ren_api<ren_impl_t>(m_renderer);
  };

  FORCE_INLINE rast_api<rast_impl_t> rasterizer() {
    return rast_api<rast_impl_t>(m_rasterizer);
  };

  void allocate(ui16 p_window_width, ui16 p_window_height) {
    m_window_system.allocate();
    m_input_system.allocate();

    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    l_rast.init();
    l_renderer.allocate();

    m_window_system.open_window(
        m_window_system.create_window(p_window_width, p_window_height));
  };

  void free() {

    api_decltype(ren::ren_api, l_renderer, m_renderer);
    api_decltype(rast_api, l_rast, m_rasterizer);

    m_window_system.free();
    m_input_system.free();
    l_renderer.free();
    l_rast.shutdown();
  };

  template <typename UpdateCallback>
  ui8 update(const UpdateCallback &p_update) {
    if (m_window_system.fetch_events()) {

      api_decltype(ren::ren_api, l_renderer, m_renderer);
      api_decltype(rast_api, l_rast, m_rasterizer);

      m_input_system.update(m_window_system.input_system_events());

      p_update();

      l_renderer.frame(l_rast);
      l_rast.frame();
      m_window_system.draw_window(
          0, m_renderer.frame_view(ren::camera_handle{.m_idx = 0}, l_rast));
      return 1;
    }

    return 0;
  };
};
}; // namespace details

}; // namespace eng

// TODO -> remove this
#include <ren/impl/ren_impl.hpp>