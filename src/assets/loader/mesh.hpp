#pragma once

#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <m/math.hpp>
#include <m/vec.hpp>

namespace assets {

struct mesh_composition {
  ui8 m_position : 1;
  ui8 m_color : 1;
  ui8 m_uv : 1;
  ui8 m_normal : 1;
};

enum class mesh_attribute_type : ui8 { undefined, position, color, uv, normal };

struct mesh {
  mesh_composition m_composition;

  container::span<m::vec<fix32, 3>> m_positions;
  container::span<m::vec<ui8, 3>> m_colors;
  container::span<m::vec<fix32, 2>> m_uvs;
  container::span<m::vec<fix32, 3>> m_normals;
  container::span<ui32> m_indices;

  void allocate(mesh_composition p_composition, uimax p_unique_indices_count,
                uimax p_face_indices_count) {
    m_composition = p_composition;
    if (m_composition.m_position) {
      m_positions.allocate(p_unique_indices_count);
    }
    if (m_composition.m_color) {
      m_colors.allocate(p_unique_indices_count);
    }
    if (m_composition.m_uv) {
      m_uvs.allocate(p_unique_indices_count);
    }
    if (m_composition.m_normal) {
      m_normals.allocate(p_unique_indices_count);
    }
    m_indices.allocate(p_face_indices_count);
  };

  void free() {
    if (m_composition.m_position) {
      m_positions.free();
    }
    if (m_composition.m_color) {
      m_colors.free();
    }
    if (m_composition.m_uv) {
      m_uvs.free();
    }
    if (m_composition.m_normal) {
      m_normals.free();
    }
  };
};

namespace details {

struct mesh_intermediary {

  mesh_composition m_composition;

  struct heap {
    container::span<m::vec<fix32, 3>> m_positions;
    container::span<m::vec<ui8, 3>> m_colors;
    container::span<m::vec<fix32, 2>> m_uvs;
    container::span<m::vec<fix32, 3>> m_normals;
    container::span<container::arr<container::arr<ui32, 4>, 3>> m_faces;
  } m_heap;

  void allocate(uimax p_position_count, uimax p_color_count, uimax p_uv_count,
                uimax p_normal_count, uimax p_face_count) {
    m_heap.m_positions.allocate(p_position_count);
    m_heap.m_colors.allocate(p_color_count);
    m_heap.m_uvs.allocate(p_uv_count);
    m_heap.m_normals.allocate(p_normal_count);
    m_heap.m_faces.allocate(p_face_count);
    m_composition.m_position = p_position_count > 0;
    m_composition.m_color = p_color_count > 0;
    m_composition.m_uv = p_uv_count > 0;
    m_composition.m_normal = p_normal_count > 0;
  };

  void free() {
    m_heap.m_positions.free();
    m_heap.m_colors.free();
    m_heap.m_uvs.free();
    m_heap.m_normals.free();
    m_heap.m_faces.free();
  };

  container::span<m::vec<fix32, 3>> &position() { return m_heap.m_positions; };
  container::span<m::vec<ui8, 3>> &color() { return m_heap.m_colors; };
  container::span<m::vec<fix32, 2>> &uv() { return m_heap.m_uvs; };
  container::span<m::vec<fix32, 3>> &normal() { return m_heap.m_normals; };
  container::span<container::arr<container::arr<ui32, 4>, 3>> &face() {
    return m_heap.m_faces;
  };

  mesh allocate_mesh() {
    using index_value = ui32;
    using face_hash = uimax;
    //     using vertex_face_index = uimax;

    struct index {
      face_hash m_hash;
      container::arr<ui32, 4> m_attributes;
    };

    container::vector<index> l_unique_indices;

    struct per_face {
      table_span_meta;
      table_cols_2(index_value, face_hash);
      table_define_span_2;
    } l_per_face_indices;

    l_unique_indices.allocate(0);
    l_per_face_indices.allocate(face().count() * 3);

    container::arr<mesh_attribute_type, 4> m_attributes = {
        mesh_attribute_type::undefined, mesh_attribute_type::undefined,
        mesh_attribute_type::undefined, mesh_attribute_type::undefined};

    ui8 l_attribute_count = 0;
    if (m_composition.m_position) {
      m_attributes.at(l_attribute_count) = mesh_attribute_type::position;
      l_attribute_count += 1;
    }
    if (m_composition.m_color) {
      m_attributes.at(l_attribute_count) = mesh_attribute_type::color;
      l_attribute_count += 1;
    }
    if (m_composition.m_uv) {
      m_attributes.at(l_attribute_count) = mesh_attribute_type::uv;
      l_attribute_count += 1;
    }
    if (m_composition.m_normal) {
      m_attributes.at(l_attribute_count) = mesh_attribute_type::normal;
      l_attribute_count += 1;
    }

    for (auto l_face_it = 0; l_face_it < m_heap.m_faces.count(); ++l_face_it) {
      for (auto i = 0; i < 3; ++i) {
        auto &l_face = m_heap.m_faces.at(l_face_it).at(i);
        face_hash l_hash = 0;
        for (auto l_attribute_it = 0; l_attribute_it < l_attribute_count;
             ++l_attribute_it) {
          l_hash = algorithm::hash_combine(
              l_hash,
              container::range<ui32>::make(&l_face.at(l_attribute_it), 1)
                  .cast_to<ui8>());
        }

        ui32 l_index_found = -1;
        for (auto l_index_it = 0; l_index_it < l_unique_indices.count();
             ++l_index_it) {
          if (l_unique_indices.at(l_index_it).m_hash == l_hash) {
            l_index_found = l_index_it;
            break;
          }
        }

        uimax l_face_index = (l_face_it * 3) + i;
        if (l_index_found == -1) {
          l_unique_indices.push_back(
              index{.m_hash = l_hash, .m_attributes = l_face});
          l_index_found = l_unique_indices.count() - 1;
        }

        index_value *l_face_index_value;
        face_hash *l_face_hash;
        l_per_face_indices.at(l_face_index, &l_face_index_value, &l_face_hash);
        *l_face_index_value = l_index_found;
        *l_face_hash = l_hash;
      }
    }

    // TODO -> create mesh and fill values
    mesh l_mesh;
    l_mesh.allocate(m_composition, l_unique_indices.count(),
                    l_per_face_indices.count());

    // fill unique indices
    for (auto l_unique_index_it = 0;
         l_unique_index_it < l_unique_indices.count(); ++l_unique_index_it) {
      index &l_index = l_unique_indices.at(l_unique_index_it);
      for (auto l_attribute_it = 0; l_attribute_it < l_attribute_count;
           ++l_attribute_it) {
        auto l_attrubute_index = l_index.m_attributes.at(l_attribute_it);
        mesh_attribute_type l_attribute_type = m_attributes.at(l_attribute_it);
        if (l_attribute_type == mesh_attribute_type::position) {
          l_mesh.m_positions.at(l_unique_index_it) =
              position().at(l_attrubute_index);
        } else if (l_attribute_type == mesh_attribute_type::color) {
          l_mesh.m_colors.at(l_unique_index_it) = color().at(l_attrubute_index);
        } else if (l_attribute_type == mesh_attribute_type::uv) {
          l_mesh.m_uvs.at(l_unique_index_it) = uv().at(l_attrubute_index);
        } else if (l_attribute_type == mesh_attribute_type::normal) {
          l_mesh.m_normals.at(l_unique_index_it) =
              normal().at(l_attrubute_index);
        }
      }
    }

    // fill face indices

    // for(auto l_index)

    l_per_face_indices.free();
    l_unique_indices.free();
    return l_mesh;
  };
};

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

    uimax m_iterator;
    container::range<const ui8> m_line;

    enum class state {
      Undefined = 0,
      ReadPosition = 1,
      ReadColor = 2,
      ReadUv = 3,
      ReadNormal = 4,
      ReadFace = 5
    } m_state;

    void mesh_header_pass() {
      m_iterator = 0;
      m_state = state::Undefined;
      while (next_line()) {
        process_line();
      }
    };

    void mesh_fill_pass(mesh_intermediary &p_mesh_intermediary) {

      // position
      if (p_mesh_intermediary.m_composition.m_position) {
        auto l_mesh_vertices = p_mesh_intermediary.position();
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
      if (p_mesh_intermediary.m_composition.m_color) {
        auto l_mesh_color = p_mesh_intermediary.color();
        uimax l_line_count = 0;
        m_iterator = thiz.m_color_begin;
        while (l_line_count < thiz.m_color_count) {
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
      if (p_mesh_intermediary.m_composition.m_uv) {
        auto l_mesh_uv = p_mesh_intermediary.uv();
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
      if (p_mesh_intermediary.m_composition.m_normal) {
        auto l_mesh_normals = p_mesh_intermediary.normal();
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

      // faces
      if (p_mesh_intermediary.face().count() > 0) {
        auto l_faces = p_mesh_intermediary.face();
        uimax l_line_count;
        m_iterator = thiz.m_face_begin;
        while (l_line_count < thiz.m_face_count) {
          next_line();
          auto l_face_line = m_line.slide(2);
          auto l_white_space_it = 0;
          while (l_face_line.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_f1 = l_face_line.shrink_to(l_white_space_it);
          l_face_line.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_face_line.at(l_white_space_it) != ' ') {
            l_white_space_it += 1;
          }
          auto l_f2 = l_face_line.shrink_to(l_white_space_it);
          l_face_line.slide_self(l_white_space_it + 1);
          l_white_space_it = 0;
          while (l_white_space_it != l_face_line.count()) {
            l_white_space_it += 1;
          }
          auto l_f3 = l_face_line.shrink_to(l_white_space_it);

          const auto l_face = container::arr<container::arr<ui32, 4>, 3>{
              extract_face_indices(l_f1), extract_face_indices(l_f2),
              extract_face_indices(l_f3)};
          l_faces.at(l_line_count) = l_face;
          l_line_count += 1;
        }
      }
    };

    static container::arr<ui32, 4>
    extract_face_indices(const container::range<const ui8> &p_str) {
      container::arr<ui32, 4> l_out;
      l_out.range().zero();
      ui8 l_out_index = 0;

      uimax l_it = 0;
      uimax l_begin = l_it;
      uimax l_end = l_begin;
      while (true) {
        if (l_it == p_str.count() || p_str.at(l_it) == '/') {
          l_end = l_it;
          auto l_range = p_str.slide(l_begin).shrink_to(l_end - l_begin);
          l_out.at(l_out_index) =
              sys::stoui<ui32>(l_range.data(), l_range.count()) - 1;
          l_out_index += 1;
          l_begin = l_it + 1;
          l_end = l_begin;
        }

        if (l_it == p_str.count()) {
          break;
        }

        l_it += 1;
      }

      return l_out;
    };

    void process_line() {
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
        } else if (m_line.at(0) == 'f') {
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
        thiz.m_position_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadColor) {
        thiz.m_color_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadUv) {
        thiz.m_uv_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadNormal) {
        thiz.m_normal_begin = m_iterator - m_line.count() - 1;
      } else if (p_next == state::ReadFace) {
        thiz.m_face_begin = m_iterator - m_line.count() - 1;
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

}; // namespace details

struct obj_mesh_loader {
  mesh compile(const container::range<ui8> &p_raw_obj) {
    return __compile(p_raw_obj);
  };

private:
  mesh __compile(const container::range<ui8> &p_raw_obj) {
    details::obj_mesh_bytes l_mesh_bytes =
        details::obj_mesh_bytes{.m_data = p_raw_obj};
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