#pragma once

#include <m/color.hpp>
#include <m/geom.hpp>
#include <rast/impl/algorithm.hpp>
#include <rast/model.hpp>

struct rast_impl_software {
  struct memory_reference {
    uimax m_buffer_index;
    ui8 is_ref() { return m_buffer_index == -1; };
    memory_reference() = default;
    memory_reference(uimax p_buffer_index) : m_buffer_index(p_buffer_index){};
  };

  struct texture {
    bgfx::TextureInfo m_info;
    bgfx::Memory *m_buffer;

    container::range<ui8> range() {
      return container::range<ui8>::make(m_buffer->data, m_buffer->size);
    };
  };

  struct framebuffer {
    bgfx::TextureHandle m_rgb;
    bgfx::TextureHandle m_depth;

    ui8 has_depth() const { return m_depth.idx != bgfx::kInvalidHandle; };
  };

  struct vertexbuffer {
    bgfx::VertexLayout layout;
    const bgfx::Memory *memory;

    container::range<ui8> range() {
      return container::range<ui8>::make(memory->data, memory->size);
    };
  };

  struct indexbuffer {
    const bgfx::Memory *memory;

    container::range<ui8> range() {
      return container::range<ui8>::make(memory->data, memory->size);
    };
  };

  struct shader {
    const bgfx::Memory *m_buffer;
  };

  struct program {
    bgfx::ShaderHandle m_vertex;
    bgfx::ShaderHandle m_fragment;
  };

  struct command_temporary_stack {
    m::mat<fix32, 4, 4> m_transform;
    bgfx::VertexBufferHandle m_vertex_buffer;
    bgfx::IndexBufferHandle m_index_buffer;
    ui64 state;
    ui32 rgba;

    void clear() {
      m_transform = m_transform.getIdentity();
      m_vertex_buffer.idx = bgfx::kInvalidHandle;
      m_index_buffer.idx = bgfx::kInvalidHandle;
      state = -1;
      rgba = -1;
    };
  } m_command_temporary_stack;

  struct command_uniforms {
    uimax m_values_heap_handle;
    uimax m_count;
  };

  struct command_draw_call {
    bgfx::ProgramHandle m_program;
    m::mat<fix32, 4, 4> m_transform;
    bgfx::IndexBufferHandle m_index_buffer;
    bgfx::VertexBufferHandle m_vertex_buffer;
    command_uniforms m_vertex_uniforms;
    ui64 m_state;
    ui32 m_rgba;

    void make_from_temporary_stack(
        const struct command_temporary_stack &p_temporary_stack) {
      m_transform = p_temporary_stack.m_transform;
      m_index_buffer = p_temporary_stack.m_index_buffer;
      m_vertex_buffer = p_temporary_stack.m_vertex_buffer;
      m_state = p_temporary_stack.state;
      m_rgba = p_temporary_stack.rgba;
    };
  };

  struct clear_state {
    struct flags {
      union {
        struct {
          ui8 m_color : 1;
          ui8 m_depth : 1;
          ui8 m_stencil : 1;
        };
        ui16 m_int;
      };
    };

    flags m_flags;
    m::color_packed m_rgba;
    fix32 m_depth;

    void reset() {
      m_flags.m_int = 0;
      m_rgba.rgba = 0;
      m_depth = 0;
    };
  };

  struct render_pass {
    bgfx::FrameBufferHandle m_framebuffer;
    clear_state m_clear;
    m::rect_point_extend<ui16> m_rect;
    m::vec<ui16, 4> m_scissor;
    m::mat<fix32, 4, 4> m_view;
    m::mat<fix32, 4, 4> m_proj;

    container::vector<command_draw_call> m_commands;

    void allocate() { m_commands.allocate(0); };
    void free() { m_commands.free(); };

    static render_pass get_default() {
      render_pass l_render_pass;
      l_render_pass.allocate();
      l_render_pass.m_framebuffer.idx = 0;
      l_render_pass.m_rect = l_render_pass.m_rect.getZero();
      l_render_pass.m_scissor = l_render_pass.m_scissor.getZero();
      l_render_pass.m_view = l_render_pass.m_view.getZero();
      l_render_pass.m_proj = l_render_pass.m_proj.getZero();
      l_render_pass.m_clear.reset();
      return l_render_pass;
    };
  };

  struct uniform {
    bgfx::UniformType::Enum m_type;
    uimax m_hash;
    uimax m_index;
    uimax m_usage_count;
  };

  struct heap {

    orm::table_heap_paged_v2<ui8> m_buffer_memory_table;

    orm::table_heap_paged_v2<bgfx::Memory, memory_reference> m_buffers_table;

    using buffers_ptr_mapping_buffer_t = bgfx::Memory *;
    using buffers_ptr_mapping_buffer_index_t = uimax;
    using buffers_ptr_mapping_t =
        orm::table_vector_v2<buffers_ptr_mapping_buffer_t,
                             buffers_ptr_mapping_buffer_index_t>;

    buffers_ptr_mapping_t m_buffers_ptr_mapping_table;

    orm::table_pool_v2<texture> m_texture_table;
    orm::table_pool_v2<framebuffer> m_framebuffer_table;
    orm::table_pool_v2<vertexbuffer> m_vertexbuffer_table;
    orm::table_pool_v2<indexbuffer> m_indexbuffer_table;
    orm::table_vector_v2<render_pass> m_renderpass_table;
    orm::table_pool_v2<shader> m_shader_table;

    struct {
      container::pool<rast::uniform_vec4_t> vecs;
    } m_uniform_values;

    struct {
      container::pool<uniform> by_index;
      container::hashmap<uimax, uimax> by_key;
    } m_uniforms;

    container::heap_stacked m_uniform_draw_call_values_buffer;

    orm::table_pool_v2<program> m_program_table;

    void allocate() {
      m_renderpass_table.allocate(0);
      m_buffer_memory_table.allocate(4096 * 4096);
      m_buffers_table.allocate(1024);
      m_buffers_ptr_mapping_table.allocate(0);
      m_texture_table.allocate(0);
      m_framebuffer_table.allocate(0);
      m_vertexbuffer_table.allocate(0);
      m_indexbuffer_table.allocate(0);
      m_shader_table.allocate(0);
      m_program_table.allocate(0);

      m_uniform_values.vecs.allocate(0);
      m_uniforms.by_index.allocate(0);
      m_uniforms.by_key.allocate();
      m_uniform_draw_call_values_buffer.allocate(0);

      m_renderpass_table.push_back(
          render_pass::get_default()); // at least one renderpass
    };

    void free() {
      assert_debug(!m_uniforms.by_key.has_allocated_elements());
      assert_debug(!m_uniforms.by_index.has_allocated_elements());
      assert_debug(!m_uniform_draw_call_values_buffer.has_allocated_elements());
      assert_debug(!m_vertexbuffer_table.has_allocated_elements());
      assert_debug(!m_indexbuffer_table.has_allocated_elements());
      assert_debug(m_renderpass_table.count() == 1);
      assert_debug(!m_shader_table.has_allocated_elements());
      assert_debug(!m_program_table.has_allocated_elements());
      assert_debug(!m_framebuffer_table.has_allocated_elements());

      m_uniforms.by_key.free();
      m_uniforms.by_index.free();
      m_uniform_values.vecs.free();
      m_uniform_draw_call_values_buffer.free();

      for (auto l_render_pass_it = 0;
           l_render_pass_it < m_renderpass_table.count(); ++l_render_pass_it) {
        render_pass *l_render_pass;
        m_renderpass_table.at(l_render_pass_it, &l_render_pass);
        l_render_pass->free();
      }
      m_renderpass_table.free();
      m_buffer_memory_table.free();
      m_buffers_table.free();
      m_buffers_ptr_mapping_table.free();
      m_texture_table.free();
      m_vertexbuffer_table.free();
      m_indexbuffer_table.free();
      m_shader_table.free();
      m_program_table.free();
      m_framebuffer_table.free();
    };

    bgfx::Memory *allocate_buffer(uimax p_size) {
      auto l_buffer_index = m_buffer_memory_table.push_back(p_size);

      bgfx::Memory l_buffer{};
      ui8 *l_data;
      l_buffer.size = m_buffer_memory_table.at(l_buffer_index, &l_data);
      l_buffer.data = l_data;

      uimax l_index = m_buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      memory_reference *l_memory_refence;
      uimax l_memory_count =
          m_buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = memory_reference(l_buffer_index);

      m_buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    bgfx::Memory *allocate_buffer(uimax p_size, uimax p_alignment) {
      auto l_buffer_index =
          m_buffer_memory_table.push_back(p_size, p_alignment);

      bgfx::Memory l_buffer{};
      ui8 *l_data;
      l_buffer.size = m_buffer_memory_table.at(l_buffer_index, &l_data);
      l_buffer.data = l_data;

      uimax l_index = m_buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      memory_reference *l_memory_refence;
      uimax l_memory_count =
          m_buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = memory_reference(l_buffer_index);

      m_buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    bgfx::Memory *allocate_ref(const void *p_ptr, ui32 p_size) {
      bgfx::Memory l_buffer{};
      l_buffer.data = (ui8 *)p_ptr;
      l_buffer.size = p_size;

      uimax l_index = m_buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      memory_reference *l_memory_refence;
      uimax l_memory_count =
          m_buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = memory_reference(-1);

      m_buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    void free_buffer(const bgfx::Memory *p_buffer) {

      for (auto i = 0; i < m_buffers_ptr_mapping_table.m_meta.m_count; ++i) {
        buffers_ptr_mapping_buffer_t *l_buffer;
        m_buffers_ptr_mapping_table.at(i, &l_buffer, none());
        if (*l_buffer == p_buffer) {
          buffers_ptr_mapping_buffer_index_t *l_buffer_index;
          m_buffers_ptr_mapping_table.at(i, none(), &l_buffer_index);
          memory_reference *l_reference;
          uimax l_buffers_table_count =
              m_buffers_table.at(*l_buffer_index, none(), &l_reference);
          assert_debug(l_buffers_table_count == 1);
          if (!l_reference->is_ref()) {
            m_buffer_memory_table.remove_at(l_reference->m_buffer_index);
          }
          m_buffers_ptr_mapping_table.remove_at(i);
          return;
        }
      }
    };

    bgfx::TextureHandle
    allocate_texture(const bgfx::TextureInfo &p_texture_info) {
      uimax l_image_size = uimax(p_texture_info.bitsPerPixel *
                                 p_texture_info.width * p_texture_info.height);
      texture l_texture;
      l_texture.m_info = p_texture_info;
      l_texture.m_buffer = allocate_buffer(l_image_size);
      bgfx::TextureHandle l_texture_handle;
      l_texture_handle.idx = m_texture_table.push_back(l_texture);

      return l_texture_handle;
    };

    void free_texture(bgfx::TextureHandle p_texture) {
      texture *l_texture;
      m_texture_table.at(p_texture.idx, &l_texture);
      free_buffer(l_texture->m_buffer);
      m_texture_table.remove_at(p_texture.idx);
    };

    bgfx::FrameBufferHandle
    allocate_frame_buffer(bgfx::TextureHandle p_rgb_texture,
                          bgfx::TextureHandle p_depth_texture) {
      framebuffer l_frame_buffer;
      l_frame_buffer.m_rgb = p_rgb_texture;
      l_frame_buffer.m_depth = p_depth_texture;

      bgfx::FrameBufferHandle l_handle;
      l_handle.idx = m_framebuffer_table.push_back(l_frame_buffer);
      return l_handle;
    };

    framebuffer *get_frame_buffer(bgfx::FrameBufferHandle p_frame_buffer) {
      framebuffer *l_frame_buffer;
      m_framebuffer_table.at(p_frame_buffer.idx, &l_frame_buffer);
      return l_frame_buffer;
    };

    void free_frame_buffer(bgfx::FrameBufferHandle p_frame_buffer) {
      framebuffer *l_frame_buffer = get_frame_buffer(p_frame_buffer);
      free_texture(l_frame_buffer->m_rgb);
      if (l_frame_buffer->has_depth()) {
        free_texture(l_frame_buffer->m_depth);
      }
      m_framebuffer_table.remove_at(p_frame_buffer.idx);
    };

    bgfx::VertexBufferHandle
    allocate_vertex_buffer(const bgfx::Memory *p_memory,
                           const bgfx::VertexLayout &p_layout) {

      assert_debug((p_memory->size % p_layout.getStride()) == 0);
      assert_debug((p_memory->size % 2) == 0); // memory is aligned

      vertexbuffer l_vertex_buffer;
      l_vertex_buffer.layout = p_layout;
      l_vertex_buffer.memory = p_memory;
      bgfx::VertexBufferHandle l_handle;
      l_handle.idx = m_vertexbuffer_table.push_back(l_vertex_buffer);
      return l_handle;
    };

    void free_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
      vertexbuffer *l_vertex_buffer;
      m_vertexbuffer_table.at(p_handle.idx, &l_vertex_buffer);
      free_buffer(l_vertex_buffer->memory);
      m_vertexbuffer_table.remove_at(p_handle.idx);
    };

    bgfx::IndexBufferHandle
    allocate_index_buffer(const bgfx::Memory *p_memory) {
      indexbuffer l_index_buffer;
      l_index_buffer.memory = p_memory;
      bgfx::IndexBufferHandle l_handle;
      l_handle.idx = m_indexbuffer_table.push_back(l_index_buffer);
      return l_handle;
    };

    void free_index_buffer(bgfx::IndexBufferHandle p_handle) {
      indexbuffer *l_index_buffer;
      m_indexbuffer_table.at(p_handle.idx, &l_index_buffer);
      free_buffer(l_index_buffer->memory);
      m_indexbuffer_table.remove_at(p_handle.idx);
    };

    bgfx::ShaderHandle allocate_shader(const bgfx::Memory *p_memory) {
      bgfx::ShaderHandle l_handle;
      shader l_shader;
      l_shader.m_buffer = p_memory;
      l_handle.idx = m_shader_table.push_back(l_shader, 0);
      return l_handle;
    };

    void free_shader(bgfx::ShaderHandle p_handle) {
      m_shader_table.remove_at(p_handle.idx);
    };

    bgfx::ProgramHandle allocate_program(const program &p_program) {
      bgfx::ProgramHandle l_handle;
      l_handle.idx = m_program_table.push_back(p_program);
      return l_handle;
    };

    void free_program(bgfx::ProgramHandle p_handle) {
      m_program_table.remove_at(p_handle.idx);
    };

    bgfx::UniformHandle allocate_uniform(const ui8 *p_name,
                                         bgfx::UniformType::Enum p_type) {
      auto l_uniform_hash = algorithm::hash(
          container::range<ui8>::make((ui8 *)p_name, sys::strlen(p_name)));
      uimax l_uniform_index;
      if (!m_uniforms.by_key.has_key(l_uniform_hash)) {
        uniform l_uniform;
        l_uniform.m_usage_count = 0;
        l_uniform.m_hash = l_uniform_hash;
        l_uniform.m_type = p_type;
        if (p_type == bgfx::UniformType::Enum::Vec4) {
          l_uniform.m_index = m_uniform_values.vecs.push_back({});
        } else {
          sys::abort();
        }
        l_uniform_index = m_uniforms.by_index.push_back(l_uniform);
        m_uniforms.by_key.push_back(l_uniform_hash, l_uniform_index);
      } else {
        l_uniform_index = m_uniforms.by_key.at(l_uniform_hash);
      }

      m_uniforms.by_index.at(m_uniforms.by_key.at(l_uniform_hash))
          .m_usage_count += 1;

      bgfx::UniformHandle l_handle;
      l_handle.idx = l_uniform_index;
      return l_handle;
    };

    void free_uniform(bgfx::UniformHandle p_uniform) {
      uniform &l_uniform = m_uniforms.by_index.at(p_uniform.idx);
      l_uniform.m_usage_count -= 1;
      if (l_uniform.m_usage_count == 0) {
        m_uniforms.by_key.remove_at(l_uniform.m_hash);
        m_uniforms.by_index.remove_at(p_uniform.idx);
      }
    };

  } heap;

  rast::algorithm::rasterize_heap m_rasterize_heap;

  struct texture_proxy {
    struct heap &m_heap;
    texture *m_value;

    texture *value() { return m_value; };
  };

  struct framebuffer_proxy {
    struct heap &m_heap;
    framebuffer *m_value;

    texture_proxy RGBTexture() {
      texture *l_texture;
      m_heap.m_texture_table.at(m_value->m_rgb.idx, &l_texture);
      return {.m_heap = m_heap, .m_value = l_texture};
    };

    texture_proxy DepthTexture() {
      texture *l_texture;
      m_heap.m_texture_table.at(m_value->m_depth.idx, &l_texture);
      return {.m_heap = m_heap, .m_value = l_texture};
    };
  };

  struct renderpass_proxy {
    struct heap &m_heap;
    render_pass *m_value;

    renderpass_proxy(struct heap &p_heap, render_pass *p_value)
        : m_heap(p_heap), m_value(p_value){

                          };
    render_pass *value() { return m_value; };

    framebuffer_proxy FrameBuffer() {
      struct framebuffer *l_frame_buffer;
      m_heap.m_framebuffer_table.at(m_value->m_framebuffer.idx,
                                    &l_frame_buffer);
      return {.m_heap = m_heap, .m_value = l_frame_buffer};
    };

    template <typename Callback> void for_each_commands(const Callback &p_cb) {
      for (auto l_it = 0; l_it < m_value->m_commands.count(); ++l_it) {
        p_cb(m_value->m_commands.at(l_it));
      }
    };
  };

  struct shader_proxy {
    struct heap &m_heap;
    shader *m_shader;
  };

  struct program_proxy {
    struct heap &m_heap;
    program *m_program;

    program_proxy(struct heap &p_heap, program *p_program)
        : m_heap(p_heap), m_program(p_program){

                          };

    shader_proxy VertexShader() {
      shader *l_shader;
      m_heap.m_shader_table.at(m_program->m_vertex.idx, &l_shader);
      return {.m_heap = m_heap, .m_shader = l_shader};
    };

    shader_proxy FragmentShader() {
      shader *l_shader;
      m_heap.m_shader_table.at(m_program->m_fragment.idx, &l_shader);
      return {.m_heap = m_heap, .m_shader = l_shader};
    };
  };

  struct command_draw_call_proxy {
    struct heap &m_heap;
    command_draw_call *m_value;

    command_draw_call_proxy(struct heap &p_heap, command_draw_call *p_value)
        : m_heap(p_heap), m_value(p_value){

                          };

    command_draw_call *value() { return m_value; };

    indexbuffer *IndexBuffer() {
      struct indexbuffer *l_index_buffer;
      m_heap.m_indexbuffer_table.at(m_value->m_index_buffer.idx,
                                    &l_index_buffer);
      return l_index_buffer;
    };

    vertexbuffer *VertexBuffer() {
      struct vertexbuffer *l_vertex_buffer;
      m_heap.m_vertexbuffer_table.at(m_value->m_vertex_buffer.idx,
                                     &l_vertex_buffer);
      return l_vertex_buffer;
    };

    program_proxy Program() {
      struct program *l_program;
      m_heap.m_program_table.at(m_value->m_program.idx, &l_program);
      return program_proxy(m_heap, l_program);
    };
  };

  struct heap_proxy {
    struct heap &m_heap;

    framebuffer_proxy FrameBuffer(bgfx::FrameBufferHandle p_handle) {
      struct framebuffer *l_frame_buffer = m_heap.get_frame_buffer(p_handle);
      return {.m_heap = m_heap, .m_value = l_frame_buffer};
    };

    renderpass_proxy RenderPass(bgfx::ViewId p_handle) {
      struct render_pass *l_render_pass;
      m_heap.m_renderpass_table.at(p_handle, &l_render_pass);
      return renderpass_proxy(m_heap, l_render_pass);
    };

    texture_proxy Texture(bgfx::TextureHandle p_handle) {
      struct texture *l_texture;
      m_heap.m_texture_table.at(p_handle.idx, &l_texture);
      return texture_proxy{.m_heap = m_heap, .m_value = l_texture};
    };

    template <typename Callback>
    void for_each_renderpass(const Callback &p_cb) {
      for (auto i = 0; i < m_heap.m_renderpass_table.m_meta.m_count; ++i) {
        struct render_pass *l_render_pass;
        m_heap.m_renderpass_table.at(i, &l_render_pass);
        renderpass_proxy l_proxy(m_heap, l_render_pass);
        p_cb(l_proxy);
      }
    };
  };

  heap_proxy proxy() { return {.m_heap = heap}; };

  void set_uniform(bgfx::UniformHandle p_handle, const void *p_value) {
    auto l_uniform = __get_uniform(p_handle);
    container::range<ui8> l_value;
    l_value.m_begin = (ui8 *)p_value;
    l_value.count() = l_uniform.count();
    l_value.copy_to(l_uniform);
  };

  bgfx::TextureHandle allocate_texture(uint16_t p_width, uint16_t p_height,
                                       bool p_hasMips, uint16_t p_numLayers,
                                       bgfx::TextureFormat::Enum p_format,
                                       uint64_t p_flags) {
    bgfx::TextureInfo l_texture_info{};
    l_texture_info.format = p_format;
    l_texture_info.width = p_width;
    l_texture_info.height = p_height;
    l_texture_info.bitsPerPixel = textureformat_to_pixel_size(p_format);
    assert_debug(l_texture_info.bitsPerPixel != 0);
    return heap.allocate_texture(l_texture_info);
  };

  void read_texture(bgfx::TextureHandle p_texture, ui8 *out) {
    container::range<ui8> l_texture_range =
        proxy().Texture(p_texture).value()->range();
    container::range<ui8> l_target_range =
        container::range<ui8>::make(out, l_texture_range.count());
    l_texture_range.copy_to(l_target_range);
  };

  bgfx::FrameBufferHandle
  allocate_frame_buffer(uint16_t p_width, uint16_t p_height,
                        bgfx::TextureFormat::Enum p_rgb_format,
                        uint64_t p_textureFlags) {
    auto l_texture =
        allocate_texture(p_width, p_height, 0, 0, p_rgb_format, p_textureFlags);
    return heap.allocate_frame_buffer(
        l_texture, bgfx::TextureHandle{bgfx::kInvalidHandle});
  };

  bgfx::FrameBufferHandle
  allocate_frame_buffer(uint16_t p_width, uint16_t p_height,
                        bgfx::TextureFormat::Enum p_rgb_format,
                        bgfx::TextureFormat::Enum p_depth_format) {
    auto l_rgb_texture =
        allocate_texture(p_width, p_height, 0, 0, p_rgb_format, 0);
    auto l_depth_format =
        allocate_texture(p_width, p_height, 0, 0, p_depth_format, 0);
    return heap.allocate_frame_buffer(l_rgb_texture, l_depth_format);
  };

  bgfx::TextureHandle get_texture(bgfx::FrameBufferHandle p_frame_buffer) {
    return proxy().FrameBuffer(p_frame_buffer).m_value->m_rgb;
  };

  bgfx::ProgramHandle allocate_program(bgfx::ShaderHandle p_vertex,
                                       bgfx::ShaderHandle p_fragment) {
    program l_program;
    l_program.m_vertex = p_vertex;
    l_program.m_fragment = p_fragment;
    return heap.allocate_program(l_program);
  };

  void view_set_rect(bgfx::ViewId p_id, uint16_t p_x, uint16_t p_y,
                     uint16_t p_width, uint16_t p_height) {
    m::rect_point_extend<ui16> &l_view =
        proxy().RenderPass(p_id).value()->m_rect;
    l_view.point() = {p_x, p_y};
    l_view.extend() = {p_width, p_height};
  };

  void view_set_framebuffer(bgfx::ViewId p_id,
                            bgfx::FrameBufferHandle p_handle) {
    proxy().RenderPass(p_id).value()->m_framebuffer = p_handle;
  };

  void view_set_clear(bgfx::ViewId p_id, ui16 p_flags, ui32 p_rgba,
                      fix32 p_depth) {
    clear_state &l_clear = proxy().RenderPass(p_id).value()->m_clear;
    l_clear.m_rgba.rgba = p_rgba;
    l_clear.m_flags.m_int = p_flags;
    l_clear.m_depth = p_depth;
  };

  void view_set_transform(bgfx::ViewId p_id, const m::mat<fix32, 4, 4> &p_view,
                          const m::mat<fix32, 4, 4> &p_proj) {
    renderpass_proxy l_render_pass = proxy().RenderPass(p_id);
    l_render_pass.value()->m_view = p_view;
    l_render_pass.value()->m_proj = p_proj;
  };

  void view_submit(bgfx::ViewId p_id, bgfx::ProgramHandle p_program) {
    command_draw_call l_draw_call;
    l_draw_call.m_program = p_program;
    l_draw_call.make_from_temporary_stack(m_command_temporary_stack);
    m_command_temporary_stack.clear();

    program *__program;
    heap.m_program_table.at(l_draw_call.m_program.idx, &__program);
    program_proxy l_program = program_proxy(heap, __program);

    rast::algorithm::program l_rasterizer_program;
    l_rasterizer_program.m_vertex =
        l_program.VertexShader().m_shader->m_buffer->data;
    l_rasterizer_program.m_fragment =
        l_program.FragmentShader().m_shader->m_buffer->data;

    rast::shader_vertex_bytes::view l_shader_vertex_view = {
        (ui8 *)l_rasterizer_program.m_vertex};
    rast::algorithm::program_uniforms l_vertex_uniforms;
    auto l_vertex_shader_uniforms = l_shader_vertex_view.uniforms();

    heap.m_uniform_draw_call_values_buffer.push_back(
        sizeof(void *) * l_vertex_shader_uniforms.count(), 1);
    l_draw_call.m_vertex_uniforms.m_values_heap_handle =
        heap.m_uniform_draw_call_values_buffer.count() - 1;
    l_draw_call.m_vertex_uniforms.m_count = l_vertex_shader_uniforms.count();

    for (auto l_vertex_uniform_it = 0;
         l_vertex_uniform_it < l_vertex_shader_uniforms.count();
         ++l_vertex_uniform_it) {
      rast::shader_uniform &l_shader_uniform =
          l_vertex_shader_uniforms.at(l_vertex_uniform_it);
      if (l_shader_uniform.m_type == bgfx::UniformType::Vec4) {
        heap.m_uniform_draw_call_values_buffer.push_back(
            sizeof(rast::uniform_vec4_t), 1);
        heap.m_uniform_draw_call_values_buffer
            .at(heap.m_uniform_draw_call_values_buffer.count() - 1)
            .cast_to<rast::uniform_vec4_t>()
            .at(0) = *__get_uniform_vec4(l_shader_uniform.m_hash);
      }
    }

    l_draw_call.m_vertex_uniforms.m_count = l_vertex_shader_uniforms.count();

    proxy().RenderPass(p_id).value()->m_commands.push_back(l_draw_call);
  };

  void set_transform(const m::mat<fix32, 4, 4> &p_transform) {
    m_command_temporary_stack.m_transform = p_transform;
  };

  void set_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
    m_command_temporary_stack.m_vertex_buffer = p_handle;
  };

  void set_index_buffer(bgfx::IndexBufferHandle p_handle) {
    m_command_temporary_stack.m_index_buffer = p_handle;
  };

  void set_state(uint64_t p_state, uint32_t p_rgba) {
    m_command_temporary_stack.state = p_state;
    m_command_temporary_stack.rgba = p_rgba;
  };

  void frame() {

    proxy().for_each_renderpass([&](renderpass_proxy &p_render_pass) {
      framebuffer_proxy l_frame_buffer = p_render_pass.FrameBuffer();
      texture_proxy l_frame_rgb_texture = l_frame_buffer.RGBTexture();
      container::range<ui8> l_frame_rgb_texture_range =
          l_frame_rgb_texture.value()->range();

      container::range<ui8> l_frame_depth_texture_range;
      bgfx::TextureInfo l_frame_depth_texture_info;

      if (l_frame_buffer.m_value->has_depth()) {
        texture_proxy l_frame_depth_texture =
            p_render_pass.FrameBuffer().DepthTexture();
        l_frame_depth_texture_range = l_frame_depth_texture.value()->range();
        l_frame_depth_texture_info = l_frame_depth_texture.value()->m_info;
      } else {
        l_frame_depth_texture_range = container::range<ui8>::make(0, 0);
        l_frame_depth_texture_info.bitsPerPixel = 0;
      }

      // color clear
      {
        const clear_state &l_clear_state = p_render_pass.value()->m_clear;
        if (l_clear_state.m_flags.m_color) {
          texture *l_texture = l_frame_rgb_texture.value();
          rast::image_view l_target_view(
              l_texture->m_info.width, l_texture->m_info.height,
              l_texture->m_info.bitsPerPixel, l_frame_rgb_texture_range);
          l_target_view.for_each<rgb_t>([&](rgb_t &p_pixel) {
            p_pixel.x() = l_clear_state.m_rgba.r;
            p_pixel.y() = l_clear_state.m_rgba.g;
            p_pixel.z() = l_clear_state.m_rgba.b;
          });
        }

        if (l_clear_state.m_flags.m_depth) {
          assert_debug(l_frame_buffer.m_value->has_depth());
          rast::image_view l_depth_view(l_frame_depth_texture_info.width,
                                        l_frame_depth_texture_info.height,
                                        l_frame_depth_texture_info.bitsPerPixel,
                                        l_frame_depth_texture_range);
          l_depth_view.for_each<fix32>(
              [&](fix32 &p_pixel) { p_pixel = l_clear_state.m_depth; });
        }
      }

      p_render_pass.for_each_commands([&](command_draw_call &p_command) {
        command_draw_call_proxy l_draw_call(heap, &p_command);
        indexbuffer *l_index_buffer = l_draw_call.IndexBuffer();
        vertexbuffer *l_vertex_buffer = l_draw_call.VertexBuffer();
        program_proxy l_program = l_draw_call.Program();
        rast::algorithm::program l_rasterizer_program;
        l_rasterizer_program.m_vertex =
            l_program.VertexShader().m_shader->m_buffer->data;
        l_rasterizer_program.m_fragment =
            l_program.FragmentShader().m_shader->m_buffer->data;

        rast::algorithm::program_uniforms l_vertex_uniforms =
            heap.m_uniform_draw_call_values_buffer
                .at(l_draw_call.m_value->m_vertex_uniforms.m_values_heap_handle)
                .cast_to<void *>();

        for (auto l_vertex_uniform_idx = 0;
             l_vertex_uniform_idx < l_vertex_uniforms.count();
             ++l_vertex_uniform_idx) {
          l_vertex_uniforms.at(l_vertex_uniform_idx) =
              (void *)heap.m_uniform_draw_call_values_buffer
                  .at(l_draw_call.m_value->m_vertex_uniforms
                          .m_values_heap_handle +
                      1 + l_vertex_uniform_idx)
                  .data();
        }

        rast::algorithm::rasterize_unit(
            m_rasterize_heap, l_rasterizer_program,
            p_render_pass.value()->m_rect, p_render_pass.value()->m_proj,
            p_render_pass.value()->m_view, l_draw_call.value()->m_transform,
            l_index_buffer->range(), l_vertex_buffer->layout,
            l_vertex_buffer->range(), l_vertex_uniforms,
            l_draw_call.value()->m_state, l_draw_call.value()->m_rgba,
            l_frame_rgb_texture.value()->m_info, l_frame_rgb_texture_range,
            l_frame_depth_texture_info, l_frame_depth_texture_range)
            .rasterize();
      });
    });

    proxy().for_each_renderpass([&](renderpass_proxy &p_render_passs) {
      p_render_passs.value()->m_commands.clear();
    });

    heap.m_uniform_draw_call_values_buffer.clear();
  };

  void initialize() {
    heap.allocate();
    m_rasterize_heap.allocate();
    m_command_temporary_stack.clear();
  };

  void terminate() {

    heap.free();
    m_rasterize_heap.free();
    // TODO
  };

private:
  const rast::uniform_vec4_t *__get_uniform_vec4(uimax p_hash) {
    return __get_uniform(p_hash).cast_to<rast::uniform_vec4_t>().data();
  }

  container::range<ui8> __get_uniform(uimax p_hash) {
    auto &l_uniform =
        heap.m_uniforms.by_index.at(heap.m_uniforms.by_key.at(p_hash));
    if (l_uniform.m_type == bgfx::UniformType::Vec4) {
      auto &l_value = heap.m_uniform_values.vecs.at(l_uniform.m_index);
      return container::range<ui8>::make((ui8 *)&l_value, sizeof(l_value));
    }
    return container::range<ui8>::make(0, 0);
  };

  container::range<ui8> __get_uniform(bgfx::UniformHandle p_handle) {
    auto &l_uniform = heap.m_uniforms.by_index.at(p_handle.idx);
    if (l_uniform.m_type == bgfx::UniformType::Vec4) {
      auto &l_value = heap.m_uniform_values.vecs.at(l_uniform.m_index);
      return container::range<ui8>::make((ui8 *)&l_value, sizeof(l_value));
    }
    return container::range<ui8>::make(0, 0);
  };
};

FORCE_INLINE ui8 rast_api_init(rast_impl_software *thiz,
                               const bgfx::Init &p_init = {}) {
  thiz->initialize();
  return 1;
};

FORCE_INLINE void rast_api_shutdown(rast_impl_software *thiz) {
  thiz->terminate();
};

FORCE_INLINE const bgfx::Memory *rast_api_alloc(rast_impl_software *thiz,
                                                uint32_t _size) {
  return thiz->heap.allocate_buffer(_size);
};

FORCE_INLINE const bgfx::Memory *
rast_api_alloc(rast_impl_software *thiz, uint32_t _size, uint32_t _alignment) {
  return thiz->heap.allocate_buffer(_size, _alignment);
};

FORCE_INLINE const bgfx::Memory *
rast_api_makeRef(rast_impl_software *thiz, const void *_data, uint32_t _size,
                 bgfx::ReleaseFn _releaseFn = NULL, void *_userData = NULL) {
  return thiz->heap.allocate_ref(_data, _size);
};

FORCE_INLINE bgfx::TextureHandle rast_api_createTexture2D(
    rast_impl_software *thiz, uint16_t _width, uint16_t _height, bool _hasMips,
    uint16_t _numLayers, bgfx::TextureFormat::Enum _format,
    uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
    const bgfx::Memory *_mem = NULL) {
  return thiz->allocate_texture(_width, _height, _hasMips, _numLayers, _format,
                                _flags);
};

FORCE_INLINE bgfx::FrameBufferHandle rast_api_createFrameBuffer(
    rast_impl_software *thiz, uint16_t _width, uint16_t _height,
    bgfx::TextureFormat::Enum _format,
    uint64_t _textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP) {
  return thiz->allocate_frame_buffer(_width, _height, _format, _textureFlags);
};

FORCE_INLINE bgfx::FrameBufferHandle rast_api_createFrameBuffer(
    rast_impl_software *thiz, void *_nwh, uint16_t _width, uint16_t _height,
    bgfx::TextureFormat::Enum _format = bgfx::TextureFormat::Count,
    bgfx::TextureFormat::Enum _depthFormat = bgfx::TextureFormat::Count) {
  return thiz->allocate_frame_buffer(_width, _height, _format, _depthFormat);
};

FORCE_INLINE void rast_api_destroy(rast_impl_software *thiz,
                                   bgfx::FrameBufferHandle _handle) {
  thiz->heap.free_frame_buffer(_handle);
};

FORCE_INLINE bgfx::TextureHandle
rast_api_getTexture(rast_impl_software *thiz, bgfx::FrameBufferHandle _handle,
                    uint8_t _attachment = 0) {
  return thiz->get_texture(_handle);
};

FORCE_INLINE container::range<ui8>
rast_api_fetchTextureSync(rast_impl_software *thiz,
                          bgfx::TextureHandle _texture) {
  return thiz->proxy().Texture(_texture).value()->range();
};

FORCE_INLINE bgfx::VertexBufferHandle
rast_api_createVertexBuffer(rast_impl_software *thiz, const bgfx::Memory *_mem,
                            const bgfx::VertexLayout &_layout,
                            uint16_t _flags = BGFX_BUFFER_NONE) {
  return thiz->heap.allocate_vertex_buffer(_mem, _layout);
};

FORCE_INLINE void rast_api_destroy(rast_impl_software *thiz,
                                   bgfx::VertexBufferHandle _handle) {
  thiz->heap.free_vertex_buffer(_handle);
};

FORCE_INLINE bgfx::IndexBufferHandle
rast_api_createIndexBuffer(rast_impl_software *thiz, const bgfx::Memory *_mem,
                           uint16_t _flags) {
  return thiz->heap.allocate_index_buffer(_mem);
};

FORCE_INLINE void rast_api_destroy(rast_impl_software *thiz,
                                   bgfx::IndexBufferHandle _handle) {
  thiz->heap.free_index_buffer(_handle);
};

FORCE_INLINE bgfx::ShaderHandle
rast_api_createShader(rast_impl_software *thiz, const bgfx::Memory *_mem) {
  return thiz->heap.allocate_shader(_mem);
};

FORCE_INLINE void rast_api_destroy(rast_impl_software *thiz,
                                   bgfx::ShaderHandle _handle) {
  thiz->heap.free_shader(_handle);
};

FORCE_INLINE bgfx::ProgramHandle
rast_api_createProgram(rast_impl_software *thiz, bgfx::ShaderHandle _vsh,
                       bgfx::ShaderHandle _fsh, bool _destroyShaders = false) {
  return thiz->allocate_program(_vsh, _fsh);
};

FORCE_INLINE void rast_api_destroy(rast_impl_software *thiz,
                                   bgfx::ProgramHandle _handle) {
  thiz->heap.free_program(_handle);
};

FORCE_INLINE bgfx::UniformHandle
rast_api_createUniform(rast_impl_software *thiz, const char *_name,
                       bgfx::UniformType::Enum _type, uint16_t _num) {
  return thiz->heap.allocate_uniform((const ui8 *)_name, _type);
};

FORCE_INLINE void rast_api_destroy(rast_impl_software *thiz,
                                   bgfx::UniformHandle p_uniform) {
  thiz->heap.free_uniform(p_uniform);
};

FORCE_INLINE void rast_api_setUniform(rast_impl_software *thiz,
                                      bgfx::UniformHandle _handle,
                                      const void *_value, uint16_t _num) {
  thiz->set_uniform(_handle, _value);
};

FORCE_INLINE void rast_api_setViewRect(rast_impl_software *thiz,
                                       bgfx::ViewId _id, uint16_t _x,
                                       uint16_t _y, uint16_t _width,
                                       uint16_t _height) {
  thiz->view_set_rect(_id, _x, _y, _width, _height);
};

FORCE_INLINE void rast_api_setViewFrameBuffer(rast_impl_software *thiz,
                                              bgfx::ViewId _id,
                                              bgfx::FrameBufferHandle _handle) {
  thiz->view_set_framebuffer(_id, _handle);
};

FORCE_INLINE void rast_api_setViewClear(rast_impl_software *thiz,
                                        bgfx::ViewId _id, uint16_t _flags,
                                        uint32_t _rgba = 0x000000ff,
                                        float _depth = 1.0f,
                                        uint8_t _stencil = 0) {
  thiz->view_set_clear(_id, _flags, _rgba, _depth);
};

FORCE_INLINE void rast_api_setViewTransform(rast_impl_software *thiz,
                                            bgfx::ViewId _id, const void *_view,
                                            const void *_proj) {
  thiz->view_set_transform(_id, *(const m::mat<fix32, 4, 4> *)_view,
                           *(const m::mat<fix32, 4, 4> *)_proj);
};

FORCE_INLINE uint32_t rast_api_setTransform(rast_impl_software *thiz,
                                            const void *_mtx,
                                            uint16_t _num = 1) {
  thiz->set_transform(*(const m::mat<fix32, 4, 4> *)_mtx);
  return 0;
};

FORCE_INLINE void rast_api_setVertexBuffer(rast_impl_software *thiz,
                                           uint8_t _stream,
                                           bgfx::VertexBufferHandle _handle) {
  thiz->set_vertex_buffer(_handle);
};

FORCE_INLINE void rast_api_setIndexBuffer(rast_impl_software *thiz,
                                          bgfx::IndexBufferHandle _handle) {
  thiz->set_index_buffer(_handle);
};

FORCE_INLINE void rast_api_setState(rast_impl_software *thiz, uint64_t _state,
                                    uint32_t _rgba = 0) {
  thiz->set_state(_state, _rgba);
};

FORCE_INLINE void rast_api_submit(rast_impl_software *thiz, bgfx::ViewId _id,
                                  bgfx::ProgramHandle _program,
                                  uint32_t _depth = 0,
                                  uint8_t _flags = BGFX_DISCARD_ALL) {
  thiz->view_submit(_id, _program);
};

FORCE_INLINE uint32_t rast_api_frame(rast_impl_software *thiz,
                                     bool _capture = false) {
  thiz->frame();
  return 0;
};