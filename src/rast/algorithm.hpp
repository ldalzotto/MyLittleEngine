#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <m/mat.hpp>
#include <m/vec.hpp>

namespace rast {
namespace algorithm {

struct program {
  void *m_vertex;
  void *m_fragment;
};

static inline void rasterize(const program &p_program, m::vec<ui16, 2> &p_rect,
                             const m::mat<f32, 4, 4> &p_proj,
                             const m::mat<f32, 4, 4> &p_view,
                             const m::mat<f32, 4, 4> &p_transform,
                             const container::range<ui8> &p_index_buffer,
                             bgfx::VertexLayout p_vertex_layout,
                             const container::range<ui8> &p_vertex_buffer,
                             ui64 p_state, ui32 p_rgba,
                             const bgfx::TextureInfo &p_target_info,
                             container::range<ui8> &p_target_buffer) {

  ui16 l_vertex_stride = p_vertex_layout.getStride();
  uimax l_vertex_count = p_vertex_buffer.count() / l_vertex_stride;
  assert_debug(p_vertex_layout.getSize(l_vertex_count) ==
               p_vertex_buffer.count());

  ui8 l_position_num;
  bgfx::AttribType::Enum l_position_type;
  bool l_position_normalized;
  bool l_position_as_int;
  p_vertex_layout.decode(bgfx::Attrib::Enum::Position, l_position_num,
                         l_position_type, l_position_normalized,
                         l_position_as_int);

  assert_debug(l_position_num == 3);

  m::mat<f32, 4, 4> l_local_to_screen = p_transform * p_view * p_proj;

  container::span<m::vec<f32, 2>> l_screen_vertices;
  l_screen_vertices.allocate(l_vertex_count);

  for (auto i = 0; i < l_vertex_count; ++i) {
    ui8 *l_vertex_bytes = p_vertex_buffer.m_begin + (i * l_vertex_stride);
    m::vec<f32, 4> l_vertex_vec = *(m::vec<f32, 4> *)l_vertex_bytes;
    l_vertex_vec.at(3) = 1;
    m::vec<f32, 4> l_vertex_screen = l_local_to_screen * l_vertex_vec;
    l_screen_vertices.at(i) = {l_vertex_screen.at(0), l_vertex_screen.at(1)};
  }

  // container::span<>

  static const ui8 l_index_stride = sizeof(ui16);

  l_screen_vertices.free();
};

} // namespace algorithm
} // namespace rast