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

struct shader {};

struct mesh {};

}; // namespace ren