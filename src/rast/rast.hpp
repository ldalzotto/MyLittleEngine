#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <cor/types.hpp>
#include <m/mat.hpp>
#include <m/vec.hpp>

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
  };

  struct FrameBuffer {
    bgfx::TextureHandle texture;
  };

  struct VertexBuffer {
    bgfx::VertexLayout layout;
    const bgfx::Memory *memory;
  };

  struct IndexBuffer {
    const bgfx::Memory *memory;
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
    void clear() {
      transform.setIdentity();
      vertex_buffer.idx = bgfx::kInvalidHandle;
      index_buffer.idx = bgfx::kInvalidHandle;
    };
  } command_temporary_stack;

  struct CommandType {
    enum Enum { Undefined = 0, DrawCall = 1 };
  };

  struct CommandHeader {
    CommandType::Enum type;
    CommandHeader() = default;
    CommandHeader(CommandType::Enum p_type) : type(p_type){};
  };

  struct Command_DrawCall {
    CommandHeader header = CommandHeader(CommandType::Enum::DrawCall);
    m::mat<f32, 4, 4> transform;
    bgfx::IndexBufferHandle index_buffer;
    bgfx::VertexBufferHandle vertex_buffer;

    void
    make_from_temporary_stack(const CommandTemporaryStack &p_temporary_stack) {
      transform = p_temporary_stack.transform;
      index_buffer = p_temporary_stack.index_buffer;
      vertex_buffer = p_temporary_stack.vertex_buffer;
    };
  };

  struct RenderPass {
    bgfx::FrameBufferHandle framebuffer;
    m::vec<ui16, 2> rect;
    m::vec<ui16, 4> scissor;
    m::mat<f32, 4, 4> view;
    m::mat<f32, 4, 4> proj;

    container::vector<uimax> commands;

    void allocate() { commands.allocate(0); };
    void free() { commands.free(); };

    static RenderPass get_default() {
      RenderPass l_render_pass;
      l_render_pass.allocate();
      l_render_pass.framebuffer.idx = 0;
      l_render_pass.rect.setZero();
      l_render_pass.scissor.setZero();
      l_render_pass.view.setZero();
      l_render_pass.proj.setZero();
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

    struct commands_memory_table {
      table_heap_paged_meta;
      table_heap_paged_cols_1(ui8);
      table_define_heap_paged_1;
    } commands_memory_table;

    // TODO -> move to table
    container::vector<RenderPass> renderpasses;

    heap() {
      renderpasses.allocate(0);
      renderpasses.push_back(
          RenderPass::get_default()); // at least one renderpass
      buffer_memory_table.allocate(4096 * 4096);
      buffers_table.allocate(1024);
      buffers_ptr_mapping_table.allocate(0);
      texture_table.allocate(0);
      framebuffer_table.allocate(0);
      vertexbuffer_table.allocate(0);
      indexbuffer_table.allocate(0);
      commands_memory_table.allocate(1024);
    };

    void free() {
      assert_debug(!vertexbuffer_table.has_allocated_elements());
      assert_debug(!indexbuffer_table.has_allocated_elements());
      assert_debug(renderpasses.count() == 1);
      for (auto l_render_pass_it = 0; l_render_pass_it < renderpasses.count();
           ++l_render_pass_it) {
        renderpasses.at(l_render_pass_it).free();
      }
      renderpasses.free();
      buffer_memory_table.free();
      buffers_table.free();
      buffers_ptr_mapping_table.free();
      texture_table.free();
      vertexbuffer_table.free();
      indexbuffer_table.free();
      commands_memory_table.free();
    };

    bgfx::Memory *allocate_buffer(uimax p_size) {
      auto l_buffer_index = buffer_memory_table.push_back(p_size);
      container::range<ui8> l_buffer_range;
      buffer_memory_table.at(l_buffer_index, &l_buffer_range);

      bgfx::Memory l_buffer{};
      l_buffer.data = (uint8_t *)l_buffer_range.m_begin;
      l_buffer.size = l_buffer_range.m_count;

      uimax l_index = buffers_table.push_back(1);
      bgfx::Memory *l_bgfx_memory;
      MemoryReference *l_memory_refence;
      uimax l_memory_count;
      buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence,
                       &l_memory_count);
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
      uimax l_memory_count;
      buffers_table.at(l_index, &l_bgfx_memory, &l_memory_refence,
                       &l_memory_count);
      assert_debug(l_memory_count == 1);
      *l_bgfx_memory = l_buffer;
      *l_memory_refence = MemoryReference(-1);

      buffers_ptr_mapping_table.push_back(l_bgfx_memory, l_index);
      return l_bgfx_memory;
    };

    void free_buffer(const bgfx::Memory *p_buffer) {

      for (auto i = 0; i < buffers_ptr_mapping_table.m_meta.m_count; ++i) {
        if (buffers_ptr_mapping_table.m_col_0[i] == p_buffer) {

          bgfx::Memory *l_tmp_buffer;
          MemoryReference *l_reference;
          uimax l_buffers_table_count;
          buffers_table.at(buffers_ptr_mapping_table.m_col_1[i], &l_tmp_buffer,
                           &l_reference, &l_buffers_table_count);
          assert_debug(l_buffers_table_count == 1);
          assert_debug(l_tmp_buffer == p_buffer);
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
      // texture_table.
      free_buffer(m_db.at<texture_table_idx, 0>(p_texture.idx).buffer);
      m_db.remove_at<texture_table_idx>(p_texture.idx);
    };

    bgfx::FrameBufferHandle
    allocate_frame_buffer(bgfx::TextureHandle p_texture) {
      FrameBuffer l_frame_buffer;
      l_frame_buffer.texture = p_texture;
      bgfx::FrameBufferHandle l_handle;
      l_handle.idx = m_db.push_back<framebuffer_table_idx>(l_frame_buffer);
      return l_handle;
    };

    void free_frame_buffer(bgfx::FrameBufferHandle p_frame_buffer) {
      auto &l_frame_buffer =
          m_db.at<framebuffer_table_idx, 0>(p_frame_buffer.idx);
      free_texture(l_frame_buffer.texture);
      m_db.remove_at<framebuffer_table_idx>(p_frame_buffer.idx);
    };

    bgfx::VertexBufferHandle
    allocate_vertex_buffer(const bgfx::Memory *p_memory,
                           const bgfx::VertexLayout &p_layout) {
      VertexBuffer l_vertex_buffer;
      l_vertex_buffer.layout = p_layout;
      l_vertex_buffer.memory = p_memory;
      bgfx::VertexBufferHandle l_handle;
      l_handle.idx = m_db.push_back<vertexbuffer_table_idx>(l_vertex_buffer);
      return l_handle;
    };

    void free_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
      auto &l_vertex_buffer = m_db.at<vertexbuffer_table_idx, 0>(p_handle.idx);
      free_buffer(l_vertex_buffer.memory);
      m_db.remove_at<vertexbuffer_table_idx>(p_handle.idx);
    };

    bgfx::IndexBufferHandle
    allocate_index_buffer(const bgfx::Memory *p_memory) {
      IndexBuffer l_index_buffer;
      l_index_buffer.memory = p_memory;
      bgfx::IndexBufferHandle l_handle;
      l_handle.idx = m_db.push_back<indexbuffer_table_idx>(l_index_buffer);
      return l_handle;
    };

    void free_index_buffer(bgfx::IndexBufferHandle p_handle) {
      auto &l_index_buffer = m_db.at<indexbuffer_table_idx, 0>(p_handle.idx);
      free_buffer(l_index_buffer.memory);
      m_db.remove_at<indexbuffer_table_idx>(p_handle.idx);
    };

  } heap;

  bgfx::TextureHandle allocate_texture(uint16_t p_width, uint16_t p_height,
                                       bool p_hasMips, uint16_t p_numLayers,
                                       bgfx::TextureFormat::Enum p_format,
                                       uint64_t p_flags) {
    bgfx::TextureInfo l_texture_info{};
    l_texture_info.format = p_format;
    l_texture_info.width = p_width;
    l_texture_info.height = p_height;
    l_texture_info.bitsPerPixel =
        get_pixel_size_from_texture_format(p_format).value;
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
    auto &l_render_pass = heap.renderpasses.at(p_id);
    l_render_pass.rect = {p_x, p_y};
  };

  void view_set_framebuffer(bgfx::ViewId p_id,
                            bgfx::FrameBufferHandle p_handle) {
    auto &l_render_pass = heap.renderpasses.at(p_id);
    l_render_pass.framebuffer = p_handle;
  };

  void view_set_transform(bgfx::ViewId p_id, const m::mat<f32, 4, 4> &p_view,
                          const m::mat<f32, 4, 4> &p_proj) {
    auto &l_render_pass = heap.renderpasses.at(p_id);
    l_render_pass.view = p_view;
    l_render_pass.proj = p_proj;
  };

  void view_submit(bgfx::ViewId p_id) {
    auto &l_render_pass = heap.renderpasses.at(p_id);
    Command_DrawCall l_draw_call;
    l_draw_call.make_from_temporary_stack(command_temporary_stack);
    command_temporary_stack.clear();

    auto l_command_index = heap.m_db.push_back<heap::commands_memory_table_idx>(
        sizeof(l_draw_call));
    container::range<ui8_t> l_command =
        heap.m_db.range<heap::commands_memory_table_idx, 0>(l_command_index);
    l_command.copy_from(container::range<ui8_t>::make((ui8_t *)&l_draw_call,
                                                      sizeof(l_draw_call)));
    l_render_pass.commands.push_back(l_command_index);
  };

  void set_transform(const mat<f32, 4, 4> &p_transform) {
    command_temporary_stack.transform = p_transform;
  };

  void set_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
    command_temporary_stack.vertex_buffer = p_handle;
  };

  void set_index_buffer(bgfx::IndexBufferHandle p_handle) {
    command_temporary_stack.index_buffer = p_handle;
  };

  void frame() {

    // TOOD -> do stuffs

    for (auto l_it = 0; l_it < heap.renderpasses.count(); ++l_it) {
      RenderPass &l_render_pass = heap.renderpasses.at(l_it);
      for (auto l_command_it = 0; l_command_it < l_render_pass.commands.count();
           ++l_command_it) {
        heap.m_db.remove_at<heap::commands_memory_table_idx>(
            l_render_pass.commands.at(l_command_it));
      }
      l_render_pass.commands.clear();
    }
  };

  void initialize() { command_temporary_stack.clear(); };
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
      return ui8(23);
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
  BOOST_ASSERT(!has(_attrib));
  m_attributes[_attrib] = _attrib;
  m_offset[_attrib] = m_stride;
  m_stride += bgfx_impl::AttribType::get_size(_type) * _num;

  ui8_t l_byte_offset = (9 * _attrib);
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
  bgfx_impl.view_set_transform(_id, *(const mat<f32, 4, 4> *)_view,
                               *(const mat<f32, 4, 4> *)_proj);
};

inline uint32_t setTransform(const void *_mtx, uint16_t _num) {
  bgfx_impl.set_transform(*(const mat<f32, 4, 4> *)_mtx);
  return -1;
};

inline void setVertexBuffer(uint8_t _stream, VertexBufferHandle _handle) {
  bgfx_impl.set_vertex_buffer(_handle);
};

inline void setIndexBuffer(IndexBufferHandle _handle) {
  bgfx_impl.set_index_buffer(_handle);
};

inline void touch(ViewId _id) { bgfx_impl.view_submit(_id); };

inline void submit(ViewId _id, ProgramHandle _program, uint32_t _depth,
                   uint8_t _flags) {
  bgfx_impl.view_submit(_id);
};

inline uint32_t frame(bool _capture) {
  bgfx_impl.frame();
  return 0;
};

}; // namespace bgfx