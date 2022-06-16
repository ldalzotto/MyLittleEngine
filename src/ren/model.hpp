#pragma once

#include <m/mat.hpp>

namespace ren {

struct camera_handle {
  uimax m_idx;
};

struct mesh_handle {
  uimax m_idx;
};

struct shader_handle {
  uimax m_idx;
};

struct camera {
  ui32 m_rendertexture_width;
  ui32 m_rendertexture_height;

  ui32 m_width;
  ui32 m_height;

  m::mat<fix32, 4, 4> m_view;
  m::mat<fix32, 4, 4> m_projection;

  camera() = default;
};

struct shader {};

struct mesh {};

}; // namespace ren