#include <rast/model.hpp>
#include <sys/win.hpp>

namespace win {

inline static container::arr<win::event, 128> s_events;
inline static ui8 s_events_count = 0;

struct window_headless {
  container::span<ui8> m_buffer;
  ui16 m_width;
  ui16 m_height;

  rast::image_view image_view() {
    return rast::image_view(m_width, m_height, sizeof(ui8) * 3,
                            m_buffer.range());
  };
};

struct window_headless_image {
  void *m_buffer;
  ui32 m_width;
  ui32 m_height;

  rast::image_view image_view() {
    return rast::image_view(
        m_width, m_height, sizeof(ui8) * 3,
        container::range<ui8>::make((ui8 *)m_buffer,
                                    m_width * m_height * sizeof(ui8) * 3));
  };
};

void *create_window(ui32 p_width, ui32 p_height) {
  window_headless *l_window_headless = new window_headless();
  l_window_headless->m_buffer.allocate(p_width * p_height * (sizeof(ui8) * 3));
  return l_window_headless;
};

void show_window(void *p_window){

};

void close_window(void *p_window) {
  window_headless *l_window = (window_headless *)p_window;
  l_window->m_buffer.free();
  delete l_window;
};

void *allocate_image(void *p_window, void *p_buffer, ui32 p_width,
                     ui32 p_height) {
  window_headless_image *l_image = new window_headless_image();
  *l_image = {.m_buffer = p_buffer, .m_width = p_width, .m_height = p_height};
  return l_image;
};

void free_image(void *p_image) {
  window_headless_image *l_image = (window_headless_image *)p_image;
  delete l_image;
};

void draw(void *p_window, void *p_image, ui32 p_width, ui32 p_height) {
  window_headless_image *l_image = (window_headless_image *)p_image;
  window_headless *l_window = (window_headless *)p_window;
  l_image->image_view().copy_to(l_window->image_view());
};

void fetch_events(container::range<events> &in_out_events) {
  auto &l_events = in_out_events.at(0).m_events;
  for (auto i = 0; i < s_events_count; ++i) {
    l_events.push_back(s_events.m_data[i]);
  }
  s_events_count = 0;
};

void debug_simulate_event(void* p_window, const event &p_event) {
  s_events.m_data[s_events_count] = p_event;
  s_events_count += 1;
};

}; // namespace win