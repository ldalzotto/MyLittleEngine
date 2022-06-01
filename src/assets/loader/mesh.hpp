#pragma once

#include <cor/container.hpp>
#include <m/math.hpp>
#include <m/vec.hpp>

namespace assets {

struct mesh_compiled_bytes {
  // container::span<ui8> m_data;

  struct header {
    uimax m_vertex_begin;
    uimax m_vertex_count;
    uimax m_vertex_color_begin;

    ui8 has_vertex_color() const { return m_vertex_color_begin != -1; };
  };

  static uimax size_of(uimax p_vertex_count, ui8 p_vertex_color) {
    return sizeof(header) + (sizeof(m::vec<fix32, 3>) * p_vertex_count) +
           (p_vertex_color ? (sizeof(m::vec<ui8, 3>) * p_vertex_count) : 0);
  };

  struct view {
    ui8 *m_data;

    header &header() const { return *(struct header *)m_data; };

    void initialize_header(uimax p_vertex_count, ui8 p_vertex_color) const {
      auto &l_header = header();
      l_header.m_vertex_count = p_vertex_count;
      l_header.m_vertex_begin = sizeof(struct header);
      l_header.m_vertex_color_begin =
          l_header.m_vertex_begin +
          l_header.m_vertex_count * sizeof(m::vec<fix32, 3>);
    };

    container::range<m::vec<fix32, 3>> vertices() const {
      return container::range<m::vec<fix32, 3>>::make(
          (m::vec<fix32, 3> *)(m_data + header().m_vertex_begin),
          header().m_vertex_count);
    };

    container::range<m::vec<ui8, 3>> vertex_color() const {
      assert_debug(header().has_vertex_color());
      return container::range<m::vec<ui8, 3>>::make(
          (m::vec<ui8, 3> *)(m_data + header().m_vertex_color_begin),
          header().m_vertex_count);
    };
  };
};

struct obj_mesh_bytes {
  const container::range<ui8> &m_data;

  uimax m_vertex_count;
  uimax m_vertex_begin;
  uimax m_vertex_color_begin;

  void mesh_header_pass() {
    m_vertex_count = 0;
    m_vertex_begin = -1;
    m_vertex_color_begin = -1;
    deserializer{*this}.mesh_header_pass();
  };

  void mesh_fill_pass(const mesh_compiled_bytes::view &p_mesh_view) {
    deserializer{*this}.mesh_fill_pass(p_mesh_view);
  };

private:
  struct deserializer {
    obj_mesh_bytes &thiz;

    uimax m_iterator;
    container::range<const ui8> m_line;

    enum class state {
      Undefined = 0,
      ReadVertex = 1,
      ReadVertexColor = 2
    } m_state;

    void mesh_header_pass() {
      m_iterator = 0;
      m_state = state::Undefined;
      while (next_line()) {
        process_line();
      }
    };

    void mesh_fill_pass(const mesh_compiled_bytes::view &p_mesh_view) {

      p_mesh_view.initialize_header(thiz.m_vertex_count,
                                    thiz.m_vertex_color_begin != -1);

      // vertices
      {
        auto l_mesh_vertices = p_mesh_view.vertices();
        uimax l_line_count = 0;
        m_iterator = thiz.m_vertex_begin;
        while (l_line_count < thiz.m_vertex_count) {
          next_line();
          auto l_vertex_coordinates = m_line.slide(2);
          auto l_white_space_it = 0;
          while (l_vertex_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_x = l_vertex_coordinates.shrink_to(l_white_space_it);
          l_vertex_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_vertex_coordinates.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_y = l_vertex_coordinates.shrink_to(l_white_space_it);
          l_vertex_coordinates.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_white_space_it != l_vertex_coordinates.count()) {
            l_white_space_it += 1;
          }
          auto l_z = l_vertex_coordinates.shrink_to(l_white_space_it);

          l_mesh_vertices.at(l_line_count) = {
              sys::stof(l_x.data(), l_x.count()),
              sys::stof(l_y.data(), l_y.count()),
              sys::stof(l_z.data(), l_z.count())};

          l_line_count += 1;
        }
      }

      // vertex color
      if (p_mesh_view.header().has_vertex_color()) {
        auto l_mesh_vertex_color = p_mesh_view.vertex_color();
        uimax l_line_count = 0;
        m_iterator = thiz.m_vertex_color_begin;
        while (l_line_count < thiz.m_vertex_count) {
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

         const m::vec<ui8, 3> l_color = {sys::stoui<ui8>(l_r.data(), l_r.count()),
                                    sys::stoui<ui8>(l_g.data(), l_g.count()),
                                    sys::stoui<ui8>(l_b.data(), l_b.count())};
          l_mesh_vertex_color.at(l_line_count) = l_color;

          l_line_count += 1;
        }
      }
    };

    void process_line() {
      state l_next_state;
      if (state_will_change(&l_next_state)) {
        on_state_change(l_next_state);
      }

      if (m_state == state::ReadVertex) {
        thiz.m_vertex_count += 1;
      }
    };

    ui8 state_will_change(state *out_state) {

      state l_line_state = state::Undefined;
      if (m_line.count() > 0) {
        if (m_line.count() >= 2) {
          if (m_line.at(0) == 'v') {
            if (m_line.at(1) == 'c') {
              l_line_state = state::ReadVertexColor;
            } else {
              l_line_state = state::ReadVertex;
            }
          }
        } else {
          if (m_line.at(0) == 'v') {
            l_line_state = state::ReadVertex;
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
      if (p_next == state::ReadVertex) {
        thiz.m_vertex_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadVertexColor) {
        thiz.m_vertex_color_begin = m_iterator - m_line.count() - 1;
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
    l_value.allocate(mesh_compiled_bytes::size_of(
        l_mesh_bytes.m_vertex_count, l_mesh_bytes.m_vertex_color_begin != -1));

    l_mesh_bytes.mesh_fill_pass(mesh_compiled_bytes::view{l_value.data()});
    return l_value;
  };
};

}; // namespace assets