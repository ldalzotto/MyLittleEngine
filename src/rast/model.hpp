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

  uimax get_buffer_index(ui32 r, ui32 c) {

    assert_debug(r < m_target_info.height);
    assert_debug(c < m_target_info.width);

    return r * stride() + (c * m_target_info.bitsPerPixel);
  };
  uimax get_buffer_index(ui32 p) {

    assert_debug(p < (m_target_info.height * m_target_info.width));

    return m_target_info.bitsPerPixel * p;
  };

  ui8 *at(ui32 r, ui32 c) { return &m_buffer.at(get_buffer_index(r, c)); };
  ui8 *at(ui32 p) { return &m_buffer.at(get_buffer_index(p)); };

  template <typename T> T *at(ui32 r, ui32 c) {
    assert_debug(m_target_info.bitsPerPixel == sizeof(T));
    return (T *)at(r, c);
  };

  template <typename T> T *at(ui32 p) {
    assert_debug(m_target_info.bitsPerPixel == sizeof(T));
    return (T *)at(p);
  };

  void set_pixel(ui32 r, ui32 c, const m::vec<ui8, 3> &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_target_info.bitsPerPixel);
    *(m::vec<ui8, 3> *)at(r, c) = p_pixel;
  };

  void set_pixel(ui32 p, const m::vec<ui8, 3> &p_pixel) {
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

  template <typename Callback>
  void for_each_pixels_depth(const Callback &p_cb) {
    assert_debug(m_target_info.bitsPerPixel == sizeof(f32));
    auto l_pix_count = pixel_count();
    for (auto i = 0; i < l_pix_count; ++i) {
      p_cb(*(f32 *)at(i));
    }
  };
};

struct shader_vertex_runtime_ctx {
  const m::mat<f32, 4, 4> &m_proj;
  const m::mat<f32, 4, 4> &m_view;
  const m::mat<f32, 4, 4> &m_transform;
  const m::mat<f32, 4, 4> &m_local_to_unit;
  const bgfx::VertexLayout &m_vertex_layout;

  shader_vertex_runtime_ctx(const m::mat<f32, 4, 4> &p_proj,
                            const m::mat<f32, 4, 4> &p_view,
                            const m::mat<f32, 4, 4> &p_transform,
                            const m::mat<f32, 4, 4> &p_local_to_unit,
                            const bgfx::VertexLayout &p_vertex_layout)
      : m_proj(p_proj), m_view(p_view), m_transform(p_transform),
        m_local_to_unit(p_local_to_unit), m_vertex_layout(p_vertex_layout){};
};

using shader_vertex_function = void (*)(const shader_vertex_runtime_ctx &p_ctx,
                                        const ui8 *p_vertex,
                                        m::vec<f32, 4> &out_screen_position,
                                        ui8 **out_vertex);

using shader_fragment_function = void (*)(ui8 **p_vertex_output_interpolated,
                                          m::vec<f32, 3> &out_color);

struct shader_vertex {
  const shader_vertex_runtime_ctx &m_ctx;

  template <typename T>
  const T &get_vertex(bgfx::Attrib::Enum p_attrib, const ui8 *p_vertex) {
    return *(T *)(p_vertex + m_ctx.m_vertex_layout.m_offset[p_attrib]);
  };
};

struct shader_vertex_output_parameter {
  bgfx::AttribType::Enum m_attrib_type;
  ui8 m_attrib_element_count;
  ui16 m_single_element_size;

  shader_vertex_output_parameter() = default;
  shader_vertex_output_parameter(bgfx::AttribType::Enum p_attrib_type,
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

struct shader_vertex_bytes {

  struct table {
    uimax m_output_parameters_byte_size;
  };

  static uimax byte_size(uimax p_output_parameter_count) {
    return sizeof(table) + sizeof(uimax) +
           (sizeof(shader_vertex_output_parameter) * p_output_parameter_count) +
           sizeof(shader_vertex_function);
  };

  struct view {
    ui8 *m_data;

    void fill(const container::range<shader_vertex_output_parameter>
                  &p_output_parameters,
              shader_vertex_function p_function) {
      table *l_table = (table *)m_data;
      l_table->m_output_parameters_byte_size = p_output_parameters.size_of();

      uimax *l_output_parameters_count = (uimax *)(m_data + sizeof(table));
      *l_output_parameters_count = p_output_parameters.count();

      container::range<shader_vertex_output_parameter> l_byte_output_parameters;
      l_byte_output_parameters.m_begin =
          (shader_vertex_output_parameter *)(m_data + sizeof(table) +
                                             sizeof(uimax));
      l_byte_output_parameters.m_count = p_output_parameters.count();

      p_output_parameters.copy_to(l_byte_output_parameters);

      shader_vertex_function *l_function =
          (shader_vertex_function *)(m_data + sizeof(table) + sizeof(uimax) +
                                     l_table->m_output_parameters_byte_size);
      *l_function = p_function;
    };

    container::range<shader_vertex_output_parameter> output_parameters() {
      container::range<shader_vertex_output_parameter> l_range;
      l_range.m_begin =
          (shader_vertex_output_parameter *)(m_data + sizeof(table) +
                                             sizeof(uimax));
      l_range.m_count = *(uimax *)(m_data + sizeof(table));
      return l_range;
    };

    shader_vertex_function function() {
      table *l_table = (table *)m_data;
      return *(
          shader_vertex_function *)(m_data + sizeof(table) + sizeof(uimax) +
                                    l_table->m_output_parameters_byte_size);
    };
  };
};

struct shader_fragment_bytes {

  static uimax byte_size() { return sizeof(shader_fragment_function); };

  struct view {
    ui8 *m_data;

    void fill(shader_fragment_function p_function) {
      *(shader_fragment_function *)m_data = p_function;
    };

    shader_fragment_function fonction() {
      return *(shader_fragment_function *)m_data;
    };
  };
};

} // namespace rast