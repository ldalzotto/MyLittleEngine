#pragma once

#include <assets/mesh.hpp>
#include <rast/rast.hpp>

namespace ren {
namespace details {

namespace algorithm {

template <typename Rasterizer>
inline void upload_mesh_to_gpu(rast_api<Rasterizer> p_rast,
                               const assets::mesh &p_mesh,
                               bgfx::VertexBufferHandle *out_vertex_buffer,
                               bgfx::IndexBufferHandle *out_index_buffer) {

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin();
  if (p_mesh.m_composition.m_position) {
    l_vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
  }
  if (p_mesh.m_composition.m_color) {
    l_vertex_layout.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8);
  }
  if (p_mesh.m_composition.m_normal) {
    l_vertex_layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
  }
  if (p_mesh.m_composition.m_uv) {
    l_vertex_layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
  }
  l_vertex_layout.end();

  const bgfx::Memory *l_vertex_index =
      p_rast.alloc(p_mesh.m_indices.size_of(), sizeof(vindex_t));
  container::range<ui8>::make(l_vertex_index->data, l_vertex_index->size)
      .copy_from(p_mesh.m_indices.range());

  auto l_mesh_view = p_mesh.view();

  const bgfx::Memory *l_vertex_buffer = p_rast.alloc(
      l_vertex_layout.getSize(l_mesh_view.m_positions.count()), sizeof(fix32));
  container::range<ui8> l_vertex_buffer_range =
      container::range<ui8>::make(l_vertex_buffer->data, l_vertex_buffer->size);

  auto l_vertex_stride = l_vertex_layout.getStride();
  for (auto i = 0; i < l_mesh_view.m_positions.count(); ++i) {
    if (p_mesh.m_composition.m_position) {
      *(position_t *)l_vertex_buffer_range
           .slide(l_vertex_layout.getOffset(bgfx::Attrib::Position))
           .data() = l_mesh_view.m_positions.at(i);
    }
    if (p_mesh.m_composition.m_color) {
      *(rgb_t *)l_vertex_buffer_range
           .slide(l_vertex_layout.getOffset(bgfx::Attrib::Color0))
           .data() = l_mesh_view.m_colors.at(i);
    }
    if (p_mesh.m_composition.m_normal) {
      *(normal_t *)l_vertex_buffer_range
           .slide(l_vertex_layout.getOffset(bgfx::Attrib::Normal))
           .data() = l_mesh_view.m_normals.at(i);
    }
    if (p_mesh.m_composition.m_uv) {
      *(uv_t *)l_vertex_buffer_range
           .slide(l_vertex_layout.getOffset(bgfx::Attrib::TexCoord0))
           .data() = l_mesh_view.m_uvs.at(i);
    }

    l_vertex_buffer_range.slide_self(l_vertex_stride);
  }

  *out_index_buffer = p_rast.createIndexBuffer(l_vertex_index);
  *out_vertex_buffer =
      p_rast.createVertexBuffer(l_vertex_buffer, l_vertex_layout);
};

}; // namespace algorithm

}; // namespace details

}; // namespace ren