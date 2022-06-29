#pragma once

#include <eng/engine.hpp>

#ifndef TEST_RESOURCE_PATH_RAW
#define TEST_RESOURCE_PATH_RAW                                                 \
  "/media/loic/SSD/SoftwareProjects/Once/test_data/"
#endif

inline static constexpr auto TEST_RESOURCE_PATH =
    container::arr_literal<ui8>(TEST_RESOURCE_PATH_RAW);

struct TestUtils {
  static i32 write_png(const ui8 *filename, i32 x, i32 y, i32 comp,
                       const void *data, i32 stride_bytes);
  static container::range<ui8>
  write_png_to_mem(const ui8 *pixels, i32 stride_bytes, i32 x, i32 y, i32 n);
  static void write_free(ui8 *p_ptr);
  static ui8 *read_png(const ui8 *filename, i32 *x, i32 *y, i32 *comp,
                       i32 req_comp);
  static void read_free(ui8 *ptr);
};