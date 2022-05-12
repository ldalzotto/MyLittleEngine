#pragma once

#include <eng/input.hpp>
#include <eng/tmp_renderer.hpp>
#include <eng/window.hpp>
#include <rast/rast.hpp>

namespace eng {
struct engine {

  window::system m_window_system;
  tmp_renderer m_tmp_renderer;

  input::system m_input_system;

  void allocate(ui16 p_window_width, ui16 p_window_height) {
    m_window_system.allocate();
    m_input_system.allocate();
    bgfx::init();
    m_tmp_renderer.allocate();
    m_tmp_renderer.draw();

    m_window_system.open_window(
        m_window_system.create_window(p_window_width, p_window_height));
  };

  void free() {
    m_window_system.free();
    m_input_system.free();
    m_tmp_renderer.free();
    bgfx::shutdown();
  };

  ui8 update() {

    if (m_window_system.fetch_events()) {
      m_input_system.update(m_window_system.input_system_events());
      m_window_system.draw_window(0, m_tmp_renderer.frame_view());

      return 1;
    }

    return 0;
  };
};

}; // namespace eng