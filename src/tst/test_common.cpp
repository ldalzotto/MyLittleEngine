#include <tst/test_common.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

i32 TestUtils::write_png(const ui8 *filename, i32 x, i32 y, i32 comp,
                         const void *data, i32 stride_bytes) {
  return stbi_write_png((const char *)filename, x, y, comp, data, stride_bytes);
};

container::range<ui8> TestUtils::write_png_to_mem(const ui8 *pixels,
                                                  i32 stride_bytes, i32 x,
                                                  i32 y, i32 n) {
  container::range<ui8> l_png_frame;
  i32 l_length;
  l_png_frame.m_begin = (ui8 *)stbi_write_png_to_mem(
      (const unsigned char *)pixels, stride_bytes, x, y, n, &l_length);
  l_png_frame.m_count = l_length;
  return l_png_frame;
};

void TestUtils::write_free(ui8 *p_ptr) { STBIW_FREE(p_ptr); };

ui8 *TestUtils::read_png(const ui8 *filename, i32 *x, i32 *y, i32 *comp,
                         i32 req_comp) {
  return stbi_load((const char *)filename, x, y, comp, req_comp);
};

void TestUtils::read_free(ui8 *ptr) { stbi_image_free(ptr); };