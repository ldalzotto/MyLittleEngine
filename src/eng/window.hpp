#pragma once

#include <cor/orm.hpp>
#include <cor/types.hpp>
#include <rast/model.hpp>
#include <shared/types.hpp>
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
    m_data.range().memset(255);
    m_width = p_width;
    m_height = p_height;
    m_native =
        win::allocate_image(p_window.m_idx, m_data.data(), p_width, p_height);
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
  void draw_window(ui8 p_window_index, const rast::image_view &p_image) {
    window_handle *l_handle;
    window_image_buffer *l_image_buffer;
    win::events *l_events;
    m_window_table.at(p_window_index, &l_handle, &l_image_buffer, &l_events);
    rast::image_copy_stretch((rgb_t *)p_image.m_buffer.m_begin, p_image.m_width,
                             p_image.m_height,
                             (rgba_t *)l_image_buffer->m_data.m_data,
                             l_image_buffer->m_width, l_image_buffer->m_height);
    win::draw(l_handle->m_idx, l_image_buffer->m_native,
              l_image_buffer->m_width, l_image_buffer->m_height);
  };

  ui8 fetch_events() { return __fetch_events(); };
  container::range<eng::input::Event> input_system_events() {
    return m_input_system_events.range();
  };

private:
  using window_table =
      orm::table_vector_v2<window_handle, window_image_buffer, win::events>;

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
    window_handle l_handle;
    window_image_buffer l_image_buffer;
    win::events l_events;
    l_handle = {win::create_window(p_width, p_height)};
    l_image_buffer.allocate(l_handle, p_width, p_height);
    l_events.m_window = l_handle.m_idx;
    l_events.allocate();

    m_window_table.push_back(l_handle, l_image_buffer, l_events);

    return l_handle;
  };

  void __open_window(window_handle p_window) {
    win::show_window(p_window.m_idx);
  };

  void __close_window(window_handle p_window) {
    window_handle *l_handle;
    window_image_buffer *l_image_buffer;
    win::events *l_events;
    for (auto i = 0; i < m_window_table.count(); ++i) {
      m_window_table.at(i, &l_handle, &l_image_buffer, &l_events);
      if (l_handle->m_idx == p_window.m_idx) {
        win::close_window(l_handle->m_idx);
        l_image_buffer->free();
        l_events->free();
        m_window_table.remove_at(i);
        return;
      }
    }
    assert_debug(0);
  };

  ui8 __fetch_events() {
    m_input_system_events.clear();

    ui8 l_window_index = 0;
    window_handle *l_handle;
    window_image_buffer *l_image_buffer;
    win::events *l_events;
    m_window_table.at(l_window_index, &l_handle, &l_image_buffer, &l_events);

    {
      auto l_events_for_fetch = container::range<win::events>::make(
          m_window_table.m_cols.m_col_2, m_window_table.count());
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
          l_image_buffer->allocate(*l_handle, l_event.m_draw.m_width,
                                   l_event.m_draw.m_height);
        }
      } else if (l_event.m_type == win::event::type::Close) {
        __close_window(*l_handle);
      }
    }
    l_events->m_events.clear();

    if (m_window_table.count() == 0) {
      return 0;
    }

    return 1;
  };
};

}; // namespace window

}; // namespace eng