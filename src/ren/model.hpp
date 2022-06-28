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

struct shader_meta {
  enum class cull_mode { clockwise, cclockwise } m_cull_mode;
  ui8 m_write_depth;
  enum class depth_test { none, less } m_depth_test;

  inline static shader_meta get_default() {
    return {.m_cull_mode = cull_mode::clockwise,
            .m_write_depth = 1,
            .m_depth_test = depth_test::less};
  };
};

struct mesh {};

}; // namespace ren