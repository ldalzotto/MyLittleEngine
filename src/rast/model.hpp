#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <m/mat.hpp>
#include <m/rect.hpp>

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

struct shader_vertex_runtime_ctx {
  m::rect_point_extend<ui16> &m_rect;
  const m::mat<f32, 4, 4> &m_proj;
  const m::mat<f32, 4, 4> &m_view;
  const m::mat<f32, 4, 4> &m_transform;
  const m::mat<f32, 4, 4> &m_local_to_unit;
  const bgfx::VertexLayout &m_vertex_layout;

  shader_vertex_runtime_ctx(m::rect_point_extend<ui16> &p_rect,
                            const m::mat<f32, 4, 4> &p_proj,
                            const m::mat<f32, 4, 4> &p_view,
                            const m::mat<f32, 4, 4> &p_transform,
                            const m::mat<f32, 4, 4> &p_local_to_unit,
                            const bgfx::VertexLayout &p_vertex_layout)
      : m_rect(p_rect), m_proj(p_proj), m_view(p_view),
        m_transform(p_transform), m_local_to_unit(p_local_to_unit),
        m_vertex_layout(p_vertex_layout){};
};

using shader_vertex_function = void (*)(const shader_vertex_runtime_ctx &p_ctx,
                                        const ui8 *p_vertex,
                                        m::vec<f32, 4> &out_screen_position);

struct shader_utils {
  static const m::vec<f32, 3> &
  get_vertex_vec3f32(const shader_vertex_runtime_ctx &p_ctx, ui8 p_attrib_index,
                     const ui8 *p_vertex) {
    return *(
        const m::vec<f32, 3> *)(p_vertex +
                                p_ctx.m_vertex_layout.m_offset[p_attrib_index]);
  };
};

} // namespace rast