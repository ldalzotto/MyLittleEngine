#include <assets/loader/image_jpg.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace assets {

namespace details {
inline ui8* read(const container::range<ui8> &p_data, i32 *out_x, i32 *out_y,
                 i32 *out_channel) {
  return stbi_load_from_memory(p_data.data(), p_data.count(), out_x, out_y,
                        out_channel, 0);
}
} // namespace details

} // namespace assets