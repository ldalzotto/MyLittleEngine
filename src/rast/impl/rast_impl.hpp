#pragma once

#include <m/color.hpp>
#include <m/geom.hpp>
#include <rast/impl/algorithm.hpp>
#include <rast/model.hpp>

struct rast_impl_software {
  struct MemoryReference {
    uimax m_buffer_index;
    ui8 is_ref() { return m_buffer_index == -1; };
    MemoryReference() = default;
    MemoryReference(uimax p_buffer_index) : m_buffer_index(p_buffer_index){};
  };

  struct Texture {
    bgfx::TextureInfo info;
    bgfx::Memory *buffer;

    container::range<ui8> range() {
      return container::range<ui8>::make(buffer->data, buffer->size);
    };
  };

  struct FrameBuffer {
    bgfx::TextureHandle m_rgb;
    bgfx::TextureHandle m_depth;

    ui8 has_depth() const { return m_depth.idx != bgfx::kInvalidHandle; };
  };

  struct VertexBuffer {
    bgfx::VertexLayout layout;
    const bgfx::Memory *memory;

    container::range<ui8> range() {
      return container::range<ui8>::make(memory->data, memory->size);
    };
  };

  struct IndexBuffer {
    const bgfx::Memory *memory;

    container::range<ui8> range() {
      return container::range<ui8>::make(memory->data, memory->size);
    };
  };

  struct Shader {
    const bgfx::Memory *m_buffer;
  };

  struct Program {
    bgfx::ShaderHandle vertex;
    bgfx::ShaderHandle fragment;
  };

  struct CommandTemporaryStack {
    m::mat<fix32, 4, 4> transform;
    bgfx::VertexBufferHandle vertex_buffer;
    bgfx::IndexBufferHandle index_buffer;
    ui64 state;
    ui32 rgba;

    void clear() {
      transform = transform.getIdentity();
      vertex_buffer.idx = bgfx::kInvalidHandle;
      index_buffer.idx = bgfx::kInvalidHandle;
      state = -1;
      rgba = -1;
    };
  } command_temporary_stack;

  struct CommandUniform {
    union {
      m::vec<fix32, 4> m_vecs;
    };
  };

  // TODO -> use a stack heap instead
  struct CommandUniforms {
    uimax m_count;
    container::arr<CommandUniform, 255> m_uniforms;
  };

  struct CommandDrawCall {
    bgfx::ProgramHandle program;
    m::mat<fix32, 4, 4> transform;
    bgfx::IndexBufferHandle index_buffer;
    bgfx::VertexBufferHandle vertex_buffer;
    CommandUniforms vertex_uniforms;
    ui64 state;
    ui32 rgba;

    void
    make_from_temporary_stack(const CommandTemporaryStack &p_temporary_stack) {
      transform = p_temporary_stack.transform;
      index_buffer = p_temporary_stack.index_buffer;
      vertex_buffer = p_temporary_stack.vertex_buffer;
      state = p_temporary_stack.state;
      rgba = p_temporary_stack.rgba;
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

  struct RenderPass {
    bgfx::FrameBufferHandle framebuffer;
    clear_state clear;
    m::rect_point_extend<ui16> rect;
    m::vec<ui16, 4> scissor;
    m::mat<fix32, 4, 4> view;
    m::mat<fix32, 4, 4> proj;

    container::vector<CommandDrawCall> commands;

    void allocate() { commands.allocate(0); };
    void free() { commands.free(); };

    static RenderPass get_default() {
      RenderPass l_render_pass;
      l_render_pass.allocate();
      l_render_pass.framebuffer.idx = 0;
      l_render_pass.rect = l_render_pass.rect.getZero();
      l_render_pass.scissor = l_render_pass.scissor.getZero();
      l_render_pass.view = l_render_pass.view.getZero();
      l_render_pass.proj = l_render_pass.proj.getZero();
      l_render_pass.clear.reset();
      return l_render_pass;
    };
  };

  struct Uniform {
    bgfx::UniformType::Enum type;
    uimax hash;
    uimax index;
    uimax usage_count;
  };

  struct heap {

    orm::table_heap_paged_v2<ui8> buffer_memory_table;

    orm::table_heap_paged_v2<bgfx::Memory, MemoryReference> buffers_table;

    using buffers_ptr_mapping_buffer_t = bgfx::Memory *;
    using buffers_ptr_mapping_buffer_index_t = uimax;
    using buffers_ptr_mapping_t =
        orm::table_vector_v2<buffers_ptr_mapping_buffer_t,
                             buffers_ptr_mapping_buffer_index_t>;

    buffers_ptr_mapping_t buffers_ptr_mapping_table;

    orm::table_pool_v2<Texture> texture_table;
    orm::table_pool_v2<FrameBuffer> framebuffer_table;
    orm::table_pool_v2<VertexBuffer> vertexbuffer_table;
    orm::table_pool_v2<IndexBuffer> indexbuffer_table;
    orm::table_vector_v2<RenderPass> renderpass_table;
    orm::table_pool_v2<Shader> shader_table;

    struct {
      container::pool<m::vec<fix32, 4>> vecs;
    } uniform_values;

    struct {
      container::pool<Uniform> by_index;
      container::hashmap<uimax, uimax> by_key;
    } uniforms;

    orm::table_pool_v2<Program> program_table;

    void allocate() {
      renderpass_table.allocate(0);
      buffer_memory_table.allocate(4096 * 4096);
      buffers_table.allocate(1024);
      buffers_ptr_mapping_table.allocate(0);
      texture_table.allocate(0);
      framebuffer_table.allocate(0);
      vertexbuffer_table.allocate(0);
      indexbuffer_table.allocate(0);
      shader_table.allocate(0);
      program_table.allocate(0);

      uniform_values.vecs.allocate(0);
      uniforms.by_index.allocate(0);
      uniforms.by_key.allocate();

      renderpass_table.push_back(
          RenderPass::get_default()); // at least one renderpass
    };

    void free() {
      assert_debug(!uniforms.by_key.has_allocated_elements());
      assert_debug(!uniforms.by_index.has_allocated_elements());
      assert_debug(!vertexbuffer_table.has_allocated_elements());
      assert_debug(!indexbuffer_table.has_allocated_elements());
      assert_debug(renderpass_table.count() == 1);
      assert_debug(!shader_table.has_allocated_elements());
      assert_debug(!program_table.has_allocated_elements());
      assert_debug(!framebuffer_table.has_allocated_elements());

      uniforms.by_key.free();
      uniforms.by_index.free();

      for (auto l_render_pass_it = 0;
           l_render_pass_it < renderpass_table.count(); ++l_render_pass_it) {
        RenderPass *l_render_pass;
        renderpass_table.at(l_render_pass_it, &l_render_pass);
        l_render_pass->free();
      }
      renderpass_table.free();
      buffer_memory_table.free();
      buffers_table.free();
      buffers_ptr_mapping_table.free();
      texture_table.free();
      vertexbuffer_table.free();
      indexbuffer_table.free();
      shader_table.free();
      program_table.free();
      framebuffer_table.free();
    };

    bgfx::Memory *allocate_buffer(uimax p_size) {
      auto l_buffer_index = buffer_memory_table.push_back(p_size);

      bgfx::Memory l_buffer{};
      ui8 *l_data;
      l_buffer.size = buffer_memory_table.at(l_buffer_index, &l_data);
      l_buffer.data = l_data;

      uimax l_index = buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      MemoryReference *l_memory_refence;
      uimax l_memory_count =
          buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = MemoryReference(l_buffer_index);

      buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    bgfx::Memory *allocate_buffer(uimax p_size, uimax p_alignment) {
      auto l_buffer_index = buffer_memory_table.push_back(p_size, p_alignment);

      bgfx::Memory l_buffer{};
      ui8 *l_data;
      l_buffer.size = buffer_memory_table.at(l_buffer_index, &l_data);
      l_buffer.data = l_data;

      uimax l_index = buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      MemoryReference *l_memory_refence;
      uimax l_memory_count =
          buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = MemoryReference(l_buffer_index);

      buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    bgfx::Memory *allocate_ref(const void *p_ptr, ui32 p_size) {
      bgfx::Memory l_buffer{};
      l_buffer.data = (ui8 *)p_ptr;
      l_buffer.size = p_size;

      uimax l_index = buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      MemoryReference *l_memory_refence;
      uimax l_memory_count =
          buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = MemoryReference(-1);

      buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    void free_buffer(const bgfx::Memory *p_buffer) {

      for (auto i = 0; i < buffers_ptr_mapping_table.m_meta.m_count; ++i) {
        buffers_ptr_mapping_buffer_t *l_buffer;
        buffers_ptr_mapping_table.at(i, &l_buffer, none());
        if (*l_buffer == p_buffer) {
          buffers_ptr_mapping_buffer_index_t *l_buffer_index;
          buffers_ptr_mapping_table.at(i, none(), &l_buffer_index);
          MemoryReference *l_reference;
          uimax l_buffers_table_count =
              buffers_table.at(*l_buffer_index, none(), &l_reference);
          assert_debug(l_buffers_table_count == 1);
          if (!l_reference->is_ref()) {
            buffer_memory_table.remove_at(l_reference->m_buffer_index);
          }
          buffers_ptr_mapping_table.remove_at(i);
          return;
        }
      }
    };

    bgfx::TextureHandle
    allocate_texture(const bgfx::TextureInfo &p_texture_info) {
      uimax l_image_size = uimax(p_texture_info.bitsPerPixel *
                                 p_texture_info.width * p_texture_info.height);
      Texture l_texture;
      l_texture.info = p_texture_info;
      l_texture.buffer = allocate_buffer(l_image_size);
      bgfx::TextureHandle l_texture_handle;
      l_texture_handle.idx = texture_table.push_back(l_texture);

      return l_texture_handle;
    };

    void free_texture(bgfx::TextureHandle p_texture) {
      Texture *l_texture;
      texture_table.at(p_texture.idx, &l_texture);
      free_buffer(l_texture->buffer);
      texture_table.remove_at(p_texture.idx);
    };

    bgfx::FrameBufferHandle
    allocate_frame_buffer(bgfx::TextureHandle p_rgb_texture,
                          bgfx::TextureHandle p_depth_texture) {
      FrameBuffer l_frame_buffer;
      l_frame_buffer.m_rgb = p_rgb_texture;
      l_frame_buffer.m_depth = p_depth_texture;

      bgfx::FrameBufferHandle l_handle;
      l_handle.idx = framebuffer_table.push_back(l_frame_buffer);
      return l_handle;
    };

    FrameBuffer *get_frame_buffer(bgfx::FrameBufferHandle p_frame_buffer) {
      FrameBuffer *l_frame_buffer;
      framebuffer_table.at(p_frame_buffer.idx, &l_frame_buffer);
      return l_frame_buffer;
    };

    void free_frame_buffer(bgfx::FrameBufferHandle p_frame_buffer) {
      FrameBuffer *l_frame_buffer = get_frame_buffer(p_frame_buffer);
      free_texture(l_frame_buffer->m_rgb);
      if (l_frame_buffer->has_depth()) {
        free_texture(l_frame_buffer->m_depth);
      }
      framebuffer_table.remove_at(p_frame_buffer.idx);
    };

    bgfx::VertexBufferHandle
    allocate_vertex_buffer(const bgfx::Memory *p_memory,
                           const bgfx::VertexLayout &p_layout) {

      assert_debug((p_memory->size % p_layout.getStride()) == 0);
      assert_debug((p_memory->size % 2) == 0); // memory is aligned

      VertexBuffer l_vertex_buffer;
      l_vertex_buffer.layout = p_layout;
      l_vertex_buffer.memory = p_memory;
      bgfx::VertexBufferHandle l_handle;
      l_handle.idx = vertexbuffer_table.push_back(l_vertex_buffer);
      return l_handle;
    };

    void free_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
      VertexBuffer *l_vertex_buffer;
      vertexbuffer_table.at(p_handle.idx, &l_vertex_buffer);
      free_buffer(l_vertex_buffer->memory);
      vertexbuffer_table.remove_at(p_handle.idx);
    };

    bgfx::IndexBufferHandle
    allocate_index_buffer(const bgfx::Memory *p_memory) {
      IndexBuffer l_index_buffer;
      l_index_buffer.memory = p_memory;
      bgfx::IndexBufferHandle l_handle;
      l_handle.idx = indexbuffer_table.push_back(l_index_buffer);
      return l_handle;
    };

    void free_index_buffer(bgfx::IndexBufferHandle p_handle) {
      IndexBuffer *l_index_buffer;
      indexbuffer_table.at(p_handle.idx, &l_index_buffer);
      free_buffer(l_index_buffer->memory);
      indexbuffer_table.remove_at(p_handle.idx);
    };

    bgfx::ShaderHandle allocate_shader(const bgfx::Memory *p_memory) {
      bgfx::ShaderHandle l_handle;
      Shader l_shader;
      l_shader.m_buffer = p_memory;
      l_handle.idx = shader_table.push_back(l_shader, 0);
      return l_handle;
    };

    void free_shader(bgfx::ShaderHandle p_handle) {
      shader_table.remove_at(p_handle.idx);
    };

    bgfx::ProgramHandle allocate_program(const Program &p_program) {
      bgfx::ProgramHandle l_handle;
      l_handle.idx = program_table.push_back(p_program);
      return l_handle;
    };

    void free_program(bgfx::ProgramHandle p_handle) {
      program_table.remove_at(p_handle.idx);
    };

    bgfx::UniformHandle allocate_uniform(const ui8 *p_name,
                                         bgfx::UniformType::Enum p_type) {
      auto l_uniform_hash = algorithm::hash(
          container::range<ui8>::make((ui8 *)p_name, sys::strlen(p_name)));
      uimax l_uniform_index;
      if (!uniforms.by_key.has_key(l_uniform_hash)) {
        Uniform l_uniform;
        l_uniform.usage_count = 0;
        l_uniform.hash = l_uniform_hash;
        l_uniform.type = p_type;
        if (p_type == bgfx::UniformType::Enum::Vec4) {
          l_uniform.index = uniform_values.vecs.push_back({});
        } else {
          sys::abort();
        }
        l_uniform_index = uniforms.by_index.push_back(l_uniform);
        uniforms.by_key.push_back(l_uniform_hash, l_uniform_index);
      } else {
        l_uniform_index = uniforms.by_key.at(l_uniform_hash);
      }

      uniforms.by_index.at(uniforms.by_key.at(l_uniform_hash)).usage_count += 1;

      bgfx::UniformHandle l_handle;
      l_handle.idx = l_uniform_index;
      return l_handle;
    };

    void free_uniform(bgfx::UniformHandle p_uniform) {
      Uniform &l_uniform = uniforms.by_index.at(p_uniform.idx);
      l_uniform.usage_count -= 1;
      if (l_uniform.usage_count == 0) {
        uniforms.by_key.remove_at(l_uniform.hash);
        uniforms.by_index.remove_at(p_uniform.idx);
      }
    };

  } heap;

  rast::algorithm::rasterize_heap m_rasterize_heap;

  struct TextureProxy {
    struct heap &m_heap;
    Texture *m_value;

    Texture *value() { return m_value; };
  };

  struct FrameBufferProxy {
    struct heap &m_heap;
    FrameBuffer *m_value;

    TextureProxy RGBTexture() {
      Texture *l_texture;
      m_heap.texture_table.at(m_value->m_rgb.idx, &l_texture);
      return {.m_heap = m_heap, .m_value = l_texture};
    };

    TextureProxy DepthTexture() {
      Texture *l_texture;
      m_heap.texture_table.at(m_value->m_depth.idx, &l_texture);
      return {.m_heap = m_heap, .m_value = l_texture};
    };
  };

  struct RenderPassProxy {
    struct heap &m_heap;
    RenderPass *m_value;

    RenderPassProxy(struct heap &p_heap, RenderPass *p_value)
        : m_heap(p_heap), m_value(p_value){

                          };
    RenderPass *value() { return m_value; };

    FrameBufferProxy FrameBuffer() {
      struct FrameBuffer *l_frame_buffer;
      m_heap.framebuffer_table.at(m_value->framebuffer.idx, &l_frame_buffer);
      return {.m_heap = m_heap, .m_value = l_frame_buffer};
    };

    template <typename Callback> void for_each_commands(const Callback &p_cb) {
      for (auto l_it = 0; l_it < m_value->commands.count(); ++l_it) {
        p_cb(m_value->commands.at(l_it));
      }
    };
  };

  struct ShaderProxy {
    struct heap &m_heap;
    Shader *m_shader;
  };

  struct ProgramProxy {
    struct heap &m_heap;
    Program *m_program;

    ProgramProxy(struct heap &p_heap, Program *p_program)
        : m_heap(p_heap), m_program(p_program){

                          };

    ShaderProxy VertexShader() {
      Shader *l_shader;
      m_heap.shader_table.at(m_program->vertex.idx, &l_shader);
      return {.m_heap = m_heap, .m_shader = l_shader};
    };

    ShaderProxy FragmentShader() {
      Shader *l_shader;
      m_heap.shader_table.at(m_program->fragment.idx, &l_shader);
      return {.m_heap = m_heap, .m_shader = l_shader};
    };
  };

  struct CommandDrawCallProxy {
    struct heap &m_heap;
    CommandDrawCall *m_value;

    CommandDrawCallProxy(struct heap &p_heap, CommandDrawCall *p_value)
        : m_heap(p_heap), m_value(p_value){

                          };

    CommandDrawCall *value() { return m_value; };

    IndexBuffer *IndexBuffer() {
      struct IndexBuffer *l_index_buffer;
      m_heap.indexbuffer_table.at(m_value->index_buffer.idx, &l_index_buffer);
      return l_index_buffer;
    };

    VertexBuffer *VertexBuffer() {
      struct VertexBuffer *l_vertex_buffer;
      m_heap.vertexbuffer_table.at(m_value->vertex_buffer.idx,
                                   &l_vertex_buffer);
      return l_vertex_buffer;
    };

    ProgramProxy Program() {
      struct Program *l_program;
      m_heap.program_table.at(m_value->program.idx, &l_program);
      return ProgramProxy(m_heap, l_program);
    };
  };

  struct heap_proxy {
    struct heap &m_heap;

    FrameBufferProxy FrameBuffer(bgfx::FrameBufferHandle p_handle) {
      struct FrameBuffer *l_frame_buffer = m_heap.get_frame_buffer(p_handle);
      return {.m_heap = m_heap, .m_value = l_frame_buffer};
    };

    RenderPassProxy RenderPass(bgfx::ViewId p_handle) {
      struct RenderPass *l_render_pass;
      m_heap.renderpass_table.at(p_handle, &l_render_pass);
      return RenderPassProxy(m_heap, l_render_pass);
    };

    TextureProxy Texture(bgfx::TextureHandle p_handle) {
      struct Texture *l_texture;
      m_heap.texture_table.at(p_handle.idx, &l_texture);
      return TextureProxy{.m_heap = m_heap, .m_value = l_texture};
    };

    template <typename Callback>
    void for_each_renderpass(const Callback &p_cb) {
      for (auto i = 0; i < m_heap.renderpass_table.m_meta.m_count; ++i) {
        struct RenderPass *l_render_pass;
        m_heap.renderpass_table.at(i, &l_render_pass);
        RenderPassProxy l_proxy(m_heap, l_render_pass);
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
    Program l_program;
    l_program.vertex = p_vertex;
    l_program.fragment = p_fragment;
    return heap.allocate_program(l_program);
  };

  void view_set_rect(bgfx::ViewId p_id, uint16_t p_x, uint16_t p_y,
                     uint16_t p_width, uint16_t p_height) {
    m::rect_point_extend<ui16> &l_view = proxy().RenderPass(p_id).value()->rect;
    l_view.point() = {p_x, p_y};
    l_view.extend() = {p_width, p_height};
  };

  void view_set_framebuffer(bgfx::ViewId p_id,
                            bgfx::FrameBufferHandle p_handle) {
    proxy().RenderPass(p_id).value()->framebuffer = p_handle;
  };

  void view_set_clear(bgfx::ViewId p_id, ui16 p_flags, ui32 p_rgba,
                      fix32 p_depth) {
    clear_state &l_clear = proxy().RenderPass(p_id).value()->clear;
    l_clear.m_rgba.rgba = p_rgba;
    l_clear.m_flags.m_int = p_flags;
    l_clear.m_depth = p_depth;
  };

  void view_set_transform(bgfx::ViewId p_id, const m::mat<fix32, 4, 4> &p_view,
                          const m::mat<fix32, 4, 4> &p_proj) {
    RenderPassProxy l_render_pass = proxy().RenderPass(p_id);
    l_render_pass.value()->view = p_view;
    l_render_pass.value()->proj = p_proj;
  };

  void view_submit(bgfx::ViewId p_id, bgfx::ProgramHandle p_program) {
    CommandDrawCall l_draw_call;
    l_draw_call.program = p_program;
    l_draw_call.make_from_temporary_stack(command_temporary_stack);
    command_temporary_stack.clear();

    Program *__program;
    heap.program_table.at(l_draw_call.program.idx, &__program);
    ProgramProxy l_program = ProgramProxy(heap, __program);

    rast::algorithm::program l_rasterizer_program;
    l_rasterizer_program.m_vertex =
        l_program.VertexShader().m_shader->m_buffer->data;
    l_rasterizer_program.m_fragment =
        l_program.FragmentShader().m_shader->m_buffer->data;

    rast::algorithm::program_uniforms l_vertex_uniforms;
    auto l_vertex_shader_uniforms =
        rast::shader_vertex_bytes::view{(ui8 *)l_rasterizer_program.m_vertex}
            .uniforms();
    for (auto l_vertex_uniform_it = 0;
         l_vertex_uniform_it < l_vertex_shader_uniforms.count();
         ++l_vertex_uniform_it) {
      rast::shader_uniform &l_shader_uniform =
          l_vertex_shader_uniforms.at(l_vertex_uniform_it);
      if (l_shader_uniform.type == bgfx::UniformType::Vec4) {
        l_draw_call.vertex_uniforms.m_uniforms.at(l_vertex_uniform_it).m_vecs =
            *__get_uniform_vec4(l_shader_uniform.hash);
      }
    }

    l_draw_call.vertex_uniforms.m_count = l_vertex_shader_uniforms.count();

    proxy().RenderPass(p_id).value()->commands.push_back(l_draw_call);
  };

  void set_transform(const m::mat<fix32, 4, 4> &p_transform) {
    command_temporary_stack.transform = p_transform;
  };

  void set_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
    command_temporary_stack.vertex_buffer = p_handle;
  };

  void set_index_buffer(bgfx::IndexBufferHandle p_handle) {
    command_temporary_stack.index_buffer = p_handle;
  };

  void set_state(uint64_t p_state, uint32_t p_rgba) {
    command_temporary_stack.state = p_state;
    command_temporary_stack.rgba = p_rgba;
  };

  void frame() {

    proxy().for_each_renderpass([&](RenderPassProxy &p_render_pass) {
      FrameBufferProxy l_frame_buffer = p_render_pass.FrameBuffer();
      TextureProxy l_frame_rgb_texture = l_frame_buffer.RGBTexture();
      container::range<ui8> l_frame_rgb_texture_range =
          l_frame_rgb_texture.value()->range();

      container::range<ui8> l_frame_depth_texture_range;
      bgfx::TextureInfo l_frame_depth_texture_info;

      if (l_frame_buffer.m_value->has_depth()) {
        TextureProxy l_frame_depth_texture =
            p_render_pass.FrameBuffer().DepthTexture();
        l_frame_depth_texture_range = l_frame_depth_texture.value()->range();
        l_frame_depth_texture_info = l_frame_depth_texture.value()->info;
      } else {
        l_frame_depth_texture_range = container::range<ui8>::make(0, 0);
        l_frame_depth_texture_info.bitsPerPixel = 0;
      }

      // color clear
      {
        const clear_state &l_clear_state = p_render_pass.value()->clear;
        if (l_clear_state.m_flags.m_color) {
          Texture *l_texture = l_frame_rgb_texture.value();
          rast::image_view l_target_view(
              l_texture->info.width, l_texture->info.height,
              l_texture->info.bitsPerPixel, l_frame_rgb_texture_range);
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

      p_render_pass.for_each_commands([&](CommandDrawCall &p_command) {
        CommandDrawCallProxy l_draw_call(heap, &p_command);
        IndexBuffer *l_index_buffer = l_draw_call.IndexBuffer();
        VertexBuffer *l_vertex_buffer = l_draw_call.VertexBuffer();
        ProgramProxy l_program = l_draw_call.Program();
        rast::algorithm::program l_rasterizer_program;
        l_rasterizer_program.m_vertex =
            l_program.VertexShader().m_shader->m_buffer->data;
        l_rasterizer_program.m_fragment =
            l_program.FragmentShader().m_shader->m_buffer->data;

        rast::algorithm::program_uniforms l_vertex_uniforms;
        for (auto i = 0; i < l_draw_call.m_value->vertex_uniforms.m_count;
             ++i) {
          l_vertex_uniforms.at(i) =
              &l_draw_call.m_value->vertex_uniforms.m_uniforms.at(i).m_vecs;
        }

        rast::algorithm::rasterize(
            m_rasterize_heap, l_rasterizer_program, p_render_pass.value()->rect,
            p_render_pass.value()->proj, p_render_pass.value()->view,
            l_draw_call.value()->transform, l_index_buffer->range(),
            l_vertex_buffer->layout, l_vertex_buffer->range(),
            l_vertex_uniforms, l_draw_call.value()->state,
            l_draw_call.value()->rgba, l_frame_rgb_texture.value()->info,
            l_frame_rgb_texture_range, l_frame_depth_texture_info,
            l_frame_depth_texture_range);
      });
    });

    proxy().for_each_renderpass([&](RenderPassProxy &p_render_passs) {
      p_render_passs.value()->commands.clear();
    });
  };

  void initialize() {
    heap.allocate();
    m_rasterize_heap.allocate();
    command_temporary_stack.clear();
  };

  void terminate() {

    heap.free();
    m_rasterize_heap.free();
    // TODO
  };

private:
  const m::vec<fix32, 4> *__get_uniform_vec4(uimax p_hash) {
    return __get_uniform(p_hash).cast_to<m::vec<fix32, 4>>().data();
  }

  container::range<ui8> __get_uniform(uimax p_hash) {
    auto &l_uniform =
        heap.uniforms.by_index.at(heap.uniforms.by_key.at(p_hash));
    if (l_uniform.type == bgfx::UniformType::Vec4) {
      auto &l_value = heap.uniform_values.vecs.at(l_uniform.index);
      return container::range<ui8>::make((ui8 *)&l_value, sizeof(l_value));
    }
    return container::range<ui8>::make(0, 0);
  };

  container::range<ui8> __get_uniform(bgfx::UniformHandle p_handle) {
    auto &l_uniform = heap.uniforms.by_index.at(p_handle.idx);
    if (l_uniform.type == bgfx::UniformType::Vec4) {
      auto &l_value = heap.uniform_values.vecs.at(l_uniform.index);
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