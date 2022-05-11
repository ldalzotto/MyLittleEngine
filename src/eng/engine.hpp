#pragma once

#include <eng/input.hpp>
#include <eng/window.hpp>
#include <rast/rast.hpp>

namespace eng {
struct engine {

  container::vector<window_handle> m_windows;
  container::vector<window_image_buffer> m_window_image;
  container::vector<win::events> m_native_events;

  input::system m_input_system;

  void allocate() {
    m_windows.allocate(0);
    m_native_events.allocate(0);
    m_window_image.allocate(0);
    m_input_system.allocate();
    bgfx::Init();

    bgfx::createFrameBuffer();
    
  };

  void free() {
    assert_debug(m_windows.count() == 0);
    assert_debug(m_native_events.count() == 0);
    m_windows.free();
    m_native_events.free();
    m_input_system.free();
    m_window_image.free();
  };

  void update() {
    window::fetch(m_native_events);
    auto &l_win_events = m_native_events.at(0);
    auto &l_events = l_win_events.m_events;
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
        /*
        m::vec<ui8, 3> *l_data = (m::vec<ui8, 3> *)m_window_image.at(0).m_data;
        for (auto i = 0; i < 10000; ++i) {
          l_data[i] = {255, 0, 0};
        }
        */
        /*
        ui8 *l_data = m_window_image.at(0).m_data;
        for (auto i = 0; i < 800 * 800; ++i) {
          m::vec<ui8, 3> *l_pixel = (m::vec<ui8, 3> *)l_data;
          *l_pixel = {0, 0, 255};
          l_data += sizeof(ui8) * 4;
        }
        */
        win::draw(l_win_events.m_window, m_window_image.at(0).m_native,
                  l_event.m_draw.m_width, l_event.m_draw.m_height);
        /*
        ui8 *l_buffer = (ui8 *)l_event.m_draw.m_buffer;
        for (auto j = 0; j < l_event.m_draw.m_height; j++) {
          for (auto i = 0; i < l_event.m_draw.m_width; i++) {
            l_buffer[i + (j * l_event.m_draw.m_width)] = 0;
          }
        }
        */
      }
    }

    m_input_system.update();
  };

  window_handle window_open(ui32 p_width, ui32 p_height) {
    window_handle l_window = window::open(p_width, p_height);
    m_windows.push_back(l_window);
    win::events l_native_event;
    l_native_event.allocate();
    l_native_event.m_window = l_window.m_idx;
    m_native_events.push_back(l_native_event);

    window_image_buffer l_image_buffer;
    l_image_buffer.allocate(l_window.m_idx, p_width, p_height);
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