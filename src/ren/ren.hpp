#pragma once

#include <assets/mesh.hpp>
#include <rast/model.hpp>
#include <ren/model.hpp>

namespace ren {

struct ren_handle {
  void *m_ptr;

  void allocate();
  void free();

  camera_handle create_camera(const camera &p_camera);
  void destroy(camera_handle p_camera);
  mesh_handle create_mesh(const assets::mesh &p_mesh);
  void destroy(mesh_handle p_mesh);

  // TODO -> having a version with shader assets ?
  shader_handle
  create_shader(const container::range<rast::shader_vertex_output_parameter>
                    &p_vertex_output,
                rast::shader_vertex_function, rast::shader_fragment_function);
  void destroy(shader_handle);

  // Pushes a render pass.
  // TODO -> this will evolve in the future ?
  // TODO -> have special allocations for buffer ?
  void draw(camera_handle, shader_handle,
            const container::range<m::mat<fix32, 4, 4>> &p_transforms,
            const container::range<mesh_handle> &p_meshes);

  void frame();

  rast::image_view frame_view(camera_handle p_camera);
};

extern ren_handle ren_handle_allocate();
extern void ren_handle_free(ren_handle);

}; // namespace ren