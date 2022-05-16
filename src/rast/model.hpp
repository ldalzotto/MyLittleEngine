#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <m/mat.hpp>
#include <m/rect.hpp>

namespace rast {

// TODO -> improve that
static void image_copy_stretch(m::vec<ui8, 3> *p_from, ui16 p_from_width,
                               ui16 p_from_height, m::vec<ui8, 4> *p_to,
                               ui16 p_to_width, ui16 p_to_height) {
  ff32 l_width_delta_ratio = ff32(p_from_width) / p_to_width;
  ff32 l_height_delta_ratio = ff32(p_from_height) / p_to_height;
  for (auto y = 0; y < p_to_height; ++y) {
    ui16 l_from_y = ui16((l_height_delta_ratio * y).to_f32());
    for (auto x = 0; x < p_to_width; ++x) {
      ui16 l_from_x = ui16((l_width_delta_ratio * x).to_f32());
      *(m::vec<ui8, 3> *)&p_to[x + (y * p_to_width)] =
          p_from[l_from_x + (l_from_y * p_from_height)];
    }
  }
};

struct image_view {
  const ui16 &m_width;
  const ui16 &m_height;
  const ui8 &m_bits_per_pixel;
  container::range<ui8> m_buffer;
  uimax m_stride;

  image_view(const ui16 &p_width, const ui16 &p_height,
             const ui8 &p_bits_per_pixel, const container::range<ui8> &p_buffer)
      : m_width(p_width), m_height(p_height),
        m_bits_per_pixel(p_bits_per_pixel), m_buffer(p_buffer) {
    m_stride = m_bits_per_pixel * m_width;
  };

  template <typename SizeType> uimax get_buffer_index(SizeType r, SizeType c) {

    assert_debug(r < m_height);
    assert_debug(c < m_width);

    return (r * m_stride) + (c * m_bits_per_pixel);
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
  void set_pixel(SizeType r, SizeType c, const m::vec<ui8, 3> &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_bits_per_pixel);
    *(m::vec<ui8, 3> *)at(r, c) = p_pixel;
  };

  template <typename SizeType>
  void set_pixel(SizeType p, const m::vec<ui8, 3> &p_pixel) {
    assert_debug(sizeof(p_pixel) == m_bits_per_pixel);
    *(m::vec<ui8, 3> *)at(p) = p_pixel;
  };

  uimax size_of() { return m_stride * m_height; };
  uimax pixel_count() { return m_height * m_width; };

  template <typename T, typename CallbackFunc>
  void for_each(const CallbackFunc &p_callback) {
    assert_debug(m_bits_per_pixel >= sizeof(T));
    auto l_pix_count = pixel_count();
    for (auto i = 0; i < l_pix_count; ++i) {
      p_callback(*(T *)at(i));
    }
  };

  void copy_to(const image_view &p_other) {
    m_buffer.copy_to(p_other.m_buffer);
  };
};

struct shader_vertex_runtime_ctx {
  const m::mat<ff32, 4, 4> &m_proj;
  const m::mat<ff32, 4, 4> &m_view;
  const m::mat<ff32, 4, 4> &m_transform;
  const m::mat<ff32, 4, 4> &m_local_to_unit;
  const bgfx::VertexLayout &m_vertex_layout;

  shader_vertex_runtime_ctx(const m::mat<ff32, 4, 4> &p_proj,
                            const m::mat<ff32, 4, 4> &p_view,
                            const m::mat<ff32, 4, 4> &p_transform,
                            const m::mat<ff32, 4, 4> &p_local_to_unit,
                            const bgfx::VertexLayout &p_vertex_layout)
      : m_proj(p_proj), m_view(p_view), m_transform(p_transform),
        m_local_to_unit(p_local_to_unit), m_vertex_layout(p_vertex_layout){};
};

using shader_vertex_function = void (*)(const shader_vertex_runtime_ctx &p_ctx,
                                        const ui8 *p_vertex,
                                        m::vec<ff32, 4> &out_screen_position,
                                        ui8 **out_vertex);

using shader_fragment_function = void (*)(ui8 **p_vertex_output_interpolated,
                                          m::vec<ff32, 3> &out_color);

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
      m_single_element_size = sizeof(ff32);
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