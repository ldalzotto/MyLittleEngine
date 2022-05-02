#pragma once

#include <eng/input.hpp>
#include <eng/window.hpp>

namespace eng {
struct engine {

  container::vector<window_handle> m_windows;
  container::vector<win::events> m_native_events;

  input::system m_input_system;

  void allocate() {
    m_windows.allocate(0);
    m_native_events.allocate(0);
    m_input_system.allocate();
  };

  void free() {
    assert_debug(m_windows.count() == 0);
    assert_debug(m_native_events.count() == 0);
    m_windows.free();
    m_native_events.free();
    m_input_system.free();
  };

  void update() {
    window::fetch(m_native_events);
    auto &l_events = m_native_events.at(0).m_events;
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

inline static engine *engine_allocate() {
  engine *l_engine = new engine();
  l_engine->allocate();
  return l_engine;
};

inline static void engine_free(engine *p_engine) {
  p_engine->free();
  delete p_engine;
};

}; // namespace eng