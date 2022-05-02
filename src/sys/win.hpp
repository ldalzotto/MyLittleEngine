#pragma once

#include <cor/container.hpp>
#include <cor/types.hpp>
#include <eng/input.hpp>

namespace win {
void *create_window(ui32 p_width, ui32 p_height);
void show_window(void *p_window);
void close_window(void *p_window);

struct event {

  enum class type { Undefined = 0, InputPress = 1, InputRelease = 2 } m_type;

  struct input {
    eng::input::Key m_key;
  };

  union {
    input m_input;
  };
};

struct events {
    void* m_window;
    container::vector<event> m_events;

    void allocate(){
        m_events.allocate(0);
    };
    void free(){
        m_events.free();
    };
};

void fetch_events(container::vector<events> &in_out_events);

}; // namespace win
