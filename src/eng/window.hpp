#pragma once

#include <cor/types.hpp>
#include <sys/win.hpp>

namespace eng {

struct window_handle {
  void *m_idx;
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

}; // namespace window
}; // namespace eng