#pragma once

#include <m/geom.hpp>
#include <rast/model.hpp>
#include <ren/impl/algorithm.hpp>
#include <ren/model.hpp>
#include <ren/ren.hpp>

namespace ren {
namespace details {

static constexpr bgfx::TextureFormat::Enum s_camera_rgb_format =
    bgfx::TextureFormat::RGB8;
static constexpr bgfx::TextureFormat::Enum s_camera_depth_format =
    bgfx::TextureFormat::D32F;

struct ren_impl {

  // TODO -> this should evolve in the future.
  // The camera should be able to be linked to multiple shader.
  struct render_pass {
    camera_handle m_camera;
    shader_handle m_shader;
    container::vector<m::mat<fix32, 4, 4>> m_transforms;
    container::vector<mesh_handle> m_meshes;

    void allocate(camera_handle p_camera, shader_handle p_shader,
                  const container::range<m::mat<fix32, 4, 4>> &p_transforms,
                  const container::range<mesh_handle> &p_meshes) {
      m_camera = p_camera;
      m_shader = p_shader;
      m_transforms.allocate(p_transforms.count());
      m_transforms.count() = p_transforms.count();
      m_transforms.range().copy_from(p_transforms);
      m_meshes.allocate(p_meshes.count());
      m_meshes.count() = p_meshes.count();
      m_meshes.range().copy_from(p_meshes);
    };

    void free() {
      m_transforms.free();
      m_meshes.free();
    };
  };

  struct heap {

    orm::table_pool_v2<camera, bgfx::FrameBufferHandle> m_camera_table;
    orm::table_pool_v2<shader, bgfx::ProgramHandle> m_shader_table;
    orm::table_pool_v2<mesh, bgfx::VertexBufferHandle, bgfx::IndexBufferHandle>
        m_mesh_table;

    container::vector<render_pass> m_render_passes;

    // TODO -> add render passes per shader
    void allocate() {
      m_camera_table.allocate(0);
      m_shader_table.allocate(0);
      m_mesh_table.allocate(0);
      m_render_passes.allocate(0);
    };

    void free() {
      m_camera_table.free();
      m_shader_table.free();
      m_mesh_table.free();
      m_render_passes.free();
    };

  } m_heap;

  void allocate() { m_heap.allocate(); };

  void free() { m_heap.free(); };

  camera_handle camera_create() {
    return {
        m_heap.m_camera_table.push_back(camera(), bgfx::FrameBufferHandle())};
  };

  void camera_set_width_height(camera_handle p_camera, ui32 p_width,
                               ui32 p_height) {
    camera *l_camera;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, none());
    l_camera->m_width = p_width;
    l_camera->m_height = p_height;
  };

  template <typename Rasterizer>
  void camera_set_render_width_height(camera_handle p_camera,
                                      ui32 p_rendertexture_width,
                                      ui32 p_rendertexture_height,
                                      rast_api<Rasterizer> p_rast) {

    camera *l_camera;
    bgfx::FrameBufferHandle *l_frame_buffer;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, &l_frame_buffer);
    l_camera->m_rendertexture_width = p_rendertexture_width;
    l_camera->m_rendertexture_height = p_rendertexture_height;

    *l_frame_buffer = p_rast.createFrameBuffer(
        0, l_camera->m_rendertexture_width, l_camera->m_rendertexture_height,
        s_camera_rgb_format, s_camera_depth_format);
  };

  void camera_set_perspective(camera_handle p_camera, fix32 p_fov, fix32 p_near,
                              fix32 p_far) {
    camera *l_camera;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, none());
    l_camera->m_projection = m::perspective(
        p_fov, fix32(l_camera->m_width) / l_camera->m_height, p_near, p_far);
  };

  void camera_set_view(camera_handle p_camera, m::mat<fix32, 4, 4> p_view) {
    camera *l_camera;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, none());
    l_camera->m_view = p_view;
  };

  template <typename Rasterizer>
  void camera_destroy(camera_handle p_camera, rast_api<Rasterizer> p_rast) {
    bgfx::FrameBufferHandle *l_frame_buffer;
    m_heap.m_camera_table.at(p_camera.m_idx, none(), &l_frame_buffer);
    p_rast.destroy(*l_frame_buffer);
    m_heap.m_camera_table.remove_at(p_camera.m_idx);
  };

  template <typename Rasterizer>
  mesh_handle create_mesh(const assets::mesh &p_mesh,
                          rast_api<Rasterizer> p_rast) {
    bgfx::VertexBufferHandle l_vertex_buffer;
    bgfx::IndexBufferHandle l_index_buffer;
    algorithm::upload_mesh_to_gpu(p_rast, p_mesh, &l_vertex_buffer,
                                  &l_index_buffer);
    uimax l_index = m_heap.m_mesh_table.push_back(ren::mesh{}, l_vertex_buffer,
                                                  l_index_buffer);
    return mesh_handle{.m_idx = l_index};
  };

  template <typename Rasterizer>
  void destroy_mesh(mesh_handle p_mesh, rast_api<Rasterizer> p_rast) {
    bgfx::VertexBufferHandle *l_vertex_buffer;
    bgfx::IndexBufferHandle *l_index_buffer;
    m_heap.m_mesh_table.at(p_mesh.m_idx, none(), &l_vertex_buffer,
                           &l_index_buffer);
    p_rast.destroy(*l_vertex_buffer);
    p_rast.destroy(*l_index_buffer);
    m_heap.m_mesh_table.remove_at(p_mesh.m_idx);
  };

  template <typename Rasterizer>
  shader_handle
  create_shader(const container::range<rast::shader_vertex_output_parameter>
                    &p_vertex_output,
                rast::shader_vertex_function p_vertex,
                rast::shader_fragment_function p_fragment,
                rast_api<Rasterizer> p_rast) {
    uimax l_vertex_shader_size = rast::shader_vertex_bytes::byte_size(1);
    const bgfx::Memory *l_vertex_shader_memory =
        p_rast.alloc(l_vertex_shader_size);
    rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
        p_vertex_output, p_vertex);

    const bgfx::Memory *l_fragment_shader_memory =
        p_rast.alloc(rast::shader_fragment_bytes::byte_size());
    rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
        p_fragment);

    bgfx::ShaderHandle l_vertex = p_rast.createShader(l_vertex_shader_memory);
    bgfx::ShaderHandle l_fragment =
        p_rast.createShader(l_fragment_shader_memory);

    bgfx::ProgramHandle l_program = p_rast.createProgram(l_vertex, l_fragment);
    uimax l_index = m_heap.m_shader_table.push_back(ren::shader{}, l_program);

    return shader_handle{.m_idx = l_index};
  };

  void draw(camera_handle p_camera, shader_handle p_shader,
            const container::range<m::mat<fix32, 4, 4>> &p_transforms,
            const container::range<mesh_handle> &p_meshes) {
    render_pass l_render_pass;
    l_render_pass.allocate(p_camera, p_shader, p_transforms, p_meshes);
    m_heap.m_render_passes.push_back(l_render_pass);
  };

  template <typename Rasterizer>
  void destroy_shader(shader_handle p_shader, rast_api<Rasterizer> p_rast) {
    bgfx::ProgramHandle *l_program;
    m_heap.m_shader_table.at(p_shader.m_idx, none(), &l_program);
    p_rast.destroy(*l_program);
    m_heap.m_shader_table.remove_at(p_shader.m_idx);
  };

  template <typename Rasterizer> void frame(rast_api<Rasterizer> p_rast) {
    for_each_renderpass([&](render_pass &p_render_pass) {
      camera *l_camera;
      bgfx::FrameBufferHandle *l_frame_buffer;
      m_heap.m_camera_table.at(p_render_pass.m_camera.m_idx, &l_camera,
                               &l_frame_buffer);

      // TODO -> having conditionals depneding if the frame buffer have depth ?
      p_rast.setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
      p_rast.setViewRect(0, 0, 0, l_camera->m_width, l_camera->m_height);
      p_rast.setViewTransform(0, l_camera->m_view.m_data,
                              l_camera->m_projection.m_data);
      p_rast.setViewFrameBuffer(0, *l_frame_buffer);

      // p_render_pass.m_shader;
      for (auto l_mesh_it = 0; l_mesh_it < p_render_pass.m_meshes.count();
           ++l_mesh_it) {
        bgfx::VertexBufferHandle *l_vertex_buffer;
        bgfx::IndexBufferHandle *l_index_buffer;
        m_heap.m_mesh_table.at(p_render_pass.m_meshes.at(l_mesh_it).m_idx,
                               none(), &l_vertex_buffer, &l_index_buffer);

        m::mat<fix32, 4, 4> l_transform =
            p_render_pass.m_transforms.at(l_mesh_it);

        p_rast.setTransform(l_transform.m_data);

        p_rast.setIndexBuffer(*l_index_buffer);
        p_rast.setVertexBuffer(0, *l_vertex_buffer);
        p_rast.setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z |
                        BGFX_STATE_CULL_CW);
      }

      bgfx::ProgramHandle *l_program_handle;
      m_heap.m_shader_table.at(p_render_pass.m_shader.m_idx, none(),
                               &l_program_handle);
      p_rast.submit(0, *l_program_handle);
    });
  };

  template <typename Rasterizer>
  rast::image_view frame_view(camera_handle p_camera,
                              rast_api<Rasterizer> p_rast) {
    camera *l_camera;
    bgfx::FrameBufferHandle *l_frame_buffer;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, &l_frame_buffer);
    return rast::image_view(
        l_camera->m_width, l_camera->m_height,
        textureformat_to_pixel_size(s_camera_rgb_format),
        p_rast.fetchTextureSync(p_rast.getTexture(*l_frame_buffer)));
  };

private:
  template <typename CallbackFunc>
  void for_each_renderpass(const CallbackFunc &p_cb) {
    for (auto i = 0; i < m_heap.m_render_passes.count(); ++i) {
      render_pass &l_render_pass = m_heap.m_render_passes.at(i);
      p_cb(l_render_pass);
      l_render_pass.free();
    }
    m_heap.m_render_passes.clear();
  };
};

}; // namespace details

}; // namespace ren
