#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <cor/types.hpp>
#include <m/color.hpp>
#include <m/mat.hpp>
#include <m/vec.hpp>
#include <rast/model.hpp>

template <typename Private> struct rast_api {
  Private &thiz;
  rast_api(Private &p_thiz) : thiz(p_thiz){};

  FORCE_INLINE ui8 init(const bgfx::Init &p_init = {}) {
    return rast_api_init(&thiz, p_init);
  };

  FORCE_INLINE void shutdown() { rast_api_shutdown(&thiz); };

  FORCE_INLINE const bgfx::Memory *alloc(uint32_t _size) {
    return rast_api_alloc(&thiz, _size);
  };

  FORCE_INLINE const bgfx::Memory *alloc(uint32_t _size, uint32_t _alignment) {
    return rast_api_alloc(&thiz, _size, _alignment);
  };

  FORCE_INLINE const bgfx::Memory *makeRef(const void *_data, uint32_t _size,
                                           bgfx::ReleaseFn _releaseFn = NULL,
                                           void *_userData = NULL) {
    return rast_api_makeRef(&thiz, _data, _size, _releaseFn, _userData);
  };

  FORCE_INLINE bgfx::TextureHandle
  createTexture2D(uint16_t _width, uint16_t _height, bool _hasMips,
                  uint16_t _numLayers, bgfx::TextureFormat::Enum _format,
                  uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
                  const bgfx::Memory *_mem = NULL) {
    return rast_api_createTexture2D(&thiz, _width, _height, _hasMips,
                                    _numLayers, _format, _flags, _mem);
  };

  FORCE_INLINE bgfx::FrameBufferHandle createFrameBuffer(
      uint16_t _width, uint16_t _height, bgfx::TextureFormat::Enum _format,
      uint64_t _textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP) {
    return rast_api_createFrameBuffer(&thiz, _width, _height, _format,
                                      _textureFlags);
  };

  FORCE_INLINE bgfx::FrameBufferHandle createFrameBuffer(
      void *_nwh, uint16_t _width, uint16_t _height,
      bgfx::TextureFormat::Enum _format = bgfx::TextureFormat::Count,
      bgfx::TextureFormat::Enum _depthFormat = bgfx::TextureFormat::Count) {
    return rast_api_createFrameBuffer(&thiz, _nwh, _width, _height, _format,
                                      _depthFormat);
  };

  FORCE_INLINE void destroy(bgfx::FrameBufferHandle _handle) {
    rast_api_destroy(&thiz, _handle);
  };

  FORCE_INLINE bgfx::TextureHandle getTexture(bgfx::FrameBufferHandle _handle,
                                              uint8_t _attachment = 0) {
    return rast_api_getTexture(&thiz, _handle, _attachment);
  };

  FORCE_INLINE container::range<ui8>
  fetchTextureSync(bgfx::TextureHandle _texture) {
    return rast_api_fetchTextureSync(&thiz, _texture);
  };

  FORCE_INLINE bgfx::VertexBufferHandle
  createVertexBuffer(const bgfx::Memory *_mem,
                     const bgfx::VertexLayout &_layout,
                     uint16_t _flags = BGFX_BUFFER_NONE) {
    return rast_api_createVertexBuffer(&thiz, _mem, _layout, _flags);
  };

  FORCE_INLINE void destroy(bgfx::VertexBufferHandle _handle) {
    rast_api_destroy(&thiz, _handle);
  };

  FORCE_INLINE bgfx::IndexBufferHandle
  createIndexBuffer(const bgfx::Memory *_mem,
                    uint16_t _flags = BGFX_BUFFER_NONE) {
    return rast_api_createIndexBuffer(&thiz, _mem, _flags);
  };

  FORCE_INLINE void destroy(bgfx::IndexBufferHandle _handle) {
    rast_api_destroy(&thiz, _handle);
  };

  FORCE_INLINE bgfx::ShaderHandle createShader(const bgfx::Memory *_mem) {
    return rast_api_createShader(&thiz, _mem);
  };

  FORCE_INLINE void destroy(bgfx::ShaderHandle _handle) {
    rast_api_destroy(&thiz, _handle);
  };

  FORCE_INLINE bgfx::ProgramHandle createProgram(bgfx::ShaderHandle _vsh,
                                                 bgfx::ShaderHandle _fsh,
                                                 bool _destroyShaders = false) {
    return rast_api_createProgram(&thiz, _vsh, _fsh, _destroyShaders);
  };

  FORCE_INLINE void destroy(bgfx::ProgramHandle _handle) {
    rast_api_destroy(&thiz, _handle);
  };

  FORCE_INLINE bgfx::UniformHandle createUniform(const char *_name,
                                                 bgfx::UniformType::Enum _type,
                                                 uint16_t _num = 1) {
    rast_api_createUniform(&thiz, _name, _type, _num);
  };

  FORCE_INLINE void destroy(bgfx::UniformHandle p_uniform) {
    rast_api_destroy(&thiz, p_uniform);
  };

  FORCE_INLINE void setViewRect(bgfx::ViewId _id, uint16_t _x, uint16_t _y,
                                uint16_t _width, uint16_t _height) {
    rast_api_setViewRect(&thiz, _id, _x, _y, _width, _height);
  };

  FORCE_INLINE void setViewFrameBuffer(bgfx::ViewId _id,
                                       bgfx::FrameBufferHandle _handle) {
    rast_api_setViewFrameBuffer(&thiz, _id, _handle);
  };

  FORCE_INLINE void setViewClear(bgfx::ViewId _id, uint16_t _flags,
                                 uint32_t _rgba = 0x000000ff,
                                 float _depth = 1.0f, uint8_t _stencil = 0) {
    rast_api_setViewClear(&thiz, _id, _flags, _rgba, _depth);
  };

  FORCE_INLINE void setViewTransform(bgfx::ViewId _id, const void *_view,
                                     const void *_proj) {
    rast_api_setViewTransform(&thiz, _id, _view, _proj);
  };

  FORCE_INLINE uint32_t setTransform(const void *_mtx, uint16_t _num = 1) {
    return rast_api_setTransform(&thiz, _mtx, _num);
  };

  FORCE_INLINE void setVertexBuffer(uint8_t _stream,
                                    bgfx::VertexBufferHandle _handle) {
    rast_api_setVertexBuffer(&thiz, _stream, _handle);
  };

  FORCE_INLINE void setIndexBuffer(bgfx::IndexBufferHandle _handle) {
    rast_api_setIndexBuffer(&thiz, _handle);
  };

  FORCE_INLINE void setState(uint64_t _state, uint32_t _rgba = 0) {
    rast_api_setState(&thiz, _state, _rgba);
  };

  FORCE_INLINE void touch(bgfx::ViewId _id){/* bgfx_impl.view_submit(_id); */};

  FORCE_INLINE void submit(bgfx::ViewId _id, bgfx::ProgramHandle _program,
                           uint32_t _depth = 0,
                           uint8_t _flags = BGFX_DISCARD_ALL) {
    rast_api_submit(&thiz, _id, _program, _depth, _flags);
  };

  FORCE_INLINE uint32_t frame(bool _capture = false) {
    return rast_api_frame(&thiz, _capture);
  };
};

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

inline void VertexLayout::end() {
  // Add additional size to make the memory a power of two
  m_stride += (m_stride % 2);
};

inline VertexLayout &VertexLayout::add(Attrib::Enum _attrib, uint8_t _num,
                                       AttribType::Enum _type, bool _normalized,
                                       bool _asInt) {
  assert_debug(!has(_attrib));
  m_attributes[_attrib] = _attrib;
  m_offset[_attrib] = m_stride;
  m_stride += rast::attrib_type::get_size(_type) * _num;
  return *this;
};

inline void VertexLayout::decode(Attrib::Enum _attrib, uint8_t &_num,
                                 AttribType::Enum &_type, bool &_normalized,
                                 bool &_asInt) const {
  switch (_attrib) {
  case Attrib::Enum::Position:
    _num = 3;
    _type = AttribType::Enum::Float;
    break;
  case Attrib::Enum::Color0:
    _num = 4;
    _type = AttribType::Enum::Uint8;
    break;
  default:
    _num = 0;
    _type = AttribType::Enum::Count;
  }

  _normalized = 0;
  _asInt = 0;
};
}; // namespace bgfx
