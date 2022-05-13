#pragma once

#include <cor/container.hpp>
#include <cor/types.hpp>
#include <eng/input.hpp>

namespace win {
void *create_window(ui32 p_width, ui32 p_height);
void show_window(void *p_window);
void close_window(void *p_window);
void *allocate_image(void *p_window, void *p_buffer, ui32 p_width,
                     ui32 p_height);
void free_image(void *p_image);
void draw(void *p_window, void *p_image, ui32 p_width, ui32 p_height);

struct event {

  enum class type {
    Undefined = 0,
    InputPress = 1,
    InputRelease = 2,
    Redraw = 3,
    Close = 4
  } m_type;

  struct input {
    eng::input::Key m_key;
  };

  struct draw {
    ui16 m_width;
    ui16 m_height;
  };

  union {
    input m_input;
    draw m_draw;
  };
};

struct events {
  void *m_window;
  container::vector<event> m_events;

  void allocate() { m_events.allocate(0); };
  void free() { m_events.free(); };
};

void fetch_events(container::range<events> &in_out_events);

void debug_simulate_event(void *p_window, const event &p_event);

}; // namespace win
