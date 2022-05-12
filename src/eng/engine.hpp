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

  // TODO -> having an additional internal storage instead of callbacks ?
  ui8 update() {

    if ((m_window_system.fetch_events(
            [&](input::Event &p_event) { m_input_system.push_event(p_event); },
            [&](window_handle p_window, window_image_buffer &p_window_buffer) {
              rast::image_view l_frame_buffer = m_tmp_renderer.frame_view();
              rast::image_copy_stretch(
                  (m::vec<ui8, 3> *)l_frame_buffer.m_buffer.m_begin,
                  l_frame_buffer.m_width, l_frame_buffer.m_height,
                  (m::vec<ui8, 4> *)p_window_buffer.m_data.m_data,
                  p_window_buffer.m_width, p_window_buffer.m_height);

              win::draw(p_window.m_idx, p_window_buffer.m_native,
                        p_window_buffer.m_width, p_window_buffer.m_height);
            }))) {
      m_input_system.update();
      return 1;
    }

    return 0;
  };
};

}; // namespace eng