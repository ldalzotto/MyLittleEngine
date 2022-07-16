#pragma once

#include <ren/ren.hpp>

namespace ren {
namespace algorithm {

template <typename Renderer, typename Rasterizer>
inline static material_handle material_create_from_shaderdefinition(
    ren_api<Renderer> p_ren, rast_api<Rasterizer> p_rast,
    const container::range<const ui8 *> &p_vertex_uniform_names,
    const container::range<rast::shader_uniform> &p_vertex_uniforms) {

  assert_debug(p_vertex_uniform_names.count() == p_vertex_uniforms.count());
  auto l_material = p_ren.material_create();
  for (auto i = 0; i < p_vertex_uniform_names.count(); ++i) {
    p_ren.material_push(l_material, (const char *)p_vertex_uniform_names.at(i),
                        p_vertex_uniforms.at(i).m_type, p_rast);
  }
  return l_material;
};

template <typename ShaderDefinitionType, typename Renderer, typename Rasterizer>
inline static material_handle
material_create_from_shaderdefinition(ren_api<Renderer> p_ren,
                                      rast_api<Rasterizer> p_rast) {
  return material_create_from_shaderdefinition(
      p_ren, p_rast,
      ShaderDefinitionType::s_meta.m_vertex_uniform_names.range(),
      ShaderDefinitionType::s_meta.m_vertex_uniforms.range());
};

template <typename ShaderDefinitionType, typename Renderer, typename Rasterizer>
inline static ren::program_handle
program_create_from_shaderdefinition(ren_api<Renderer> p_ren,
                                     rast_api<Rasterizer> p_rast,
                                     const ren::program_meta &p_meta) {
  return p_ren.program_create(
      p_meta, ShaderDefinitionType::s_meta.m_vertex_uniforms.range(),
      ShaderDefinitionType::s_meta.m_vertex_output.range(),
      ShaderDefinitionType::vertex, ShaderDefinitionType::fragment, p_rast);
};

} // namespace algorithm
} // namespace ren