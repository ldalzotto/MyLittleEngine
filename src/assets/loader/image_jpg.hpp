#pragma once

#include <assets/image.hpp>
#include <cor/container.hpp>

namespace assets {

namespace details {
extern ui8 *read(const container::range<ui8> &p_data, i32 *out_x, i32 *out_y,
                 i32 *out_channel);
} // namespace details

struct jpg_image_loader {
  image compile(const container::range<ui8> &p_raw_jpg) {
    return __compile(p_raw_jpg);
  };

private:
  image __compile(const container::range<ui8> &p_raw_jpg) {
    image l_image;
    i32 x, y, b;
    l_image.m_data = details::read(p_raw_jpg, &x, &y, &b);
    l_image.m_width = x;
    l_image.m_height = y;
    l_image.m_bits_per_pixel = b;
    assert_debug(l_image.m_data);
    assert_debug(l_image.m_width > 0);
    assert_debug(l_image.m_height > 0);
    assert_debug(l_image.m_bits_per_pixel > 0);
    return l_image;
  };
};

} // namespace assets