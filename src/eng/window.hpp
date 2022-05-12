#pragma once

#include <cor/types.hpp>
#include <rast/model.hpp>
#include <sys/win.hpp>

namespace eng {

struct window_handle {
  void *m_idx;
};

struct window_image_buffer {
  container::span<ui8> m_data;
  ui16 m_width;
  ui16 m_height;
  void *m_native;

  void allocate(window_handle p_window, ui16 p_width, ui16 p_height) {
    m_data.allocate(p_width * p_height * (sizeof(ui8) * 4));
    m_width = p_width;
    m_height = p_height;
    m_native =
        win::allocate_image(p_window.m_idx, m_data.data(), p_width, p_height);
  };

  void free() { m_data.free(); };

  rast::image_view image_view() {
    return rast::image_view(m_width, m_height, sizeof(ui8) * 4, m_data.range());
  };
};

namespace window {
inline static window_handle open(ui32 p_width, ui32 p_height) {
  window_handle l_handle;
  l_handle.m_idx = win::create_window(p_width, p_height);
  win::show_window(l_handle.m_idx);
  return l_handle;
};

inline static void close(window_handle p_window) {
  win::close_window(p_window.m_idx);
};

inline static void fetch(container::vector<win::events> &in_out_events) {
  win::fetch_events(in_out_events);
};

inline static void draw(window_handle p_window, window_image_buffer p_image) {
  win::draw(p_window.m_idx, p_image.m_native, p_image.m_width,
            p_image.m_height);
};

}; // namespace window
}; // namespace eng