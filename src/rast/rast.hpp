#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <cor/types.hpp>
#include <m/mat.hpp>
#include <m/vec.hpp>
#include <rast/algorithm.hpp>

struct bgfx_impl {

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
    bgfx::TextureHandle texture;
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

  struct AttribType : bgfx::AttribType {
    static uint8_t get_size(bgfx::AttribType::Enum p_attrib_type) {
      switch (p_attrib_type) {
      case bgfx::AttribType::Enum::Uint8:
        return sizeof(uint8_t);
      case bgfx::AttribType::Enum::Int16:
        return sizeof(int32_t);
      case bgfx::AttribType::Enum::Float:
        return sizeof(float);
      default:
        return 0;
      }
    };
  };

  struct CommandTemporaryStack {
    m::mat<f32, 4, 4> transform;
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

  struct CommandDrawCall {
    bgfx::ProgramHandle program;
    m::mat<f32, 4, 4> transform;
    bgfx::IndexBufferHandle index_buffer;
    bgfx::VertexBufferHandle vertex_buffer;
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

  struct RenderPass {
    bgfx::FrameBufferHandle framebuffer;
    m::vec<ui16, 2> rect;
    m::vec<ui16, 4> scissor;
    m::mat<f32, 4, 4> view;
    m::mat<f32, 4, 4> proj;

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
      return l_render_pass;
    };
  };

  struct heap {

    struct buffer_memory_table {
      table_heap_paged_meta;
      table_heap_paged_cols_1(ui8);
      table_define_heap_paged_1;
    } buffer_memory_table;

    struct buffers_table {
      table_heap_paged_meta;
      table_heap_paged_cols_2(bgfx::Memory, MemoryReference);
      table_define_heap_paged_2;
    } buffers_table;

    struct buffers_ptr_mapping_table {
      table_vector_meta;
      table_cols_2(bgfx::Memory *, uimax);
      table_define_vector_2;
    } buffers_ptr_mapping_table;

    struct texture_table {
      table_pool_meta;
      table_cols_1(Texture);
      table_define_pool_1;
    } texture_table;

    struct framebuffer_table {
      table_pool_meta;
      table_cols_1(FrameBuffer);
      table_define_pool_1;
    } framebuffer_table;

    struct vertexbuffer_table {
      table_pool_meta;
      table_cols_1(VertexBuffer);
      table_define_pool_1;
    } vertexbuffer_table;

    struct indexbuffer_table {
      table_pool_meta;
      table_cols_1(IndexBuffer);
      table_define_pool_1;
    } indexbuffer_table;

    struct renderpass_table {
      table_vector_meta;
      table_cols_1(RenderPass);
      table_define_vector_1;
    } renderpass_table;

    void allocate() {
      renderpass_table.allocate(0);
      buffer_memory_table.allocate(4096 * 4096);
      buffers_table.allocate(1024);
      buffers_ptr_mapping_table.allocate(0);
      texture_table.allocate(0);
      framebuffer_table.allocate(0);
      vertexbuffer_table.allocate(0);
      indexbuffer_table.allocate(0);

      renderpass_table.push_back(
          RenderPass::get_default()); // at least one renderpass
    };

    void free() {
      assert_debug(!vertexbuffer_table.has_allocated_elements());
      assert_debug(!indexbuffer_table.has_allocated_elements());
      assert_debug(renderpass_table.element_count() == 1);
      for (auto l_render_pass_it = 0;
           l_render_pass_it < renderpass_table.element_count();
           ++l_render_pass_it) {
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
    };

    bgfx::Memory *allocate_buffer(uimax p_size) {
      auto l_buffer_index = buffer_memory_table.push_back(p_size);

      bgfx::Memory l_buffer{};
      l_buffer.size = buffer_memory_table.at(l_buffer_index, &l_buffer.data);

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
        if (buffers_ptr_mapping_table.m_col_0[i] == p_buffer) {

          MemoryReference *l_reference;
          uimax l_buffers_table_count = buffers_table.at(
              buffers_ptr_mapping_table.m_col_1[i], orm::none(), &l_reference);
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
    allocate_frame_buffer(bgfx::TextureHandle p_texture) {
      FrameBuffer l_frame_buffer;
      l_frame_buffer.texture = p_texture;

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
      free_texture(l_frame_buffer->texture);
      framebuffer_table.remove_at(p_frame_buffer.idx);
    };

    bgfx::VertexBufferHandle
    allocate_vertex_buffer(const bgfx::Memory *p_memory,
                           const bgfx::VertexLayout &p_layout) {
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

  } heap;

  struct TextureProxy {
    struct heap &m_heap;
    Texture *m_value;

    Texture *value() { return m_value; };
  };

  struct FrameBufferProxy {
    struct heap &m_heap;
    FrameBuffer *m_value;

    TextureProxy Texture() {
      struct Texture *l_texture;
      m_heap.texture_table.at(m_value->texture.idx, &l_texture);
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

  bgfx::TextureHandle allocate_texture(uint16_t p_width, uint16_t p_height,
                                       bool p_hasMips, uint16_t p_numLayers,
                                       bgfx::TextureFormat::Enum p_format,
                                       uint64_t p_flags) {
    bgfx::TextureInfo l_texture_info{};
    l_texture_info.format = p_format;
    l_texture_info.width = p_width;
    l_texture_info.height = p_height;
    l_texture_info.bitsPerPixel = get_pixel_size_from_texture_format(p_format);
    return heap.allocate_texture(l_texture_info);
  };

  bgfx::FrameBufferHandle
  allocate_frame_buffer(uint16_t p_width, uint16_t p_height,
                        bgfx::TextureFormat::Enum p_format,
                        uint64_t p_textureFlags) {
    auto l_texture =
        allocate_texture(p_width, p_height, 0, 0, p_format, p_textureFlags);
    return heap.allocate_frame_buffer(l_texture);
  };

  void view_set_rect(bgfx::ViewId p_id, uint16_t p_x, uint16_t p_y,
                     uint16_t p_width, uint16_t p_height) {
    proxy().RenderPass(p_id).value()->rect = {p_x, p_y};
  };

  void view_set_framebuffer(bgfx::ViewId p_id,
                            bgfx::FrameBufferHandle p_handle) {
    proxy().RenderPass(p_id).value()->framebuffer = p_handle;
  };

  void view_set_transform(bgfx::ViewId p_id, const m::mat<f32, 4, 4> &p_view,
                          const m::mat<f32, 4, 4> &p_proj) {
    RenderPassProxy l_render_pass = proxy().RenderPass(p_id);
    l_render_pass.value()->view = p_view;
    l_render_pass.value()->proj = p_proj;
  };

  void view_submit(bgfx::ViewId p_id, bgfx::ProgramHandle p_program) {
    CommandDrawCall l_draw_call;
    l_draw_call.program = p_program;
    l_draw_call.make_from_temporary_stack(command_temporary_stack);
    command_temporary_stack.clear();

    proxy().RenderPass(p_id).value()->commands.push_back(l_draw_call);
  };

  void set_transform(const m::mat<f32, 4, 4> &p_transform) {
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
      TextureProxy l_frame_texture = p_render_pass.FrameBuffer().Texture();

      container::range<ui8> l_frame_texture_range =
          l_frame_texture.value()->range();

      p_render_pass.for_each_commands([&](CommandDrawCall &p_command) {
        CommandDrawCallProxy l_draw_call(heap, &p_command);
        IndexBuffer *l_index_buffer = l_draw_call.IndexBuffer();
        VertexBuffer *l_vertex_buffer = l_draw_call.VertexBuffer();

        rast::algorithm::rasterize(
            rast::algorithm::shader(), p_render_pass.value()->rect,
            p_render_pass.value()->proj, p_render_pass.value()->view,
            l_draw_call.value()->transform, l_index_buffer->range(),
            l_vertex_buffer->layout, l_vertex_buffer->range(),
            l_draw_call.value()->state, l_draw_call.value()->rgba,
            l_frame_texture.value()->info, l_frame_texture_range);
      });
    });

    proxy().for_each_renderpass([&](RenderPassProxy &p_render_passs) {
      p_render_passs.value()->commands.clear();
    });
  };

  void initialize() {
    heap.allocate();
    command_temporary_stack.clear();
  };

  void terminate() {

    heap.free();
    // TODO
  };

private:
  ui8 get_pixel_size_from_texture_format(bgfx::TextureFormat::Enum p_format) {
    switch (p_format) {
    case bgfx::TextureFormat::Enum::R8:
      return ui8(8);
    case bgfx::TextureFormat::Enum::RG8:
      return ui8(16);
    case bgfx::TextureFormat::Enum::RGB8:
      return ui8(24);
    case bgfx::TextureFormat::Enum::RGBA8:
      return ui8(32);
    }
    return ui8(0);
  };
};

static bgfx_impl bgfx_impl;

namespace bgfx {

inline Init::Init(){};
inline Resolution::Resolution(){};
inline Init::Limits::Limits(){};
inline PlatformData::PlatformData(){};

inline VertexLayout::VertexLayout(){};

inline VertexLayout &VertexLayout::begin(RendererType::Enum _renderer) {
  m_hash = 0;
  m_stride = 0;
  for (int8_t i = 0; i < Attrib::Count; ++i) {
    m_attributes[i] = UINT16_MAX;
    m_offset[i] = 0;
  }
  return *this;
};

inline void VertexLayout::end(){};

inline VertexLayout &VertexLayout::add(Attrib::Enum _attrib, uint8_t _num,
                                       AttribType::Enum _type, bool _normalized,
                                       bool _asInt) {
  assert_debug(!has(_attrib));
  m_attributes[_attrib] = _attrib;
  m_offset[_attrib] = m_stride;
  m_stride += bgfx_impl::AttribType::get_size(_type) * _num;

  ui8 l_byte_offset = (9 * _attrib);
  m_hash = m_hash | ((0b1111 & _num) << l_byte_offset);
  m_hash = m_hash | ((0b111 & _type) << (l_byte_offset + 4));
  m_hash = m_hash | ((0b1 & _normalized) << (l_byte_offset + 7));
  m_hash = m_hash | ((0b1 & _asInt) << (l_byte_offset + 8));
  return *this;
};

inline void VertexLayout::decode(Attrib::Enum _attrib, uint8_t &_num,
                                 AttribType::Enum &_type, bool &_normalized,
                                 bool &_asInt) const {
  uint32_t l_attrib_bytes = m_hash >> (9 * _attrib);
  _num = l_attrib_bytes & 0b1111;
  _type = (AttribType::Enum)(l_attrib_bytes & 0b1110000);
  _normalized = l_attrib_bytes & 0b10000000;
  _asInt = l_attrib_bytes & 0b100000000;
};

inline bool init(const Init &p_init) {
  bgfx_impl.initialize();
  return 1;
};

inline void shutdown() { bgfx_impl.terminate(); };

inline const Memory *alloc(uint32_t _size) {
  return bgfx_impl.heap.allocate_buffer(uimax(_size));
};

inline const Memory *makeRef(const void *_data, uint32_t _size,
                             ReleaseFn _releaseFn, void *_userData) {
  return bgfx_impl.heap.allocate_ref(_data, _size);
};

inline TextureHandle createTexture2D(uint16_t _width, uint16_t _height,
                                     bool _hasMips, uint16_t _numLayers,
                                     TextureFormat::Enum _format,
                                     uint64_t _flags, const Memory *_mem) {
  return bgfx_impl.allocate_texture(_width, _height, _hasMips, _numLayers,
                                    _format, _flags);
};

inline FrameBufferHandle createFrameBuffer(uint16_t _width, uint16_t _height,
                                           TextureFormat::Enum _format,
                                           uint64_t _textureFlags) {
  return bgfx_impl.allocate_frame_buffer(_width, _height, _format,
                                         _textureFlags);
};

inline VertexBufferHandle createVertexBuffer(const Memory *_mem,
                                             const VertexLayout &_layout,
                                             uint16_t _flags) {
  return bgfx_impl.heap.allocate_vertex_buffer(_mem, _layout);
};

inline void destroy(VertexBufferHandle _handle) {
  return bgfx_impl.heap.free_vertex_buffer(_handle);
};

inline IndexBufferHandle createIndexBuffer(const Memory *_mem,
                                           uint16_t _flags) {
  return bgfx_impl.heap.allocate_index_buffer(_mem);
};

inline void destroy(IndexBufferHandle _handle) {
  return bgfx_impl.heap.free_index_buffer(_handle);
};

inline void setViewRect(ViewId _id, uint16_t _x, uint16_t _y, uint16_t _width,
                        uint16_t _height) {
  bgfx_impl.view_set_rect(_id, _x, _y, _width, _height);
};

inline void setViewFrameBuffer(ViewId _id, FrameBufferHandle _handle) {
  bgfx_impl.view_set_framebuffer(_id, _handle);
};

inline void setViewTransform(ViewId _id, const void *_view, const void *_proj) {
  bgfx_impl.view_set_transform(_id, *(const m::mat<f32, 4, 4> *)_view,
                               *(const m::mat<f32, 4, 4> *)_proj);
};

inline uint32_t setTransform(const void *_mtx, uint16_t _num) {
  bgfx_impl.set_transform(*(const m::mat<f32, 4, 4> *)_mtx);
  return -1;
};

inline void setVertexBuffer(uint8_t _stream, VertexBufferHandle _handle) {
  bgfx_impl.set_vertex_buffer(_handle);
};

inline void setIndexBuffer(IndexBufferHandle _handle) {
  bgfx_impl.set_index_buffer(_handle);
};

inline void setState(uint64_t _state, uint32_t _rgba) {
  bgfx_impl.set_state(_state, _rgba);
};

inline void touch(ViewId _id){/* bgfx_impl.view_submit(_id); */};

inline void submit(ViewId _id, ProgramHandle _program, uint32_t _depth,
                   uint8_t _flags) {
  bgfx_impl.view_submit(_id, _program);
};

inline uint32_t frame(bool _capture) {
  bgfx_impl.frame();
  return 0;
};

}; // namespace bgfx