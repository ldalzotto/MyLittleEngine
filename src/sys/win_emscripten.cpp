#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <rast/model.hpp>
#include <sys/win.hpp>

#include <iostream>

namespace emscripten_input {

static const container::arr<ui8, 10> s_key_arrow_down = {"ArrowDown"};
static const container::arr<ui8, 8> s_key_arrow_up = {"ArrowUp"};
static const container::arr<ui8, 10> s_key_arrow_left = {"ArrowLeft"};
static const container::arr<ui8, 11> s_key_arrow_right = {"ArrowRight"};

static container::arr<ui8, i32(eng::input::Key::ENUM_MAX)>
    s_emscripten_key_pressed;

eng::input::Key
input_key_from_emscripten(const EmscriptenKeyboardEvent *p_event) {
  container::range<ui8> l_key =
      container::range<ui8>::make((ui8 *)p_event->key, 32);
  if (s_key_arrow_down.range().is_contained_by(l_key)) {
    return eng::input::Key::ARROW_DOWN;
  } else if (s_key_arrow_up.range().is_contained_by(l_key)) {
    return eng::input::Key::ARROW_UP;
  } else if (s_key_arrow_left.range().is_contained_by(l_key)) {
    return eng::input::Key::ARROW_LEFT;
  } else if (s_key_arrow_right.range().is_contained_by(l_key)) {
    return eng::input::Key::ARROW_RIGHT;
  }
  return eng::input::Key::UNDEFINED;
};

}; // namespace emscripten_input

namespace win {

using namespace emscripten_input;

inline static container::vector<event> s_events;

i32 on_key_event(i32 p_event_type, const EmscriptenKeyboardEvent *p_event,
                 void *) {
  if (p_event->repeat) {
    return 1;
  }

  event l_event;
  if (p_event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
    l_event.m_type = event::type::InputPress;
    l_event.m_input.m_key = input_key_from_emscripten(p_event);
    if (s_emscripten_key_pressed.at(uimax(l_event.m_input.m_key))) {
      return 1;
    }
    s_emscripten_key_pressed.at(uimax(l_event.m_input.m_key)) = 1;
  } else if (p_event_type == EMSCRIPTEN_EVENT_KEYUP) {
    l_event.m_type = event::type::InputRelease;
    l_event.m_input.m_key = input_key_from_emscripten(p_event);
    s_emscripten_key_pressed.at(uimax(l_event.m_input.m_key)) = 0;
  }

  std::cout << (ui32)l_event.m_input.m_key << std::endl;
  std::cout << p_event->key << std::endl;

  s_events.push_back(l_event);
  return 1;
};

inline static const emscripten::val s_document =
    emscripten::val::global("document");

struct emscripten_window {
  emscripten::val m_canvas;
  emscripten::val m_ctx;

  emscripten_window(emscripten::val &p_canvas, emscripten::val &p_ctx)
      : m_canvas(p_canvas), m_ctx(p_ctx){

                            };
};

struct emscripten_image {
  ui8 *m_data;
  ui16 m_width;
  ui16 m_height;
};

void *create_window(ui32 p_width, ui32 p_height) {

  s_events.allocate(0);
  s_emscripten_key_pressed.range().zero();

  emscripten::val l_body = s_document.call<emscripten::val>(
      "getElementById", emscripten::val("body"));
  emscripten::val l_canvas = s_document.call<emscripten::val>(
      "createElement", emscripten::val("canvas"));
  l_canvas.set("width", p_width);
  l_canvas.set("height", p_height);
  l_canvas.set("tabindex", -1);
  l_canvas.call<void>("focus");
  l_body.call<emscripten::val>("appendChild", l_canvas);

  emscripten_set_keydown_callback("canvas", 0, 0, on_key_event);
  emscripten_set_keyup_callback("canvas", 0, 0, on_key_event);

  emscripten::val l_ctx =
      l_canvas.call<emscripten::val>("getContext", emscripten::val("2d"));
  l_ctx.call<void>("rect", 0, 0, p_width, p_height);

  emscripten_window *l_window = new emscripten_window(l_canvas, l_ctx);
  return l_window;
};

void show_window(void *p_window){

};

void close_window(void *p_window) {
  emscripten_window *l_window = (emscripten_window *)p_window;
  delete l_window;
};

void *allocate_image(void *p_window, void *p_buffer, ui32 p_width,
                     ui32 p_height) {

  emscripten_image *l_image = new emscripten_image();
  l_image->m_data = (ui8 *)p_buffer;
  l_image->m_width = p_width;
  l_image->m_height = p_height;
  return l_image;
};

void free_image(void *p_image) {
  emscripten_image *l_image = (emscripten_image *)p_image;
  delete l_image;
};

void draw(void *p_window, void *p_image, ui32 p_width, ui32 p_height) {
  emscripten_window *l_window = (emscripten_window *)p_window;
  emscripten_image *l_image = (emscripten_image *)p_image;

  s_document.call<void>(
      "doACopy", l_window->m_ctx, p_width, p_height,
      emscripten::memory_view<ui8>(p_width * p_height * sizeof(ui8) * 4,
                                   l_image->m_data));
};

void fetch_events(container::range<events> &in_out_events) {
  /*
  events &l_events = in_out_events.at(0);
  for (auto i = 0; i < s_events.count(); ++i) {
    l_events.m_events.push_back(s_events.at(i));
  }
  */
  s_events.clear();
};

void debug_simulate_event(void *p_window, const event &p_event){

};
}; // namespace win