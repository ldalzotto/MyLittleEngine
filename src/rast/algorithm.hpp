#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <m/mat.hpp>
#include <m/vec.hpp>

namespace rast {
namespace algorithm {

struct shader {
  void *m_vertex;
  void *m_fragment;
};

static inline void rasterize(const shader &p_shader, m::vec<ui16, 2> &p_rect,
                             const m::mat<f32, 4, 4> &p_proj,
                             const m::mat<f32, 4, 4> &p_view,
                             const m::mat<f32, 4, 4> &p_transform,
                             const container::range<ui8> &p_index_buffer,
                             bgfx::VertexLayout p_vertex_layout,
                             const container::range<ui8> &p_vertex_buffer,
                             ui64 p_state, ui32 p_rgba
                             /* ,container::range<ui8> &p_target_buffer */) {
  // TODO
  int l_do_magical_stuff_here = 0;
};

} // namespace algorithm
} // namespace rast