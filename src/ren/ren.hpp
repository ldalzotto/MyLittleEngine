#pragma once

#include <assets/mesh.hpp>
#include <rast/model.hpp>
#include <rast/rast.hpp>
#include <ren/model.hpp>

namespace ren {

template <typename Private> struct ren_api {
  Private &thiz;
  ren_api(Private &p_thiz) : thiz(p_thiz){};

  template <typename Rasterizer>
  FORCE_INLINE void allocate(rast_api<Rasterizer> p_rasterizer) {
    thiz.allocate(p_rasterizer);
  };

  FORCE_INLINE void free() { thiz.free(); };

  FORCE_INLINE camera_handle create_camera(const camera &p_camera) {
    return thiz.create_camera(p_camera);
  };

  FORCE_INLINE void destroy(camera_handle p_camera) {
    thiz.destroy_camera(p_camera);
  };

  FORCE_INLINE mesh_handle create_mesh(const assets::mesh &p_mesh) {
    return thiz.create_mesh(p_mesh);
  };

  FORCE_INLINE void destroy(mesh_handle p_mesh) { thiz.destroy_mesh(p_mesh); };

  // TODO -> having a version with shader assets ?
  FORCE_INLINE shader_handle
  create_shader(const container::range<rast::shader_vertex_output_parameter>
                    &p_vertex_output,
                rast::shader_vertex_function p_vertex,
                rast::shader_fragment_function p_fragment) {
    return thiz.create_shader(p_vertex_output, p_vertex, p_fragment);
  };

  FORCE_INLINE void destroy(shader_handle p_shader) {
    thiz.destroy_shader(p_shader);
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

  FORCE_INLINE void frame() { thiz.frame(); };

  FORCE_INLINE rast::image_view frame_view(camera_handle p_camera) {
    return thiz.frame_view(p_camera);
  };
};

}; // namespace ren