#pragma once

#include <assets/mesh.hpp>
#include <m/math.hpp>
#include <m/vec.hpp>
namespace assets {

namespace details {

struct obj_mesh_bytes {
  const container::range<ui8> &m_data;

  uimax m_position_count;
  uimax m_position_begin;
  uimax m_color_count;
  uimax m_color_begin;
  uimax m_uv_begin;
  uimax m_uv_count;
  uimax m_normal_begin;
  uimax m_normal_count;
  uimax m_face_begin;
  uimax m_face_count;

  void validity_pass() { deserializer{*this}.validity_pass(); };

  void mesh_header_pass() {
    m_position_count = 0;
    m_position_begin = -1;
    m_color_count = 0;
    m_color_begin = -1;
    m_uv_begin = -1;
    m_uv_count = 0;
    m_normal_begin = -1;
    m_normal_count = 0;
    m_face_begin = -1;
    m_face_count = 0;
    deserializer{*this}.mesh_header_pass();
  };

  void mesh_fill_pass(mesh_intermediary &p_mesh_intermediary) {
    deserializer{*this}.mesh_fill_pass(p_mesh_intermediary);
  };

private:
  struct deserializer {
    obj_mesh_bytes &thiz;
    algorithm::str_line_iterator<const container::range<ui8>> m_line_iterator;

    enum class state {
      Undefined = 0,
      ReadPosition = 1,
      ReadColor = 2,
      ReadUv = 3,
      ReadNormal = 4,
      ReadFace = 5
    } m_state;

    deserializer(obj_mesh_bytes &p_obj_mesh_bytes)
        : thiz(p_obj_mesh_bytes), m_line_iterator(p_obj_mesh_bytes.m_data){};

    void mesh_header_pass() {
      m_state = state::Undefined;
      m_line_iterator.for_each_line(
          [&](auto) { process_line_to_fill_header(); });
    };

    void validity_pass() {
      m_state = state::Undefined;
      m_line_iterator.for_each_line(
          [&](auto) { process_line_for_validity_check(); });
    };

    void mesh_fill_pass(mesh_intermediary &p_mesh_intermediary) {

      // position
      if (p_mesh_intermediary.m_composition.m_position) {
        auto l_mesh_vertices = p_mesh_intermediary.position();
        uimax l_line_count = 0;
        m_line_iterator.set_iterator_index(thiz.m_position_begin);
        while (l_line_count < thiz.m_position_count) {
          auto l_position_coordinates = m_line_iterator.next_line().slide(2);

          algorithm::str_iterator<const container::range<ui8>> l_str_iterator(
              l_position_coordinates);

          container::range<ui8> l_x =
              l_str_iterator.range_next_char_advance(' ');
          container::range<ui8> l_y =
              l_str_iterator.range_next_char_advance(' ');
          container::range<ui8> l_z = l_str_iterator.range_until_end();

          const position_t l_position = {sys::stof(l_x.data(), l_x.count()),
                                         sys::stof(l_y.data(), l_y.count()),
                                         sys::stof(l_z.data(), l_z.count())};
          l_mesh_vertices.at(l_line_count) = l_position;

          l_line_count += 1;
        }
      }

      // color
      if (p_mesh_intermediary.m_composition.m_color) {
        auto l_mesh_color = p_mesh_intermediary.color();
        uimax l_line_count = 0;
        m_line_iterator.set_iterator_index(thiz.m_color_begin);
        while (l_line_count < thiz.m_color_count) {
          auto l_vertex_coordinates = m_line_iterator.next_line().slide(3);

          algorithm::str_iterator<const container::range<ui8>> l_str_iterator(
              l_vertex_coordinates);

          container::range<ui8> l_r =
              l_str_iterator.range_next_char_advance(' ');
          container::range<ui8> l_g =
              l_str_iterator.range_next_char_advance(' ');
          container::range<ui8> l_b = l_str_iterator.range_until_end();

          const rgb_t l_color = {sys::stoui<ui8>(l_r.data(), l_r.count()),
                                 sys::stoui<ui8>(l_g.data(), l_g.count()),
                                 sys::stoui<ui8>(l_b.data(), l_b.count())};
          l_mesh_color.at(l_line_count) = l_color;

          l_line_count += 1;
        }
      }

      // uv
      if (p_mesh_intermediary.m_composition.m_uv) {
        auto l_mesh_uv = p_mesh_intermediary.uv();
        uimax l_line_count = 0;
        m_line_iterator.set_iterator_index(thiz.m_uv_begin);
        while (l_line_count < thiz.m_uv_count) {
          auto l_vertex_coordinates = m_line_iterator.next_line().slide(3);
          algorithm::str_iterator<const container::range<ui8>> l_str_iterator(
              l_vertex_coordinates);

          auto l_u = l_str_iterator.range_next_char_advance(' ');
          auto l_v = l_str_iterator.range_until_end();

          const uv_t l_uv = {sys::stof(l_u.data(), l_u.count()),
                             sys::stof(l_v.data(), l_v.count())};
          l_mesh_uv.at(l_line_count) = l_uv;

          l_line_count += 1;
        }
      }

      // normals
      if (p_mesh_intermediary.m_composition.m_normal) {
        auto l_mesh_normals = p_mesh_intermediary.normal();
        uimax l_line_count = 0;
        m_line_iterator.set_iterator_index(thiz.m_normal_begin);
        while (l_line_count < thiz.m_normal_count) {
          auto l_normal_coordinates = m_line_iterator.next_line().slide(3);
          algorithm::str_iterator<const container::range<ui8>> l_str_iterator(
              l_normal_coordinates);

          auto l_x = l_str_iterator.range_next_char_advance(' ');
          auto l_y = l_str_iterator.range_next_char_advance(' ');
          auto l_z = l_str_iterator.range_until_end();

          const normal_t l_normal = {sys::stof(l_x.data(), l_x.count()),
                                     sys::stof(l_y.data(), l_y.count()),
                                     sys::stof(l_z.data(), l_z.count())};
          l_mesh_normals.at(l_line_count) = l_normal;

          l_line_count += 1;
        }
      }

      // faces
      if (p_mesh_intermediary.face().count() > 0) {
        auto l_faces = p_mesh_intermediary.face();
        uimax l_line_count = 0;
        m_line_iterator.set_iterator_index(thiz.m_face_begin);
        while (l_line_count < thiz.m_face_count) {
          auto l_face_line = m_line_iterator.next_line().slide(2);
          algorithm::str_iterator<const container::range<ui8>> l_str_iterator(
              l_face_line);

          auto l_f1 = l_str_iterator.range_next_char_advance(' ');
          auto l_f2 = l_str_iterator.range_next_char_advance(' ');
          auto l_f3 = l_str_iterator.range_until_end();

          const auto l_face = container::arr<container::arr<vindex_t, 4>, 3>{
              extract_face_indices(l_f1), extract_face_indices(l_f2),
              extract_face_indices(l_f3)};
          l_faces.at(l_line_count) = l_face;
          l_line_count += 1;
        }
      }
    };

    static container::arr<vindex_t, 4>
    extract_face_indices(const container::range<ui8> &p_str) {
      container::arr<vindex_t, 4> l_out;
      l_out.range().zero();
      ui8 l_out_index = 0;

      algorithm::str_iterator<const container::range<ui8>> l_str_iterator(
          p_str);
      l_str_iterator.split('/', [&](const container::range<ui8> &p_range) {
        l_out.at(l_out_index) =
            sys::stoui<vindex_t>(p_range.data(), p_range.count()) - 1;
        l_out_index += 1;
      });

      return l_out;
    };

    void process_line_to_fill_header() {
      state l_next_state;
      if (state_will_change(&l_next_state)) {
        on_state_change(l_next_state);
      }

      if (m_state == state::ReadPosition) {
        thiz.m_position_count += 1;
      } else if (m_state == state::ReadColor) {
        thiz.m_color_count += 1;
      } else if (m_state == state::ReadUv) {
        thiz.m_uv_count += 1;
      } else if (m_state == state::ReadNormal) {
        thiz.m_normal_count += 1;
      } else if (m_state == state::ReadFace) {
        thiz.m_face_count += 1;
      }
    };

    void process_line_for_validity_check() {
      state l_next_state;
      if (state_will_change(&l_next_state)) {
        m_state = l_next_state;
      }

      if (m_state != state::Undefined) {
        container::range<ui8> l_line = m_line_iterator.line();
        ui8 l_last_char = l_line.at(l_line.count() - 1);
        sys::sassert(l_last_char == '/' ||
                     (l_last_char >= '0' && l_last_char <= '9'));
      }
    };

    ui8 state_will_change(state *out_state) {

      state l_line_state = state::Undefined;
      if (m_line_iterator.line().count() > 0) {
        if (m_line_iterator.line().at(0) == 'v') {
          if (m_line_iterator.line().at(1) == 'c') {
            l_line_state = state::ReadColor;
          } else if (m_line_iterator.line().at(1) == 't') {
            l_line_state = state::ReadUv;
          } else if (m_line_iterator.line().at(1) == 'n') {
            l_line_state = state::ReadNormal;
          } else {
            l_line_state = state::ReadPosition;
          }
        } else if (m_line_iterator.line().at(0) == 'f') {
          l_line_state = state::ReadFace;
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
        thiz.m_position_begin = m_line_iterator.line_begin_absolute();
      } else if (p_next == state::ReadColor) {
        thiz.m_color_begin = m_line_iterator.line_begin_absolute();
      } else if (p_next == state::ReadUv) {
        thiz.m_uv_begin = m_line_iterator.line_begin_absolute();
      } else if (p_next == state::ReadNormal) {
        thiz.m_normal_begin = m_line_iterator.line_begin_absolute();
      } else if (p_next == state::ReadFace) {
        thiz.m_face_begin = m_line_iterator.line_begin_absolute();
      }
      m_state = p_next;
    };
  };
};

}; // namespace details

struct obj_mesh_loader {
  mesh compile(const container::range<ui8> &p_raw_obj) {
    return __compile(p_raw_obj);
  };

private:
  mesh __compile(const container::range<ui8> &p_raw_obj) {
    details::obj_mesh_bytes l_mesh_bytes =
        details::obj_mesh_bytes{.m_data = p_raw_obj};
    block_debug([&]() { l_mesh_bytes.validity_pass(); });
    l_mesh_bytes.mesh_header_pass();

    details::mesh_intermediary l_mesh_intermediary;
    l_mesh_intermediary.allocate(
        l_mesh_bytes.m_position_count, l_mesh_bytes.m_color_count,
        l_mesh_bytes.m_uv_count, l_mesh_bytes.m_normal_count,
        l_mesh_bytes.m_face_count);

    l_mesh_bytes.mesh_fill_pass(l_mesh_intermediary);
    mesh l_mesh = l_mesh_intermediary.allocate_mesh();
    l_mesh_intermediary.free();
    return l_mesh;
  };
};

}; // namespace assets