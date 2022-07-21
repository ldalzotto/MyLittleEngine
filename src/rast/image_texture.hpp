#pragma once

#include <cor/container.hpp>
#include <shared/types.hpp>

namespace rast {

// TODO -> improve that
inline static void image_copy_stretch(rgb_t *p_from, ui16 p_from_width,
                                      ui16 p_from_height, rgba_t *p_to,
                                      ui16 p_to_width, ui16 p_to_height) {
  fix32 l_width_delta_ratio = fix32(p_from_width) / p_to_width;
  fix32 l_height_delta_ratio = fix32(p_from_height) / p_to_height;
  for (auto y = 0; y < p_to_height; ++y) {
    ui16 l_from_y = ui16(l_height_delta_ratio * y);
    for (auto x = 0; x < p_to_width; ++x) {
      ui16 l_from_x = ui16(l_width_delta_ratio * x);
      *(rgb_t *)&p_to[x + (y * p_to_width)] =
          p_from[l_from_x + (l_from_y * p_from_height)];
    }
  }
};

struct image {
  ui16 m_width;
  ui16 m_height;
  ui8 m_bits_per_pixel;
  container::range<ui8> m_buffer;

  // FIXME -> container::range<ui8> m_buffer; should be ui8* m_buffer; instead
  image(const ui16 &p_width, const ui16 &p_height, const ui8 &p_bits_per_pixel,
        const container::range<ui8> &p_buffer)
      : m_width(p_width), m_height(p_height),
        m_bits_per_pixel(p_bits_per_pixel), m_buffer(p_buffer){};

  uimax stride() const { return m_bits_per_pixel * m_width; };

  template <typename SizeType> uimax get_buffer_index(SizeType r, SizeType c) {

    assert_debug(r < m_height);
    assert_debug(c < m_width);

    return (r * stride()) + (c * m_bits_per_pixel);
  };

  template <typename SizeType> uimax get_buffer_index(SizeType p) {

    assert_debug(p < (m_height * m_width));

    return m_bits_per_pixel * p;
  };

  template <typename SizeType> ui8 *at(SizeType r, SizeType c) {
    return &m_buffer.at(get_buffer_index(r, c));
  };
  template <typename SizeType> ui8 *at(SizeType p) {
    return &m_buffer.at(get_buffer_index(p));
  };

  template <typename SizeType>
  void set_pixel(SizeType r, SizeType c, const rgb_t &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_bits_per_pixel);
    *(rgb_t *)at(r, c) = p_pixel;
  };

  template <typename SizeType>
  void set_pixel(SizeType p, const rgb_t &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_bits_per_pixel);
    *(rgb_t *)at(p) = p_pixel;
  };

  template <typename SizeType> rgb_t &get_pixel(SizeType x, SizeType y) {
    return *(rgb_t *)at(get_buffer_index(x, y));
  };

  uimax size_of() { return stride() * m_height; };
  uimax pixel_count() { return m_height * m_width; };

  template <typename T, typename CallbackFunc>
  void for_each(const CallbackFunc &p_callback) {
    assert_debug(m_bits_per_pixel >= sizeof(T));
    auto l_pix_count = pixel_count();
    for (auto i = 0; i < l_pix_count; ++i) {
      p_callback(*(T *)at(i));
    }
  };

  void copy_to(const image &p_other) { m_buffer.copy_to(p_other.m_buffer); };
};

}; // namespace rast