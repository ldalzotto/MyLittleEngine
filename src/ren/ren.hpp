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

  // TODO -> remove that
  template <typename Rasterizer>
  FORCE_INLINE camera_handle camera_create(const camera &p_camera,
                                           rast_api<Rasterizer> p_rast) {
    return thiz.camera_create(p_camera, p_rast);
  };

#if 1

  FORCE_INLINE camera_handle camera_create() { return thiz.camera_create(); };
  FORCE_INLINE void camera_set_width_height(camera_handle p_camera,
                                            ui32 p_width, ui32 p_height) {
    thiz.camera_set_width_height(p_camera, p_width, p_height);
  };

  template <typename Rasterizer>
  FORCE_INLINE void
  camera_set_render_width_height(camera_handle p_camera,
                                 ui32 p_rendertexture_width,
                                 ui32 p_rendertexture_height, rast_api<Rasterizer> p_rast) {
    thiz.camera_set_render_width_height(p_camera, p_rendertexture_width,
                                        p_rendertexture_height, p_rast);
  };

#endif

  template <typename Rasterizer>
  FORCE_INLINE void camera_destroy(camera_handle p_camera,
                                   rast_api<Rasterizer> p_rast) {
    thiz.camera_destroy(p_camera, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE mesh_handle create_mesh(const assets::mesh &p_mesh,
                                       rast_api<Rasterizer> p_rast) {
    return thiz.create_mesh(p_mesh, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE void destroy(mesh_handle p_mesh, rast_api<Rasterizer> p_rast) {
    thiz.destroy_mesh(p_mesh, p_rast);
  };

  // TODO -> having a version with shader assets ?
  template <typename Rasterizer>
  FORCE_INLINE shader_handle create_shader(
      const container::range<rast::shader_vertex_output_parameter>
          &p_vertex_output,
      rast::shader_vertex_function p_vertex,
      rast::shader_fragment_function p_fragment, rast_api<Rasterizer> p_rast) {
    return thiz.create_shader(p_vertex_output, p_vertex, p_fragment, p_rast);
  };

  template <typename Rasterizer>
  FORCE_INLINE void destroy(shader_handle p_shader,
                            rast_api<Rasterizer> p_rast) {
    thiz.destroy_shader(p_shader, p_rast);
  };

  // Pushes a render pass.
  // TODO -> this will evolve in the future ?
  // TODO -> have special allocations for buffer ?
  FORCE_INLINE void
  draw(camera_handle p_camera, shader_handle p_shader,
       const container::range<m::mat<fix32, 4, 4>> &p_transforms,
       const container::range<mesh_handle> &p_meshes) {
    thiz.draw(p_camera, p_shader, p_transforms, p_meshes);
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