#pragma once

#include <assets/mesh.hpp>
#include <rast/model.hpp>
#include <rast/rast.hpp>
#include <ren/model.hpp>

namespace ren {

template <typename Private> struct ren_api {
  Private &thiz;
  ren_api(Private &p_thiz) : thiz(p_thiz){};

  FORCE_INLINE void allocate() { thiz.allocate(); };

  FORCE_INLINE void free() { thiz.free(); };

  FORCE_INLINE camera_handle camera_create() { return thiz.camera_create(); };
  FORCE_INLINE void camera_set_width_height(camera_handle p_camera,
                                            ui32 p_width, ui32 p_height) {
    thiz.camera_set_width_height(p_camera, p_width, p_height);
  };

  template <typename Rasterizer>
  FORCE_INLINE void camera_set_render_width_height(
      camera_handle p_camera, ui32 p_rendertexture_width,
      ui32 p_rendertexture_height, rast_api<Rasterizer> p_rast) {
    thiz.camera_set_render_width_height(p_camera, p_rendertexture_width,
                                        p_rendertexture_height, p_rast);
  };

  FORCE_INLINE void
  camera_set_projection(camera_handle p_camera,
                        const m::mat<fix32, 4, 4> &p_projection) {
    thiz.camera_set_projection(p_camera, p_projection);
  };

  FORCE_INLINE void camera_set_orthographic(camera_handle p_camera,
                                            fix32 p_width, fix32 p_height,
                                            fix32 p_near, fix32 p_far) {
    thiz.camera_set_orthographic(p_camera, p_width, p_height, p_near, p_far);
  };

  FORCE_INLINE void camera_set_view(camera_handle p_camera,
                                    m::mat<fix32, 4, 4> p_view) {
    thiz.camera_set_view(p_camera, p_view);
  };

  template <typename Rasterizer>
  FORCE_INLINE void camera_destroy(camera_handle p_camera,
                                   rast_api<Rasterizer> p_rast) {
    thiz.camera_destroy(p_camera, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE mesh_handle mesh_create(const assets::mesh &p_mesh,
                                       rast_api<Rasterizer> p_rast) {
    return thiz.mesh_create(p_mesh, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE void mesh_destroy(mesh_handle p_mesh,
                                 rast_api<Rasterizer> p_rast) {
    thiz.mesh_destroy(p_mesh, p_rast);
  };

  FORCE_INLINE material_handle material_create() {
    return thiz.material_create();
  };

  template <typename Rasterizer>
  FORCE_INLINE void material_destroy(material_handle p_material,
                                     rast_api<Rasterizer> p_rast) {
    thiz.material_destroy(p_material, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE void material_push(material_handle p_material,
                                  const char *p_name,
                                  bgfx::UniformType::UniformType::Enum p_type,
                                  rast_api<Rasterizer> p_rast) {
    thiz.material_push(p_material, p_name, p_type, p_rast);
  };

  template <typename ValueType, typename Rasterizer>
  void material_set_vec4(material_handle p_material, uimax p_index,
                         const ValueType &p_value,
                         rast_api<Rasterizer> p_rast) {
    thiz.material_set_vec4(p_material, p_index, p_value, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE program_handle program_create(
      const ren::program_meta &p_program_meta,
      const container::range<rast::shader_uniform> &p_vertex_uniforms,
      const container::range<rast::shader_vertex_output_parameter>
          &p_vertex_output,
      rast::shader_vertex_function p_vertex,
      rast::shader_fragment_function p_fragment, rast_api<Rasterizer> p_rast) {
    return thiz.program_create(p_program_meta, p_vertex_uniforms,
                               p_vertex_output, p_vertex, p_fragment, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE void program_destroy(program_handle p_program,
                                    rast_api<Rasterizer> p_rast) {
    thiz.program_destroy(p_program, p_rast);
  };

  // Pushes a render pass.
  // TODO -> this will evolve in the future ?
  // TODO -> have special allocations for buffer ?
  FORCE_INLINE void
  draw(camera_handle p_camera, program_handle p_shader,
       material_handle p_material,
       const container::range<m::mat<fix32, 4, 4>> &p_transforms,
       const container::range<mesh_handle> &p_meshes) {
    thiz.draw(p_camera, p_shader, p_material, p_transforms, p_meshes);
  };

  template <typename Rasterizer>
  FORCE_INLINE void frame(rast_api<Rasterizer> p_rast) {
    thiz.frame(p_rast);
  };

  FORCE_INLINE rast::image_view frame_view(camera_handle p_camera) {
    return thiz.frame_view(p_camera);
  };
};

}; // namespace ren