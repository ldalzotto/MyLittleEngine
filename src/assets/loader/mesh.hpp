#pragma once

#include <cor/container.hpp>
#include <m/math.hpp>
#include <m/vec.hpp>

namespace assets {

// TODO -> improving this format to be more generic ?
struct mesh_intermediary_bytes {

  struct header {
    uimax m_position_begin;
    uimax m_position_count;
    ui8 m_color;
    uimax m_color_begin;
    uimax m_uv_begin;
    uimax m_uv_count;
    uimax m_normal_begin;
    uimax m_normal_count;

    ui8 has_color() const { return m_color; };
    ui8 has_uv() const { return m_uv_count != 0; };
    ui8 has_normal() const { return m_normal_count != 0; }
  };

  static uimax size_of(uimax p_position_count, ui8 p_color, uimax p_uv_count,
                       uimax p_normal_count) {
    return sizeof(header) + (sizeof(m::vec<fix32, 3>) * p_position_count) +
           (p_color ? (sizeof(m::vec<ui8, 3>) * p_position_count) : 0) +
           (sizeof(m::vec<fix32, 2>) * p_uv_count) +
           (sizeof(m::vec<fix32, 3>) * p_normal_count);
  };

  struct view {
    ui8 *m_data;

    header &header() const { return *(struct header *)m_data; };

    void initialize_header(uimax p_position_count, ui8 p_color,
                           uimax p_uv_count, uimax p_normal_count) const {

      uimax l_it = 0;
      auto &l_header = header();
      l_it += sizeof(struct header);
      l_header.m_position_count = p_position_count;
      l_header.m_position_begin = l_it;
      l_it += (l_header.m_position_count * sizeof(m::vec<fix32, 3>));
      l_header.m_color_begin = l_it;
      l_header.m_color = p_color;
      if (p_color) {
        l_it += (l_header.m_position_count * sizeof(m::vec<fix32, 3>));
      }
      l_header.m_uv_count = p_uv_count;
      l_header.m_uv_begin = l_it;
      l_it += (p_uv_count * sizeof(m::vec<fix32, 2>));

      l_header.m_normal_count = p_normal_count;
      l_header.m_normal_begin = l_it;
      l_it += (p_normal_count * sizeof(m::vec<fix32, 3>));
    };

    container::range<m::vec<fix32, 3>> position() const {
      return container::range<m::vec<fix32, 3>>::make(
          (m::vec<fix32, 3> *)(m_data + header().m_position_begin),
          header().m_position_count);
    };

    container::range<m::vec<ui8, 3>> color() const {
      assert_debug(header().has_color());
      return container::range<m::vec<ui8, 3>>::make(
          (m::vec<ui8, 3> *)(m_data + header().m_color_begin),
          header().m_position_count);
    };

    container::range<m::vec<fix32, 2>> uv() const {
      assert_debug(header().has_uv());
      return container::range<m::vec<fix32, 2>>::make(
          (m::vec<fix32, 2> *)(m_data + header().m_uv_begin),
          header().m_uv_count);
    };

    container::range<m::vec<fix32, 3>> normal() const {
      assert_debug(header().has_normal());
      return container::range<m::vec<fix32, 3>>::make(
          (m::vec<fix32, 3> *)(m_data + header().m_normal_begin),
          header().m_normal_count);
    };
  };
};

struct obj_mesh_bytes {
  const container::range<ui8> &m_data;

  uimax m_position_count;
  uimax m_position_begin;
  uimax m_color_begin;
  uimax m_uv_begin;
  uimax m_uv_count;
  uimax m_normal_begin;
  uimax m_normal_count;

  void mesh_header_pass() {
    m_position_count = 0;
    m_position_begin = -1;
    m_color_begin = -1;
    m_uv_begin = -1;
    m_uv_count = 0;
    m_normal_begin = -1;
    m_normal_count = 0;
    deserializer{*this}.mesh_header_pass();
  };

  void mesh_fill_pass(const mesh_intermediary_bytes::view &p_mesh_view) {
    deserializer{*this}.mesh_fill_pass(p_mesh_view);
  };

private:
  struct deserializer {
    obj_mesh_bytes &thiz;

    uimax m_iterator;
    container::range<const ui8> m_line;

    enum class state {
      Undefined = 0,
      ReadPosition = 1,
      ReadColor = 2,
      ReadUv = 3,
      ReadNormal = 4
    } m_state;

    void mesh_header_pass() {
      m_iterator = 0;
      m_state = state::Undefined;
      while (next_line()) {
        process_line();
      }
    };

    void mesh_fill_pass(const mesh_intermediary_bytes::view &p_mesh_view) {

      p_mesh_view.initialize_header(thiz.m_position_count,
                                    thiz.m_color_begin != -1, thiz.m_uv_count,
                                    thiz.m_normal_count);

      // vertices
      {
        auto l_mesh_vertices = p_mesh_view.position();
        uimax l_line_count = 0;
        m_iterator = thiz.m_position_begin;
        while (l_line_count < thiz.m_position_count) {
          next_line();
          auto l_position_coordinates = m_line.slide(2);
          auto l_white_space_it = 0;
          while (l_position_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_x = l_position_coordinates.shrink_to(l_white_space_it);
          l_position_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_position_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_y = l_position_coordinates.shrink_to(l_white_space_it);
          l_position_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_white_space_it != l_position_coordinates.count()) {
            l_white_space_it += 1;
          }
          auto l_z = l_position_coordinates.shrink_to(l_white_space_it);

          const m::vec<fix32, 3> l_position = {
              sys::stof(l_x.data(), l_x.count()),
              sys::stof(l_y.data(), l_y.count()),
              sys::stof(l_z.data(), l_z.count())};
          l_mesh_vertices.at(l_line_count) = l_position;

          l_line_count += 1;
        }
      }

      // color
      if (p_mesh_view.header().has_color()) {
        auto l_mesh_color = p_mesh_view.color();
        uimax l_line_count = 0;
        m_iterator = thiz.m_color_begin;
        while (l_line_count < thiz.m_position_count) {
          next_line();
          auto l_vertex_coordinates = m_line.slide(3);
          auto l_white_space_it = 0;
          while (l_vertex_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_r = l_vertex_coordinates.shrink_to(l_white_space_it);
          l_vertex_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_vertex_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_g = l_vertex_coordinates.shrink_to(l_white_space_it);
          l_vertex_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_white_space_it != l_vertex_coordinates.count()) {
            l_white_space_it += 1;
          }
          auto l_b = l_vertex_coordinates.shrink_to(l_white_space_it);

          const m::vec<ui8, 3> l_color = {
              sys::stoui<ui8>(l_r.data(), l_r.count()),
              sys::stoui<ui8>(l_g.data(), l_g.count()),
              sys::stoui<ui8>(l_b.data(), l_b.count())};
          l_mesh_color.at(l_line_count) = l_color;

          l_line_count += 1;
        }
      }

      // uv
      if (p_mesh_view.header().has_uv()) {
        auto l_mesh_uv = p_mesh_view.uv();
        uimax l_line_count = 0;
        m_iterator = thiz.m_uv_begin;
        while (l_line_count < thiz.m_uv_count) {
          next_line();
          auto l_vertex_coordinates = m_line.slide(3);
          auto l_white_space_it = 0;
          while (l_vertex_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_u = l_vertex_coordinates.shrink_to(l_white_space_it);
          l_vertex_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;

          while (l_white_space_it != l_vertex_coordinates.count()) {
            l_white_space_it += 1;
          }
          auto l_v = l_vertex_coordinates.shrink_to(l_white_space_it);

          const m::vec<fix32, 2> l_uv = {sys::stof(l_u.data(), l_u.count()),
                                         sys::stof(l_v.data(), l_v.count())};
          l_mesh_uv.at(l_line_count) = l_uv;

          l_line_count += 1;
        }
      }

      // normals
      if (p_mesh_view.header().has_normal()) {
        auto l_mesh_normals = p_mesh_view.normal();
        uimax l_line_count = 0;
        m_iterator = thiz.m_normal_begin;
        while (l_line_count < thiz.m_normal_count) {
          next_line();
          auto l_normal_coordinates = m_line.slide(3);
          auto l_white_space_it = 0;
          while (l_normal_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_x = l_normal_coordinates.shrink_to(l_white_space_it);
          l_normal_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_normal_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_y = l_normal_coordinates.shrink_to(l_white_space_it);
          l_normal_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_white_space_it != l_normal_coordinates.count()) {
            l_white_space_it += 1;
          }
          auto l_z = l_normal_coordinates.shrink_to(l_white_space_it);

          const m::vec<fix32, 3> l_position = {
              sys::stof(l_x.data(), l_x.count()),
              sys::stof(l_y.data(), l_y.count()),
              sys::stof(l_z.data(), l_z.count())};
          l_mesh_normals.at(l_line_count) = l_position;

          l_line_count += 1;
        }
      }
    };

    void process_line() {
      state l_next_state;
      if (state_will_change(&l_next_state)) {
        on_state_change(l_next_state);
      }

      if (m_state == state::ReadPosition) {
        thiz.m_position_count += 1;
      } else if (m_state == state::ReadUv) {
        thiz.m_uv_count += 1;
      } else if (m_state == state::ReadNormal) {
        thiz.m_normal_count += 1;
      }
    };

    ui8 state_will_change(state *out_state) {

      state l_line_state = state::Undefined;
      if (m_line.count() > 0) {
        if (m_line.at(0) == 'v') {
          if (m_line.at(1) == 'c') {
            l_line_state = state::ReadColor;
          } else if (m_line.at(1) == 't') {
            l_line_state = state::ReadUv;
          } else if (m_line.at(1) == 'n') {
            l_line_state = state::ReadNormal;
          } else {
            l_line_state = state::ReadPosition;
          }
        }
      }

      if (l_line_state == state::Undefined) {
        if (m_state != state::Undefined) {
          *out_state = state::Undefined;
          return 1;
        }
      } else if (m_state != l_line_state) {
        *out_state = l_line_state;
        return 1;
      }

      return 0;
    };

    void on_state_change(state p_next) {
      assert_debug(m_state != p_next);
      if (p_next == state::ReadPosition) {
        thiz.m_position_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadColor) {
        thiz.m_color_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadUv) {
        thiz.m_uv_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadNormal) {
        thiz.m_normal_begin = m_iterator - m_line.count() - 1;
      }
      m_state = p_next;
    };

    // TODO -> move this to algorithm ?
    ui8 next_line() {
      m_line =
          container::range<const ui8>::make(&thiz.m_data.at(m_iterator), 0);
      while (true) {

        if (m_iterator == thiz.m_data.count() - 1) {
          return 0;
        }

        if (thiz.m_data.at(m_iterator) == '\n') {
          if ((m_iterator + 1) == thiz.m_data.count() - 1) {
            return 0;
          } else {
            m_iterator += 1;
            return 1;
          }
        }

        m_iterator += 1;
        m_line.count() += 1;
      }
    };
  };
};

struct obj_mesh_loader {
  container::span<ui8> compile(const container::range<ui8> &p_raw_obj) {
    return __compile(p_raw_obj);
  };

private:
  container::span<ui8> __compile(const container::range<ui8> &p_raw_obj) {
    obj_mesh_bytes l_mesh_bytes = obj_mesh_bytes{.m_data = p_raw_obj};
    l_mesh_bytes.mesh_header_pass();

    container::span<ui8> l_value;
    l_value.allocate(mesh_intermediary_bytes::size_of(
        l_mesh_bytes.m_position_count, l_mesh_bytes.m_color_begin != -1,
        l_mesh_bytes.m_uv_count, l_mesh_bytes.m_normal_count));

    l_mesh_bytes.mesh_fill_pass(mesh_intermediary_bytes::view{l_value.data()});
    return l_value;
  };
};

}; // namespace assets