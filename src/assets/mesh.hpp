#pragma once

#include <cor/container.hpp>
#include <cor/orm.hpp>
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

  // TODO -> all those vertex attributes can be set as an orm as they are
  // supposed to have the same count
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

    for (auto l_face_index_it = 0; l_face_index_it < l_per_face_indices.count();
         ++l_face_index_it) {
      vindex_t *l_index;
      l_per_face_indices.at(l_face_index_it, &l_index, orm::none());
      l_mesh.m_indices.at(l_face_index_it) = *l_index;
    }

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
} // namespace details

}; // namespace assets