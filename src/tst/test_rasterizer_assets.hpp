#pragma once

#include <cor/container.hpp>

// rast.single_triangle.visibility

struct frame_expected {
  static container::arr<ui8, 90> rast_single_triangle_visibility();
  static container::arr<ui8, 107>
  rast_single_triangle_vertex_color_interpolation();
  static container::arr<ui8, 92> rast_depth_comparison();
  static container::arr<ui8, 8752> rast_depth_comparison_large_framebuffer();
  static container::arr<ui8, 90> rast_depth_comparison_readonly();
  static container::arr<ui8, 114> rast_depth_comparison_outofbounds();
  static container::arr<ui8, 5106> rast_3Dcube();
};