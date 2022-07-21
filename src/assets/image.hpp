#pragma once

#include <cor/types.hpp>

namespace assets {
struct image {
  ui16 m_width;
  ui16 m_height;
  ui8 m_bits_per_pixel;
  ui8 *m_data;
};
}; // namespace assets