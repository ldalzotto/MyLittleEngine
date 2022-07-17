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

struct camera {
  ui32 m_rendertexture_width;
  ui32 m_rendertexture_height;

  ui32 m_width;
  ui32 m_height;

  m::mat<fix32, 4, 4> m_view;
  m::mat<fix32, 4, 4> m_projection;

  camera() = default;
};

struct program_rasterizer_handles {
  bgfx::ProgramHandle m_program;
  bgfx::ShaderHandle m_vertex;
  bgfx::ShaderHandle m_fragment;
};

inline uint64_t program_meta_get_state(const program_meta &p_program_meta) {
  uint64_t l_state = 0;
  if (p_program_meta.m_cull_mode == program_meta::cull_mode::clockwise) {
    l_state = l_state | BGFX_STATE_CULL_CW;
  } else if (p_program_meta.m_cull_mode ==
             program_meta::cull_mode::cclockwise) {
    l_state = l_state | BGFX_STATE_CULL_CCW;
  }

  if (p_program_meta.m_write_depth) {
    l_state = l_state | BGFX_STATE_WRITE_Z;
  }

  if (p_program_meta.m_depth_test == program_meta::depth_test::less) {
    l_state = l_state | BGFX_STATE_DEPTH_TEST_LESS;
  }

  return l_state;
};

struct ren_impl {

  struct render_pass {
    camera_handle m_camera;
    program_handle m_program;
    material_handle m_material;
    m::mat<fix32, 4, 4> m_transform;
    mesh_handle m_mesh;

    static render_pass make(camera_handle p_camera, program_handle p_program,
                            material_handle p_material,
                            const m::mat<fix32, 4, 4> &p_transform,
                            mesh_handle p_mesh) {
      render_pass l_render_pass;
      l_render_pass.m_camera = p_camera;
      l_render_pass.m_program = p_program;
      l_render_pass.m_material = p_material;
      l_render_pass.m_transform = p_transform;
      l_render_pass.m_mesh = p_mesh;
      return l_render_pass;
    };
  };

  struct material {

  private:
    orm::table_heap_stacked<ui8, orm::heap_index_tag<bgfx::UniformHandle>>
        m_heap;

  public:
    void allocate() { m_heap.allocate(0); };

    void free() { m_heap.free(); };

    void push_back(bgfx::UniformHandle p_handle, const uimax p_size) {
      m_heap.push_back(p_size, 1);
      bgfx::UniformHandle *l_handle;
      m_heap.at(m_heap.count() - 1, none(), &l_handle);
      *l_handle = p_handle;
    };

    container::range<ui8> at(uimax p_index) {
      container::range<ui8> l_range;
      m_heap.at(p_index, &l_range);
      return l_range;
    };

    template <typename Callback> void for_each_handle(const Callback &p_cb) {
      for (auto i = 0; i < m_heap.count(); ++i) {
        bgfx::UniformHandle *l_handle;
        m_heap.at(i, none(), &l_handle);
        p_cb(*l_handle);
      }
    };

    template <typename Callback>
    void for_each_handle_and_range(const Callback &p_cb) {
      for (auto i = 0; i < m_heap.count(); ++i) {
        bgfx::UniformHandle *l_handle;
        container::range<ui8> l_range;
        m_heap.at(i, &l_range, &l_handle);
        p_cb(*l_handle, l_range);
      }
    };
  };

  struct heap {

    orm::table_pool_v2<camera, bgfx::FrameBufferHandle> m_camera_table;
    orm::table_pool_v2<program_meta, program_rasterizer_handles>
        m_program_table;
    orm::table_pool_v2<bgfx::VertexBufferHandle, bgfx::IndexBufferHandle>
        m_mesh_table;
    orm::table_pool_v2<material> m_materials;
    container::vector<render_pass> m_render_passes;

    void allocate() {
      m_camera_table.allocate(0);
      m_program_table.allocate(0);
      m_mesh_table.allocate(0);
      m_materials.allocate(0);
      m_render_passes.allocate(0);
    };

    void free() {
      m_camera_table.free();
      m_program_table.free();
      m_mesh_table.free();
      m_materials.free();
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

  void camera_set_orthographic(camera_handle p_camera, fix32 p_width,
                               fix32 p_height, fix32 p_near, fix32 p_far) {
    camera *l_camera;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, none());
    l_camera->m_projection = m::orthographic<fix32>(
        -p_width, p_width, -p_height, p_height, p_near, p_far);
  };

  void camera_set_projection(camera_handle p_camera,
                             const m::mat<fix32, 4, 4> &p_projection) {
    camera *l_camera;
    m_heap.m_camera_table.at(p_camera.m_idx, &l_camera, none());
    l_camera->m_projection = p_projection;
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
  mesh_handle mesh_create(const assets::mesh &p_mesh,
                          rast_api<Rasterizer> p_rast) {
    bgfx::VertexBufferHandle l_vertex_buffer;
    bgfx::IndexBufferHandle l_index_buffer;
    algorithm::upload_mesh_to_gpu(p_rast, p_mesh, &l_vertex_buffer,
                                  &l_index_buffer);
    uimax l_index =
        m_heap.m_mesh_table.push_back(l_vertex_buffer, l_index_buffer);
    return mesh_handle{.m_idx = l_index};
  };

  template <typename Rasterizer>
  void mesh_destroy(mesh_handle p_mesh, rast_api<Rasterizer> p_rast) {
    bgfx::VertexBufferHandle *l_vertex_buffer;
    bgfx::IndexBufferHandle *l_index_buffer;
    m_heap.m_mesh_table.at(p_mesh.m_idx, &l_vertex_buffer, &l_index_buffer);
    p_rast.destroy(*l_vertex_buffer);
    p_rast.destroy(*l_index_buffer);
    m_heap.m_mesh_table.remove_at(p_mesh.m_idx);
  };

  // TODO -> what we want here is to have a fixed capacity that is getting
  // calculated in advance.
  // Because we are not supposed to add parameters on the fly on a material.
  material_handle material_create() {
    material l_material;
    l_material.allocate();
    uimax l_index = m_heap.m_materials.push_back(l_material);
    return {l_index};
  };

  template <typename Rasterizer>
  void material_destroy(material_handle p_material,
                        rast_api<Rasterizer> p_rast) {
    material *l_material;
    m_heap.m_materials.at(p_material.m_idx, &l_material);
    l_material->for_each_handle(
        [&](const bgfx::UniformHandle &p_handle) { p_rast.destroy(p_handle); });

    l_material->free();
    m_heap.m_materials.remove_at(p_material.m_idx);
  };

  template <typename Rasterizer>
  void material_push(material_handle p_material, const char *p_name,
                     bgfx::UniformType::Enum p_type,
                     rast_api<Rasterizer> p_rast) {
    auto l_uniform = p_rast.createUniform(p_name, p_type);
    material *l_material;
    m_heap.m_materials.at(p_material.m_idx, &l_material);
    l_material->push_back(l_uniform, rast::uniform_type_get_size(p_type));
  };

  template <typename Rasterizer>
  void material_set_vec4(material_handle p_material, uimax p_index,
                         const rast::uniform_vec4_t &p_value,
                         rast_api<Rasterizer> p_rast) {
    material *l_material;
    m_heap.m_materials.at(p_material.m_idx, &l_material);

    // TODO -> enable this
    /*
        block_debug([&]() {
          auto l_uniform_info = p_rast.getUniformInfo();
          assert_debug(l_uniform_info.type == bgfx::UniformType::Vec4);
        });
    */
    l_material->at(p_index).copy_from(
        container::range<traits::remove_ref<decltype(p_value)>::type>::make(
            &p_value, 1));
  };

  template <typename Rasterizer>
  program_handle program_create(
      const ren::program_meta &p_program_meta,
      const container::range<rast::shader_uniform> &p_vertex_uniforms,
      const container::range<rast::shader_vertex_output_parameter>
          &p_vertex_output,
      rast::shader_vertex_function p_vertex,
      const container::range<rast::shader_uniform> &p_fragment_uniforms,
      rast::shader_fragment_function p_fragment, rast_api<Rasterizer> p_rast) {
    auto l_vertex_shader_table = rast::shader_vertex_bytes::build_byte_header(
        p_vertex_uniforms.count(), p_vertex_output.count());
    const bgfx::Memory *l_vertex_shader_memory =
        p_rast.alloc(l_vertex_shader_table.size_of(), 8);
    rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
        l_vertex_shader_table, p_vertex_uniforms, p_vertex_output, p_vertex);

    auto l_fragment_shader_header =
        rast::shader_fragment_bytes::build_byte_header(
            p_fragment_uniforms.count());
    const bgfx::Memory *l_fragment_shader_memory =
        p_rast.alloc(l_fragment_shader_header.size_of(), 8);

    rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
        l_fragment_shader_header, p_fragment_uniforms, p_fragment);

    program_rasterizer_handles l_program_rast_handles;
    l_program_rast_handles.m_vertex =
        p_rast.createShader(l_vertex_shader_memory);
    l_program_rast_handles.m_fragment =
        p_rast.createShader(l_fragment_shader_memory);

    l_program_rast_handles.m_program = p_rast.createProgram(
        l_program_rast_handles.m_vertex, l_program_rast_handles.m_fragment);
    uimax l_index = m_heap.m_program_table.push_back(p_program_meta,
                                                     l_program_rast_handles);

    return program_handle{.m_idx = l_index};
  };

  void draw(camera_handle p_camera, program_handle p_program,
            material_handle p_material, const m::mat<fix32, 4, 4> &p_transform,
            mesh_handle p_mesh) {
    render_pass l_render_pass = l_render_pass.make(
        p_camera, p_program, p_material, p_transform, p_mesh);
    m_heap.m_render_passes.push_back(l_render_pass);
  };

  template <typename Rasterizer>
  void program_destroy(program_handle p_program, rast_api<Rasterizer> p_rast) {
    program_rasterizer_handles *l_program_rast_handles;
    m_heap.m_program_table.at(p_program.m_idx, none(), &l_program_rast_handles);

    p_rast.destroy(l_program_rast_handles->m_program);
    p_rast.destroy(l_program_rast_handles->m_vertex);
    p_rast.destroy(l_program_rast_handles->m_fragment);
    m_heap.m_program_table.remove_at(p_program.m_idx);
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

      material *l_material;
      m_heap.m_materials.at(p_render_pass.m_material.m_idx, &l_material);
      l_material->for_each_handle_and_range(
          [&](auto &p_handle, container::range<ui8> p_range) {
            p_rast.setUniform(p_handle, p_range.data());
          });

      program_meta *l_program_meta;
      program_rasterizer_handles *l_program_rast_handles;
      m_heap.m_program_table.at(p_render_pass.m_program.m_idx, &l_program_meta,
                                &l_program_rast_handles);
      auto l_state = program_meta_get_state(*l_program_meta);
      mesh_handle l_mesh = p_render_pass.m_mesh;
      bgfx::VertexBufferHandle *l_vertex_buffer;
      bgfx::IndexBufferHandle *l_index_buffer;
      m_heap.m_mesh_table.at(l_mesh.m_idx, &l_vertex_buffer, &l_index_buffer);

      m::mat<fix32, 4, 4> l_transform = p_render_pass.m_transform;

      p_rast.setTransform(l_transform.m_data);

      p_rast.setIndexBuffer(*l_index_buffer);
      p_rast.setVertexBuffer(0, *l_vertex_buffer);
      p_rast.setState(l_state);

      p_rast.submit(0, l_program_rast_handles->m_program);
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
    }
    m_heap.m_render_passes.clear();
  };
};

}; // namespace details

}; // namespace ren
