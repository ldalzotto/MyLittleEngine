#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include <iostream>
#include <rast/model.hpp>
#include <sys/win.hpp>

namespace win {

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
  emscripten::val l_body = s_document.call<emscripten::val>(
      "getElementById", emscripten::val("body"));
  emscripten::val l_canvas = s_document.call<emscripten::val>(
      "createElement", emscripten::val("canvas"));
  l_canvas.set("width", p_width);
  l_canvas.set("height", p_height);
  l_body.call<emscripten::val>("appendChild", l_canvas);

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
  // s_document.call<emscripten::val>("getElementById",
  // emscripten::val("body"));
};

void *allocate_image(void *p_window, void *p_buffer, ui32 p_width,
                     ui32 p_height) {
  // emscripten_window *l_window = (emscripten_window *)p_window;
  /*
  emscripten::val l_image_data = l_window->m_ctx.call<emscripten::val>(
      "getImageData", emscripten::val(0), emscripten::val(0),
      emscripten::val(p_width), emscripten::val(p_height));
      */
  emscripten_image *l_image = new emscripten_image();
  l_image->m_data = (ui8 *)p_buffer;
  l_image->m_width = p_width;
  l_image->m_height = p_height;
  return l_image;
};

void free_image(void *p_image) {
  emscripten_image *l_image = (emscripten_image *)p_image;
  // sys::free(l_image->m_data);
  delete l_image;
};

void draw(void *p_window, void *p_image, ui32 p_width, ui32 p_height) {
  emscripten_window *l_window = (emscripten_window *)p_window;
  emscripten_image *l_image = (emscripten_image *)p_image;
  /*
  emscripten::val l_image_data =
      emscripten::val::global("ImageData")
          .new_(emscripten::val(p_width), emscripten::val(p_height));

  l_image_data.set(
      "data", emscripten::memory_view<ui8>(p_width * p_height * sizeof(ui8) * 4,
                                           l_image->m_data));
  l_image_data.set("width", emscripten::val(p_width));
  l_image_data.set("height", emscripten::val(p_height));
  l_window->m_ctx.call<void>("putImageData", l_image_data, emscripten::val(0),
                             emscripten::val(0));
*/
  s_document.call<void>(
      "doACopy", l_window->m_canvas,
      emscripten::memory_view<ui8>(p_width * p_height * sizeof(ui8) * 4,
                                   l_image->m_data));

  std::cout << "TEST" << std::endl;

  /*
l_window->m_ctx.call<void>(
"putImageData",
emscripten::val(emscripten::memory_view<ui8>(
  p_width * p_height * sizeof(ui8) * 4, l_image->m_data)),
emscripten::val(0), emscripten::val(0));
*/
};

void fetch_events(container::range<events> &in_out_events){
    // TODO
};

void debug_simulate_event(void *p_window, const event &p_event){

};
}; // namespace win