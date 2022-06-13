#pragma once

#include <rast/model.hpp>
#include <ren/algorithm.hpp>
#include <ren/model.hpp>
#include <ren/ren.hpp>

namespace ren {
namespace details {

struct ren_implementations {

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

  camera_handle create_camera(const camera &p_camera) {
    bgfx::FrameBufferHandle l_frame_buffer = bgfx::createFrameBuffer(
        0, p_camera.m_rendertexture_width, p_camera.m_rendertexture_height,
        bgfx::TextureFormat::RGB8, bgfx::TextureFormat::D32F);
    uimax l_index = m_heap.m_camera_table.push_back(p_camera, l_frame_buffer);
    return camera_handle{.m_idx = l_index};
  };

  void destroy_camera(camera_handle p_camera) {
    bgfx::FrameBufferHandle *l_frame_buffer;
    m_heap.m_camera_table.at(p_camera.m_idx, none(), &l_frame_buffer);
    bgfx::destroy(*l_frame_buffer);
    m_heap.m_camera_table.remove_at(p_camera.m_idx);
  };

  mesh_handle create_mesh(const assets::mesh &p_mesh) {
    bgfx::VertexBufferHandle l_vertex_buffer;
    bgfx::IndexBufferHandle l_index_buffer;
    ren::algorithm::upload_mesh_to_gpu(p_mesh, &l_vertex_buffer,
                                       &l_index_buffer);
    uimax l_index = m_heap.m_mesh_table.push_back(ren::mesh{}, l_vertex_buffer,
                                                  l_index_buffer);
    return mesh_handle{.m_idx = l_index};
  };

  void destroy_mesh(mesh_handle p_mesh) {
    bgfx::VertexBufferHandle *l_vertex_buffer;
    bgfx::IndexBufferHandle *l_index_buffer;
    m_heap.m_mesh_table.at(p_mesh.m_idx, none(), &l_vertex_buffer,
                           &l_index_buffer);
    bgfx::destroy(*l_vertex_buffer);
    bgfx::destroy(*l_index_buffer);
    m_heap.m_mesh_table.remove_at(p_mesh.m_idx);
  };

  shader_handle
  create_shader(const container::range<rast::shader_vertex_output_parameter>
                    &p_vertex_output,
                rast::shader_vertex_function p_vertex,
                rast::shader_fragment_function p_fragment) {
    uimax l_vertex_shader_size = rast::shader_vertex_bytes::byte_size(1);
    const bgfx::Memory *l_vertex_shader_memory =
        bgfx::alloc(l_vertex_shader_size);
    rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
        p_vertex_output, p_vertex);

    const bgfx::Memory *l_fragment_shader_memory =
        bgfx::alloc(rast::shader_fragment_bytes::byte_size());
    rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
        p_fragment);

    bgfx::ShaderHandle l_vertex = bgfx::createShader(l_vertex_shader_memory);
    bgfx::ShaderHandle l_fragment =
        bgfx::createShader(l_fragment_shader_memory);

    bgfx::ProgramHandle l_program = bgfx::createProgram(l_vertex, l_fragment);
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

  void destroy_shader(shader_handle p_shader) {
    bgfx::ProgramHandle *l_program;
    m_heap.m_shader_table.at(p_shader.m_idx, none(), &l_program);
    bgfx::destroy(*l_program);
    m_heap.m_shader_table.remove_at(p_shader.m_idx);
  };

  void frame() {
    for_each_renderpass([&](render_pass &p_render_pass) {
      camera *l_camera;
      bgfx::FrameBufferHandle *l_frame_buffer;
      m_heap.m_camera_table.at(p_render_pass.m_camera.m_idx, &l_camera,
                               &l_frame_buffer);

      // TODO -> having conditionals depneding if the frame buffer have depth ?
      bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
      bgfx::setViewRect(0, 0, 0, l_camera->m_width, l_camera->m_height);
      bgfx::setViewTransform(0, l_camera->m_view.m_data,
                             l_camera->m_projection.m_data);
      bgfx::setViewFrameBuffer(0, *l_frame_buffer);

      // p_render_pass.m_shader;
      for (auto l_mesh_it = 0; l_mesh_it < p_render_pass.m_meshes.count();
           ++l_mesh_it) {
        bgfx::VertexBufferHandle *l_vertex_buffer;
        bgfx::IndexBufferHandle *l_index_buffer;
        m_heap.m_mesh_table.at(p_render_pass.m_meshes.at(l_mesh_it).m_idx,
                               none(), &l_vertex_buffer, &l_index_buffer);

        m::mat<fix32, 4, 4> l_transform =
            p_render_pass.m_transforms.at(l_mesh_it);

        bgfx::setTransform(l_transform.m_data);

        bgfx::setIndexBuffer(*l_index_buffer);
        bgfx::setVertexBuffer(0, *l_vertex_buffer);
        bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z |
                       BGFX_STATE_CULL_CW);
      }

      bgfx::ProgramHandle *l_program_handle;
      m_heap.m_shader_table.at(p_render_pass.m_shader.m_idx, none(),
                               &l_program_handle);
      bgfx::submit(0, *l_program_handle);
    });
  };

  rast::image_view frame_view(camera_handle p_camera) {
    bgfx::FrameBufferHandle *l_frame_buffer;
    m_heap.m_camera_table.at(p_camera.m_idx, none(), &l_frame_buffer);

    // TODO -> use bgfx api ?
    auto *l_texture =
        s_bgfx_impl.proxy().FrameBuffer(*l_frame_buffer).RGBTexture().value();
    return rast::image_view(l_texture->info.width, l_texture->info.height,
                            l_texture->info.bitsPerPixel, l_texture->range());
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

namespace ren {

inline void ren_handle::allocate() {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->allocate();
};

inline void ren_handle::free() {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->free();
};

inline void ren_handle::frame() {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->frame();
};

inline void
ren_handle::draw(camera_handle p_camera, shader_handle p_shader,
                 const container::range<m::mat<fix32, 4, 4>> &p_transforms,
                 const container::range<mesh_handle> &p_meshes) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->draw(p_camera, p_shader, p_transforms, p_meshes);
};

inline rast::image_view ren_handle::frame_view(camera_handle p_camera) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  return l_ren->frame_view(p_camera);
};

inline camera_handle ren_handle::create_camera(const camera &p_camera) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  return l_ren->create_camera(p_camera);
};

inline void ren_handle::destroy(camera_handle p_camera) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->destroy_camera(p_camera);
};

inline mesh_handle ren_handle::create_mesh(const assets::mesh &p_mesh) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  return l_ren->create_mesh(p_mesh);
};

inline void ren_handle::destroy(mesh_handle p_mesh) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->destroy_mesh(p_mesh);
};

inline shader_handle ren_handle::create_shader(
    const container::range<rast::shader_vertex_output_parameter>
        &p_vertex_output,
    rast::shader_vertex_function p_vertex,
    rast::shader_fragment_function p_fragment) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  return l_ren->create_shader(p_vertex_output, p_vertex, p_fragment);
};

inline void ren_handle::destroy(shader_handle p_shader) {
  details::ren_implementations *l_ren = (details::ren_implementations *)(m_ptr);
  l_ren->destroy_shader(p_shader);
};

inline ren_handle ren_handle_allocate() {
  ren_handle l_handle;
  l_handle.m_ptr = sys::malloc(sizeof(details::ren_implementations));
  return l_handle;
};

inline void ren_handle_free(ren_handle p_handle) { sys::free(p_handle.m_ptr); };

}; // namespace ren