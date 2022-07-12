#pragma once

#include <ren/ren.hpp>

namespace ren {
namespace algorithm {

template <typename Renderer, typename Rasterizer>
inline static material_handle create_material_from_shader(
    ren_api<Renderer> p_ren, rast_api<Rasterizer> p_rast,
    const container::range<ui8 *> &p_vertex_uniform_names,
    const container::range<rast::shader_uniform> &p_vertex_uniforms) {

  assert_debug(p_vertex_uniform_names.count() == p_vertex_uniforms.count());
  auto l_material = p_ren.material_create();
  for (auto i = 0; i < p_vertex_uniform_names.count(); ++i) {
    p_ren.material_push(l_material, (const char *)p_vertex_uniform_names.at(i),
                        p_vertex_uniforms.at(i).m_type, p_rast);
  }
  return l_material;
};

} // namespace algorithm
} // namespace ren