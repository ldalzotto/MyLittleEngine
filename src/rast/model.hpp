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

struct shader_vertex_meta {
  struct header {
    uimax m_output_offset;
    uimax m_output_count;
    uimax m_output_byte_size;
  };

  struct output_parameter {
    bgfx::AttribType::Enum m_attrib_type;
    ui8 m_attrib_element_count;
    ui16 m_single_element_size;

    output_parameter() = default;
    output_parameter(bgfx::AttribType::Enum p_attrib_type,
                     ui8 p_attrib_element_count) {
      m_attrib_type = p_attrib_type;
      m_attrib_element_count = p_attrib_element_count;
      m_single_element_size = 0;
      switch (p_attrib_type) {
      case bgfx::AttribType::Float:
        m_single_element_size = sizeof(f32);
        break;
      default:
        m_single_element_size = 0;
        break;
      }
      m_single_element_size *= p_attrib_element_count;
    };
  };

  static uimax
  size_in_bytes(const container::range<output_parameter> &p_output_parameters) {
    return sizeof(header) +
           +(p_output_parameters.count() * sizeof(output_parameter));
  };

  struct view {
    ui8 *m_buffer;

    view(ui8 *p_buffer) : m_buffer(p_buffer){};

    void
    initialize(const container::range<output_parameter> &p_output_parameters) {
      header l_header;
      l_header.m_output_offset = sizeof(l_header);
      l_header.m_output_count = p_output_parameters.count();
      l_header.m_output_byte_size =
          l_header.m_output_count * sizeof(output_parameter);

      *(header *)m_buffer = l_header;

      auto l_output_paramters_range =
          container::range<ui8>::make(m_buffer + l_header.m_output_offset,
                                      l_header.m_output_byte_size)
              .cast_to<output_parameter>();
      l_output_paramters_range.copy_from(p_output_parameters);
    };

    header &get_header() { return *(header *)m_buffer; };

    container::range<output_parameter> get_output_parameters() {
      container::range<output_parameter> l_return;
      header &l_header = get_header();
      l_return.m_count = l_header.m_output_count;
      l_return.m_begin =
          (output_parameter *)(m_buffer + l_header.m_output_offset);
      return l_return;
    };
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
                                        m::vec<f32, 4> &out_screen_position,
                                        container::span<ui8 *> &out_vertex);

struct shader_utils {
  template <typename T>
  static const T &get_vertex(const shader_vertex_runtime_ctx &p_ctx,
                             bgfx::Attrib::Enum p_attrib, const ui8 *p_vertex) {
    return *(T *)(p_vertex + p_ctx.m_vertex_layout.m_offset[p_attrib]);
  };
};

struct shader_view {
  static uimax vertex_shader_size_in_bytes(
      const container::range<shader_vertex_meta::output_parameter>
          &p_output_parameters) {
    return sizeof(shader_vertex_function) +
           shader_vertex_meta::size_in_bytes(p_output_parameters);
  };

  ui8 *m_buffer;

  shader_view(ui8 *p_buffer) : m_buffer(p_buffer){};

  void initialize(shader_vertex_function *p_function,
                  const container::range<shader_vertex_meta::output_parameter>
                      &p_output_parameters) {
    sys::memcpy(m_buffer, p_function, sizeof(*p_function));
    shader_vertex_meta::view(m_buffer + sizeof(p_function))
        .initialize(p_output_parameters);
  };

  shader_vertex_function *get_function() {
    return (shader_vertex_function *)m_buffer;
  };

  shader_vertex_meta::view get_vertex_meta() {
    return shader_vertex_meta::view(m_buffer + sizeof(shader_vertex_function));
  };
};

} // namespace rast