#pragma once

#include <cor/orm.hpp>
#include <cor/types.hpp>
#include <rast/image_texture.hpp>
#include <shared/types.hpp>
#include <sys/win.hpp>

namespace eng {

struct window_handle {
  uimax m_idx;
};

struct window_native_ptr {
  void *m_ptr;
};

struct window_image_buffer {
  container::span<ui8> m_data;
  ui16 m_width;
  ui16 m_height;
  void *m_native;

  void allocate(window_native_ptr p_window, ui16 p_width, ui16 p_height) {
    m_data.allocate(p_width * p_height * (sizeof(ui8) * 4));
    m_data.range().memset(255);
    m_width = p_width;
    m_height = p_height;
    m_native =
        win::allocate_image(p_window.m_ptr, m_data.data(), p_width, p_height);
  };

  void free() {
    m_data.free();
    win::free_image(m_native);
  };

  rast::image_view image_view() {
    return rast::image_view(m_width, m_height, sizeof(ui8) * 4, m_data.range());
  };
};

namespace window {

struct system {
public:
  void allocate() { __allocate(); };
  void free() { __free(); };

  window_handle create_window(ui16 p_width, ui16 p_height) {
    return __create_window(p_width, p_height);
  };

  void open_window(window_handle p_window) { __open_window(p_window); };
  void close_window(window_handle p_window) { __close_window(p_window); };
  void draw_window(window_handle p_window, const rast::image_view &p_image) {
    window_native_ptr *l_native_ptr;
    window_image_buffer *l_image_buffer;
    win::events *l_events;
    m_window_table.at(p_window.m_idx, &l_native_ptr, &l_image_buffer,
                      &l_events);
    rast::image_copy_stretch((rgb_t *)p_image.m_buffer.m_begin, p_image.m_width,
                             p_image.m_height,
                             (rgba_t *)l_image_buffer->m_data.m_data,
                             l_image_buffer->m_width, l_image_buffer->m_height);
    win::draw(l_native_ptr->m_ptr, l_image_buffer->m_native,
              l_image_buffer->m_width, l_image_buffer->m_height);
  };

  window_native_ptr window_get_native_ptr(window_handle p_window) {
    window_native_ptr *l_native_ptr;
    m_window_table.at(p_window.m_idx, &l_native_ptr);
    return *l_native_ptr;
  };

  window_image_buffer &window_get_image_buffer(window_handle p_window) {
    window_image_buffer *l_image_buffer;
    m_window_table.at(p_window.m_idx, none(), &l_image_buffer);
    return *l_image_buffer;
  };

  ui8 fetch_events() { return __fetch_events(); };
  container::range<eng::input::Event> input_system_events() {
    return m_input_system_events.range();
  };

private:
  using window_table =
      orm::table_pool_v2<window_native_ptr, window_image_buffer, win::events>;

  window_table m_window_table;

  container::vector<eng::input::Event> m_input_system_events;

  void __allocate() {
    m_window_table.allocate(0);
    m_input_system_events.allocate(0);
  };

  void __free() {
    assert_debug(!m_window_table.has_allocated_elements());
    m_window_table.free();
    m_input_system_events.free();
  };

  window_handle __create_window(ui16 p_width, ui16 p_height) {
    window_native_ptr l_native_ptr;
    window_image_buffer l_image_buffer;
    win::events l_events;
    l_native_ptr = {win::create_window(p_width, p_height)};
    l_image_buffer.allocate(l_native_ptr, p_width, p_height);
    l_events.m_window = l_native_ptr.m_ptr;
    l_events.allocate();

    uimax l_index =
        m_window_table.push_back(l_native_ptr, l_image_buffer, l_events);
    return {l_index};
  };

  void __open_window(window_handle p_window) {
    window_native_ptr *l_native_ptr;
    m_window_table.at(p_window.m_idx, &l_native_ptr);
    win::show_window(l_native_ptr->m_ptr);
  };

  void __close_window(window_handle p_window) {
    window_native_ptr *l_native_ptr;
    window_image_buffer *l_image_buffer;
    win::events *l_events;
    m_window_table.at(p_window.m_idx, &l_native_ptr, &l_image_buffer,
                      &l_events);
    win::close_window(l_native_ptr->m_ptr);
    l_image_buffer->free();
    l_events->free();
    m_window_table.remove_at(p_window.m_idx);
    return;
  };

  ui8 __fetch_events() {
    m_input_system_events.clear();

    window_handle l_handle = {0};
    window_native_ptr *l_native_ptr;
    window_image_buffer *l_image_buffer;
    win::events *l_events;
    m_window_table.at(l_handle.m_idx, &l_native_ptr, &l_image_buffer,
                      &l_events);

    {
      auto l_events_for_fetch =
          container::range<win::events>::make(l_events, 1);
      win::fetch_events(l_events_for_fetch);
    }

    for (auto i = 0; i < l_events->m_events.count(); ++i) {
      auto &l_event = l_events->m_events.at(i);
      if (l_event.m_type == win::event::type::InputPress) {
        eng::input::Event l_input_event;
        l_input_event.m_key = l_event.m_input.m_key;
        l_input_event.m_flag = eng::input::Event::Flag::PRESSED;
        m_input_system_events.push_back(l_input_event);
      } else if (l_event.m_type == win::event::type::InputRelease) {
        eng::input::Event l_input_event;
        l_input_event.m_key = l_event.m_input.m_key;
        l_input_event.m_flag = eng::input::Event::Flag::RELEASED;
        m_input_system_events.push_back(l_input_event);
      } else if (l_event.m_type == win::event::type::Redraw) {
        if (l_event.m_draw.m_width != l_image_buffer->m_width &&
            l_event.m_draw.m_height != l_image_buffer->m_height) {
          l_image_buffer->free();
          l_image_buffer->allocate(*l_native_ptr, l_event.m_draw.m_width,
                                   l_event.m_draw.m_height);
        }
      } else if (l_event.m_type == win::event::type::Close) {
        __close_window(l_handle);
      }
    }
    l_events->m_events.clear();

    return 1;
  };
};

}; // namespace window

}; // namespace eng