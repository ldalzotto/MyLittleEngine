#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <m/vec.hpp>

namespace rast {

struct image_view {

  const bgfx::TextureInfo &m_target_info;
  container::range<ui8> &m_buffer;

  image_view(const bgfx::TextureInfo &p_target_info,
             container::range<ui8> &p_buffer)
      : m_target_info(p_target_info), m_buffer(p_buffer) {
    m_buffer = p_buffer;

    assert_debug(m_buffer.count() ==
                 p_target_info.bitsPerPixel *
                     (p_target_info.height * p_target_info.width));
  };

  uimax get_buffer_index(ui16 r, ui16 c) {

    assert_debug(r < m_target_info.height);
    assert_debug(c < m_target_info.width);

    return r * stride() + (c * m_target_info.bitsPerPixel);
  };
  uimax get_buffer_index(ui16 p) {

    assert_debug(p < (m_target_info.height * m_target_info.width));

    return m_target_info.bitsPerPixel * p;
  };

  ui8 *at(ui16 r, ui16 c) { return &m_buffer.at(get_buffer_index(r, c)); };
  ui8 *at(ui16 p) { return &m_buffer.at(get_buffer_index(p)); };

  void set_pixel(ui16 r, ui16 c, const m::vec<ui8, 3> &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_target_info.bitsPerPixel);
    *(m::vec<ui8, 3> *)at(r, c) = p_pixel;
  };

  void set_pixel(ui16 p, const m::vec<ui8, 3> &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_target_info.bitsPerPixel);
    *(m::vec<ui8, 3> *)at(p) = p_pixel;
  };

  uimax get_image_byte_size() { return stride() * m_target_info.height; };
  uimax pixel_count() { return m_target_info.height * m_target_info.width; };
  uimax stride() { return m_target_info.bitsPerPixel * m_target_info.width; };

  template <typename Callback> void for_each_pixels_rgb(const Callback &p_cb) {
    assert_debug(m_target_info.bitsPerPixel == sizeof(ui8) * 3);
    auto l_pix_count = pixel_count();
    for (auto i = 0; i < l_pix_count; ++i) {
      p_cb(*(m::vec<ui8, 3> *)at(i));
    }
  };
};

} // namespace rast