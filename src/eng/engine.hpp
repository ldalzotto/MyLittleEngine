#pragma once

#include "./tmp_renderer.hpp"
#include <eng/input.hpp>
#include <eng/window.hpp>
#include <rast/rast.hpp>
#include <ren/ren.hpp>

namespace eng {
struct engine {

  window::system m_window_system;

  input::system m_input_system;
  ren::ren_handle m_renderer;
  tmp_renderer m_tmp_scene;

  void allocate(ui16 p_window_width, ui16 p_window_height) {
    m_window_system.allocate();
    m_input_system.allocate();
    bgfx::init();
    m_renderer = ren::ren_handle_allocate();
    m_renderer.allocate();
    m_tmp_scene.allocate(m_renderer);

    m_window_system.open_window(
        m_window_system.create_window(p_window_width, p_window_height));
  };

  void free() {
    m_window_system.free();
    m_input_system.free();
    m_tmp_scene.free(m_renderer);
    m_renderer.free();
    ren::ren_handle_free(m_renderer);
    bgfx::shutdown();
  };

  ui8 update() {

    if (m_window_system.fetch_events()) {
      m_input_system.update(m_window_system.input_system_events());

      // TODO -> custom logic here ?

      m_tmp_scene.frame(m_renderer);
      m_renderer.frame();
      bgfx::frame();
      m_window_system.draw_window(0, m_tmp_scene.frame_view(m_renderer));
      return 1;
    }

    return 0;
  };
};

}; // namespace eng

// TODO -> remove this
#include <ren/impl/ren_impl.hpp>