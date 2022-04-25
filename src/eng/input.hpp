#pragma once

#include <cor/container.hpp>
#include <cor/orm.hpp>

namespace engine {

namespace input {

enum class Key {
  UNDEFINED = 0,
  ARROW_DOWN = 1,
  ARROW_UP = 2,
  ARROW_LEFT = 3,
  ARROW_RIGHT = 4,
  ENUM_MAX = 5
};

enum class State {
  UNDEFINED = 0,
  JUST_PRESSED = 1,
  PRESSED = 2,
  JUST_RELEASED = 3
};

struct Event {
  Key m_key;
  enum class Flag { UNDEFINED = 0, PRESSED = 1, RELEASED = 2 } m_flag;
};

struct system {

  struct heap {
    struct state_table {
      table_vector_meta;
      table_cols_1(State);
      table_define_vector_1;
    } m_state_table;

    container::vector<uimax> m_just_pressed_keys;
    container::vector<uimax> m_just_released_keys;
    container::vector<Event> m_input_events;

    void allocate() {
      m_state_table.allocate((uimax)Key::ENUM_MAX);
      State *l_state;
      for (auto i = 0; i < m_state_table.m_meta.m_count; ++i) {
        m_state_table.at(i, &l_state);
        *l_state = State::UNDEFINED;
      }
      m_input_events.allocate(0);
      m_just_pressed_keys.allocate(0);
      m_just_released_keys.allocate(0);
    };

    void free() {
      m_state_table.free();
      m_input_events.free();
      m_just_pressed_keys.free();
      m_just_released_keys.free();
    };

  } m_heap;

  void allocate() { m_heap.allocate(); };
  void free() { m_heap.free(); };

  void push_event(const Event &p_event) {
    m_heap.m_input_events.push_back(p_event);
  };

  void update() {
    __last_frame_events();
    __consume_events();
  };

private:
  void __last_frame_events() {
    for (auto i = 0; i < m_heap.m_just_pressed_keys.count(); ++i) {
      uimax l_key = m_heap.m_just_pressed_keys.at(i);
      State *l_state;
      m_heap.m_state_table.at(l_key, &l_state);
      assert_debug(*l_state == State::JUST_PRESSED);
      *l_state = State::PRESSED;
    }
    m_heap.m_just_pressed_keys.clear();

    for (auto i = 0; i < m_heap.m_just_released_keys.count(); ++i) {
      uimax l_key = m_heap.m_just_released_keys.at(i);
      State *l_state;
      m_heap.m_state_table.at(l_key, &l_state);
      assert_debug(*l_state == State::JUST_RELEASED);
      *l_state = State::UNDEFINED;
    }
    m_heap.m_just_released_keys.clear();
  };

  void __consume_events() {

    for (auto i = 0; i < m_heap.m_input_events.count(); ++i) {
      Event &l_event = m_heap.m_input_events.at(i);

      State *l_state;
      m_heap.m_state_table.at((uimax)l_event.m_key, &l_state);

      if (l_event.m_flag == Event::Flag::PRESSED) {
        m_heap.m_just_pressed_keys.push_back((uimax)l_event.m_key);
        *l_state = State::JUST_PRESSED;
      } else if (l_event.m_flag == Event::Flag::RELEASED) {
        m_heap.m_just_released_keys.push_back((uimax)l_event.m_key);
        *l_state = State::JUST_RELEASED;
      }
    }

    m_heap.m_input_events.clear();
  };
};

}; // namespace input

}; // namespace engine