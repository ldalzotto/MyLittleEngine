#pragma once

#include <bgfx/bgfx.h>
#include <boost/container/list.hpp>
#include <boost/container/map.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <cor/container.hpp>
#include <cor/cor.hpp>

struct bgfx_impl {

  struct MemoryReference {};

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
    mat<f32, 4, 4> transform;
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
    mat<f32, 4, 4> transform;
    bgfx::IndexBufferHandle index_buffer;
    bgfx::VertexBufferHandle vertex_buffer;

    void
    make_from_temporary_stack(const CommandTemporaryStack &p_temporary_stack) {
      transform = p_temporary_stack.transform;
      index_buffer = p_temporary_stack.index_buffer;
      vertex_buffer = p_temporary_stack.vertex_buffer;
    };
  };

  using CommandsMemoryType = boost::pool<>;

  struct RenderPass {
    bgfx::FrameBufferHandle framebuffer;
    vec<ui16, 2> rect;
    vec<ui16, 4> scissor;
    mat<f32, 4, 4> view;
    mat<f32, 4, 4> proj;

    container::vector<CommandHeader *> commands;

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

    template <typename CommandType>
    void push_command(const CommandType &p_command,
                      CommandsMemoryType &p_commands_memory) {
      p_commands_memory.set_next_size(sizeof(p_command));
      CommandType *l_buffer = (CommandType *)p_commands_memory.malloc();
      *l_buffer = p_command;
      commands.push_back(&l_buffer->header);
    };

    void clear_commands(CommandsMemoryType &p_commands_memory) {
      for (auto l_it = commands.iter(0); l_it; ++l_it) {
        p_commands_memory.free(*l_it);
      }
      commands.clear();
    };
  };

  struct heap {

    boost::pool<> buffers_memory;
    boost::object_pool<bgfx::Memory> buffers;
    boost::container::map<bgfx::Memory *, MemoryReference> buffer_is_reference;
    container::object_pool_indexed<Texture> textures;
    container::object_pool_indexed<FrameBuffer> framebuffers;
    container::object_pool_indexed<VertexBuffer> vertexbuffers;
    container::object_pool_indexed<IndexBuffer> indexbuffers;
    CommandsMemoryType commands_memory;
    container::vector<RenderPass> renderpasses;

    heap() : buffers_memory(1000), commands_memory(1000) {
      renderpasses.allocate(0);
      renderpasses.push_back(
          RenderPass::get_default()); // at least one renderpass
      textures.allocate(0);
      framebuffers.allocate(0);
      vertexbuffers.allocate(0);
      indexbuffers.allocate(0);
    };

    void free() {
      assert_debug(vertexbuffers.free_elements_size() == 0);
      assert_debug(indexbuffers.free_elements_size() == 0);
      assert_debug(renderpasses.count() == 1);
      for (auto l_render_pass = renderpasses.iter(0); l_render_pass;
           ++l_render_pass) {
        (*l_render_pass).free();
      }
      renderpasses.free();
      framebuffers.free();
      vertexbuffers.free();
      indexbuffers.free();
      textures.free();
    };

    bgfx::Memory *allocate_buffer(uimax p_size) {
      buffers_memory.set_next_size(p_size.value);
      bgfx::Memory *l_buffer = buffers.malloc();
      l_buffer->data = (uint8_t *)buffers_memory.malloc();
      l_buffer->size = uint32_t(p_size.value);
      return l_buffer;
    };

    bgfx::Memory *allocate_ref(const void *p_ptr, ui32_t p_size) {
      bgfx::Memory *l_buffer = buffers.malloc();
      l_buffer->data = (ui8_t *)p_ptr;
      l_buffer->size = p_size;
      buffer_is_reference.emplace(l_buffer, MemoryReference());
      return l_buffer;
    };

    void free_buffer(const bgfx::Memory *p_buffer) {
      if (!buffer_is_reference.contains((bgfx::Memory *)p_buffer)) {
        buffers_memory.free(p_buffer->data);
      }
      buffers.free((bgfx::Memory *)p_buffer);
    };

    bgfx::TextureHandle
    allocate_texture(const bgfx::TextureInfo &p_texture_info) {
      uimax l_image_size = uimax(p_texture_info.bitsPerPixel *
                                 p_texture_info.width * p_texture_info.height);
      Texture l_texture;
      l_texture.info = p_texture_info;
      l_texture.buffer = allocate_buffer(l_image_size);
      bgfx::TextureHandle l_texture_handle;
      l_texture_handle.idx = uint16_t(textures.malloc(l_texture).value);

      return l_texture_handle;
    };

    void free_texture(bgfx::TextureHandle p_texture) {
      free_buffer(textures.at(uimax(p_texture.idx)).buffer);
      textures.free(uimax(p_texture.idx));
    };

    bgfx::FrameBufferHandle
    allocate_frame_buffer(bgfx::TextureHandle p_texture) {
      FrameBuffer l_frame_buffer;
      l_frame_buffer.texture = p_texture;
      bgfx::FrameBufferHandle l_handle;
      l_handle.idx = framebuffers.malloc(l_frame_buffer).value;
      return l_handle;
    };

    void free_frame_buffer(bgfx::FrameBufferHandle p_frame_buffer) {
      auto &l_frame_buffer = framebuffers.at(uimax(p_frame_buffer.idx));
      free_texture(l_frame_buffer.texture);
      framebuffers.free(uimax(p_frame_buffer.idx));
    };

    bgfx::VertexBufferHandle
    allocate_vertex_buffer(const bgfx::Memory *p_memory,
                           const bgfx::VertexLayout &p_layout) {
      VertexBuffer l_vertex_buffer;
      l_vertex_buffer.layout = p_layout;
      l_vertex_buffer.memory = p_memory;
      bgfx::VertexBufferHandle l_handle;
      l_handle.idx = vertexbuffers.malloc(l_vertex_buffer).value;
      return l_handle;
    };

    void free_vertex_buffer(bgfx::VertexBufferHandle p_handle) {
      auto &l_vertex_buffer = vertexbuffers.at(uimax(p_handle.idx));
      free_buffer(l_vertex_buffer.memory);
    };

    bgfx::IndexBufferHandle
    allocate_index_buffer(const bgfx::Memory *p_memory) {
      IndexBuffer l_index_buffer;
      l_index_buffer.memory = p_memory;
      bgfx::IndexBufferHandle l_handle;
      l_handle.idx = indexbuffers.malloc(l_index_buffer).value;
      return l_handle;
    };

    void free_index_buffer(bgfx::IndexBufferHandle p_handle) {
      auto &l_index_buffer = indexbuffers.at(uimax(p_handle.idx));
      free_buffer(l_index_buffer.memory);
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

  void view_set_transform(bgfx::ViewId p_id, const mat<f32, 4, 4> &p_view,
                          const mat<f32, 4, 4> &p_proj) {
    auto &l_render_pass = heap.renderpasses.at(p_id);
    l_render_pass.view = p_view;
    l_render_pass.proj = p_proj;
  };

  void view_submit(bgfx::ViewId p_id) {
    auto &l_render_pass = heap.renderpasses.at(p_id);
    Command_DrawCall l_draw_call;
    l_draw_call.make_from_temporary_stack(command_temporary_stack);
    command_temporary_stack.clear();
    l_render_pass.push_command(l_draw_call, heap.commands_memory);
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

    for (auto l_it = heap.renderpasses.iter(0); l_it; ++l_it) {
      RenderPass &l_render_pass = *l_it;
      l_render_pass.clear_commands(heap.commands_memory);
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