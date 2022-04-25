#pragma once

#include <eng/input.hpp>
#include <sys/win.hpp>

namespace {
struct window_tests {
  inline static void nominal() {
    engine::input::system l_input;
    l_input.allocate();

    container::vector<win::events> l_win_events;
    l_win_events.allocate(0);
    void *l_windwo = win::create_window(100, 100);

    win::events l_events;
    l_events.allocate();
    l_events.m_window = l_windwo;
    l_win_events.push_back(l_events);

    win::show_window(l_windwo);
    {
      win::fetch_events(l_win_events);
      auto &l_events = l_win_events.at(0).m_events;
      for (auto i = 0; i < l_events.count(); ++i) {
        auto &l_event = l_events.at(i);

        if (l_event.m_type == win::event::type::InputPress) {
          engine::input::Event l_input_event;
          l_input_event.m_key = l_event.m_input.m_key;
          l_input_event.m_flag = engine::input::Event::Flag::PRESSED;
          l_input.push_event(l_input_event);
        } else if (l_event.m_type == win::event::type::InputRelease) {

          engine::input::Event l_input_event;
          l_input_event.m_key = l_event.m_input.m_key;
          l_input_event.m_flag = engine::input::Event::Flag::RELEASED;
          l_input.push_event(l_input_event);
        }
      }

      l_input.update();
    }
    win::close_window(l_windwo);

    for (auto i = 0; i < l_win_events.count(); ++i) {
      l_win_events.at(i).m_events.free();
    }
    l_win_events.free();

    l_input.free();
  }
};
}; // namespace

inline static void test_window() { window_tests::nominal(); };