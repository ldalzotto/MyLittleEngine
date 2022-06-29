#pragma once

#include <eng/engine.hpp>

struct TestUtils {
  static i32 write_png(const ui8 *filename, i32 x, i32 y, i32 comp, const void *data,
                i32 stride_bytes);
  static  container::range<ui8> write_png_to_mem(const ui8 *pixels, i32 stride_bytes,
                                         i32 x, i32 y, i32 n);
  static void write_free(ui8 *p_ptr);
};