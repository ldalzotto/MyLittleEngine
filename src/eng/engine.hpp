#pragma once

#include <eng/input.hpp>
#include <eng/tmp_renderer.hpp>
#include <eng/window.hpp>
#include <rast/rast.hpp>

namespace eng {
struct engine {

  container::vector<window_handle> m_windows;
  container::vector<window_image_buffer> m_window_image;
  container::vector<win::events> m_native_events;
  tmp_renderer m_tmp_renderer;

  input::system m_input_system;

  void allocate() {
    m_windows.allocate(0);
    m_native_events.allocate(0);
    m_window_image.allocate(0);
    m_input_system.allocate();
    bgfx::init();
    m_tmp_renderer.allocate();
    m_tmp_renderer.draw();
  };

  void free() {
    assert_debug(m_windows.count() == 0);
    assert_debug(m_native_events.count() == 0);
    assert_debug(m_window_image.count() == 0);
    m_windows.free();
    m_native_events.free();
    m_input_system.free();
    m_window_image.free();
    m_tmp_renderer.free();
    bgfx::shutdown();
  };

  ui8 update() {
    window::fetch(m_native_events);
    ui8 l_window_index = 0;
    window_handle l_window = m_windows.at(l_window_index);
    auto &l_win_events = m_native_events.at(l_window_index);
    auto &l_events = l_win_events.m_events;
    window_image_buffer &l_image_buffer = m_window_image.at(0);
    for (auto i = 0; i < l_events.count(); ++i) {
      auto &l_event = l_events.at(i);
      if (l_event.m_type == win::event::type::InputPress) {
        eng::input::Event l_input_event;
        l_input_event.m_key = l_event.m_input.m_key;
        l_input_event.m_flag = eng::input::Event::Flag::PRESSED;
        m_input_system.push_event(l_input_event);
      } else if (l_event.m_type == win::event::type::InputRelease) {
        eng::input::Event l_input_event;
        l_input_event.m_key = l_event.m_input.m_key;
        l_input_event.m_flag = eng::input::Event::Flag::RELEASED;
        m_input_system.push_event(l_input_event);
      } else if (l_event.m_type == win::event::type::Redraw) {
        if (l_event.m_draw.m_width != l_image_buffer.m_width &&
            l_event.m_draw.m_height != l_image_buffer.m_height) {
          l_image_buffer.free();
          l_image_buffer.allocate(l_window, l_event.m_draw.m_width,
                                  l_event.m_draw.m_height);
        }

        rast::image_view l_frame_buffer = m_tmp_renderer.frame_view();
        rast::image_copy_stretch(
            (m::vec<ui8, 3> *)l_frame_buffer.m_buffer.m_begin,
            l_frame_buffer.m_width, l_frame_buffer.m_height,
            (m::vec<ui8, 4> *)l_image_buffer.m_data.m_data,
            l_image_buffer.m_width, l_image_buffer.m_height);

        win::draw(l_win_events.m_window, l_image_buffer.m_native,
                  l_event.m_draw.m_width, l_event.m_draw.m_height);
      } else if (l_event.m_type == win::event::type::Close) {
        window_close(l_window);
      }
    }
    l_events.clear();

    if (m_windows.count() == 0) {
      return 0;
    }

    m_input_system.update();

    return 1;
  };

  window_handle window_open(ui32 p_width, ui32 p_height) {
    window_handle l_window = window::open(p_width, p_height);
    m_windows.push_back(l_window);
    win::events l_native_event;
    l_native_event.allocate();
    l_native_event.m_window = l_window.m_idx;
    m_native_events.push_back(l_native_event);

    window_image_buffer l_image_buffer;
    l_image_buffer.allocate(l_window, 0, 0);
    m_window_image.push_back(l_image_buffer);

    return l_window;
  };

  void window_close(window_handle p_window) {
    __remove_window(p_window);
    __remove_native_events(p_window);
  };

private:
  void __remove_window(window_handle p_window) {
    for (auto i = 0; i < m_windows.count(); ++i) {
      if (m_windows.at(i).m_idx == p_window.m_idx) {
        m_windows.remove_at(i);
        m_window_image.at(i).free();
        m_window_image.remove_at(i);
        return;
      }
    }
    assert_debug(false);
  };

  void __remove_native_events(window_handle p_window) {
    for (auto i = 0; i < m_native_events.count(); ++i) {
      auto &l_win_native_events = m_native_events.at(i);
      if (l_win_native_events.m_window == p_window.m_idx) {
        l_win_native_events.free();
        m_native_events.remove_at(i);
        return;
      }
    }
    assert_debug(false);
  };
};

}; // namespace eng