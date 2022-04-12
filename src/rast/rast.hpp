#pragma once

#include <bgfx/bgfx.h>
#include <boost/container/list.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <cor/cor.hpp>

namespace boost {
template <typename T> struct object_pool_indexed {
private:
  boost::container::vector<T> data;
  boost::container::vector<i8> is_allocated;

public:
  object_pool_indexed() {
    data = boost::container::vector<T>(0);
    is_allocated = boost::container::vector<i8>(0);
  };
  object_pool_indexed(uimax p_capacity) {
    data.reserve(p_capacity);
    is_allocated.reserve(p_capacity.value);
  };

public:
  uimax malloc(const T &p_element) {
    auto l_index = uimax(-1);
    for (auto i = uimax(0); i.value < is_allocated.size(); ++i) {
      if (is_allocated.at(i.value).value) {
        l_index = i;
        break;
      }
    }

    if (l_index != uimax(-1)) {
      data.at(l_index.value) = p_element;
      is_allocated.at(l_index.value) = i8(1);
      return l_index;
    } else {
      data.push_back(p_element);
      is_allocated.push_back(i8(1));
      return uimax(data.size() - 1);
    }
    //       boost::range::fin
  };

  void free(uimax p_index) {
    BOOST_ASSERT(is_allocated.at(p_index.value).value);
    is_allocated.at(p_index.value) = i8(0);
  };

  T &at(uimax p_index) {
    BOOST_ASSERT(is_allocated.at(p_index.value).value);
    return data.at(p_index.value);
  };

private:
};
}; // namespace boost

struct bgfx_impl {

  struct heap {

    struct Texture {
      bgfx::TextureInfo info;
      bgfx::Memory *buffer;
    };

    struct FrameBuffer {
      bgfx::TextureHandle texture;
    };

    struct RenderPass {
      bgfx::FrameBufferHandle framebuffer;
      vec<ui16, 2> rect;
      vec<ui16, 4> scissor;

      static RenderPass get_default() {
        RenderPass l_render_pass;
        l_render_pass.framebuffer.idx = 0;
        l_render_pass.rect[0] = ui16(0);
        l_render_pass.rect[1] = ui16(0);
        l_render_pass.scissor[0] = ui16(0);
        l_render_pass.scissor[1] = ui16(0);
        l_render_pass.scissor[2] = ui16(0);
        l_render_pass.scissor[3] = ui16(0);
        return l_render_pass;
      };
    };

    boost::pool<> memory;
    boost::object_pool<bgfx::Memory> buffers;
    boost::object_pool_indexed<Texture> textures;
    boost::object_pool_indexed<FrameBuffer> framebuffers;
    boost::object_pool_indexed<RenderPass> renderpasses;

    heap() : memory(1000) {
      renderpasses.malloc(RenderPass::get_default()); // at least one renderpass
    };

    bgfx::Memory *allocate_buffer(uimax p_size) {
      memory.set_next_size(p_size.value);
      bgfx::Memory *l_buffer = buffers.malloc();
      l_buffer->data = (uint8_t *)memory.malloc();
      l_buffer->size = uint32_t(p_size.value);
      return l_buffer;
    };

    void free_buffer(bgfx::Memory *p_buffer) {
      memory.free(p_buffer->data);
      buffers.free(p_buffer);
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
    auto &l_render_pass = heap.renderpasses.at(uimax(p_id));
    l_render_pass.rect = {p_x, p_y};
  };

  void view_set_framebuffer(bgfx::ViewId p_id,
                            bgfx::FrameBufferHandle p_handle) {
    auto &l_render_pass = heap.renderpasses.at(uimax(p_id));
    l_render_pass.framebuffer = p_handle;
  };

  void initialize(){

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
inline bool init(const bgfx::Init &p_init) { return 0; };

inline const Memory *alloc(uint32_t _size) {
  return bgfx_impl.heap.allocate_buffer(uimax(_size));
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

inline void setViewRect(ViewId _id, uint16_t _x, uint16_t _y, uint16_t _width,
                        uint16_t _height) {
  bgfx_impl.view_set_rect(_id, _x, _y, _width, _height);
};

inline void setViewFrameBuffer(ViewId _id, FrameBufferHandle _handle) {
  bgfx_impl.view_set_framebuffer(_id, _handle);
};

}; // namespace bgfx