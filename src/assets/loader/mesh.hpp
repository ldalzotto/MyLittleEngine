#pragma once

#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <m/math.hpp>
#include <m/vec.hpp>
#include <shared/types.hpp>

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

  container::span<position_t> m_positions;
  container::span<rgb_t> m_colors;
  container::span<uv_t> m_uvs;
  container::span<normal_t> m_normals;
  container::span<vindex_t> m_indices;

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
    m_indices.free();
  };
};

struct obj_mesh_loader {
  mesh compile(const container::range<ui8> &p_raw_obj) {
    return __compile(p_raw_obj);
  };

private:
  mesh __compile(const container::range<ui8> &p_raw_obj);
};

namespace details {

struct mesh_intermediary {

  mesh_composition m_composition;

  struct heap {
    container::span<position_t> m_positions;
    container::span<rgb_t> m_colors;
    container::span<uv_t> m_uvs;
    container::span<normal_t> m_normals;
    container::span<container::arr<container::arr<vindex_t, 4>, 3>> m_faces;
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

  container::span<position_t> &position() { return m_heap.m_positions; };
  container::span<rgb_t> &color() { return m_heap.m_colors; };
  container::span<uv_t> &uv() { return m_heap.m_uvs; };
  container::span<normal_t> &normal() { return m_heap.m_normals; };
  container::span<container::arr<container::arr<vindex_t, 4>, 3>> &face() {
    return m_heap.m_faces;
  };

  mesh allocate_mesh() {
    using face_hash_t = uimax;

    struct index {
      face_hash_t m_hash;
      container::arr<vindex_t, 4> m_attributes;
    };

    container::vector<index> l_unique_indices;

    struct per_face {
      table_span_meta;
      table_cols_2(vindex_t, face_hash_t);
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
        face_hash_t l_hash = 0;
        for (auto l_attribute_it = 0; l_attribute_it < l_attribute_count;
             ++l_attribute_it) {
          l_hash = algorithm::hash_combine(
              l_hash,
              container::range<vindex_t>::make(&l_face.at(l_attribute_it), 1)
                  .cast_to<ui8>());
        }

        vindex_t l_index_found = -1;
        for (auto l_index_it = 0; l_index_it < l_unique_indices.count();
             ++l_index_it) {
          if (l_unique_indices.at(l_index_it).m_hash == l_hash) {
            l_index_found = l_index_it;
            break;
          }
        }

        uimax l_face_index = (l_face_it * 3) + i;
        if (l_index_found == vindex_t(-1)) {
          l_unique_indices.push_back(
              index{.m_hash = l_hash, .m_attributes = l_face});
          l_index_found = l_unique_indices.count() - 1;
        }

        vindex_t *l_face_index_value;
        face_hash_t *l_face_hash;
        l_per_face_indices.at(l_face_index, &l_face_index_value, &l_face_hash);
        *l_face_index_value = l_index_found;
        *l_face_hash = l_hash;
      }
    }

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
      m_line_iterator.for_each_line([&](auto) { process_line(); });
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

inline mesh obj_mesh_loader::__compile(const container::range<ui8> &p_raw_obj) {
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
}

}; // namespace assets