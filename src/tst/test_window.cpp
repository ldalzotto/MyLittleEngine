#include <doctest.h>

#include <eng/engine.hpp>

TEST_CASE("window_system.input") {
  eng::window::system l_window_system;
  l_window_system.allocate();

  REQUIRE(l_window_system.input_system_events().count() == 0);

  eng::window_handle l_window = l_window_system.create_window(800, 800);
  l_window_system.open_window(l_window);

  {
    win::event l_debug_input_event;
    l_debug_input_event.m_type = win::event::type::InputPress;
    l_debug_input_event.m_input.m_key = eng::input::Key::ARROW_UP;
    win::debug_simulate_event(l_window.m_idx, l_debug_input_event);

    REQUIRE(l_window_system.fetch_events());
    REQUIRE(l_window_system.input_system_events().count() == 1);
    auto l_input_system_events = l_window_system.input_system_events();
    REQUIRE(l_input_system_events.count() == 1);
    REQUIRE(l_input_system_events.at(0).m_key ==
            l_debug_input_event.m_input.m_key);
    REQUIRE(l_input_system_events.at(0).m_flag ==
            eng::input::Event::Flag::PRESSED);
  }

  {
    win::event l_debug_input_event;
    l_debug_input_event.m_type = win::event::type::InputRelease;
    l_debug_input_event.m_input.m_key = eng::input::Key::ARROW_DOWN;
    win::debug_simulate_event(l_window.m_idx, l_debug_input_event);

    REQUIRE(l_window_system.fetch_events());
    REQUIRE(l_window_system.input_system_events().count() == 1);
    auto l_input_system_events = l_window_system.input_system_events();
    REQUIRE(l_input_system_events.count() == 1);
    REQUIRE(l_input_system_events.at(0).m_key ==
            l_debug_input_event.m_input.m_key);
    REQUIRE(l_input_system_events.at(0).m_flag ==
            eng::input::Event::Flag::RELEASED);
  }

  l_window_system.close_window(l_window);

  l_window_system.free();
}

#include <sys/sys_impl.hpp>
#include <sys/win_impl.hpp>