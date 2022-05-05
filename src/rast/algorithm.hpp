#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <m/mat.hpp>
#include <m/polygon.hpp>
#include <m/rect.hpp>
#include <m/vec.hpp>
#include <rast/model.hpp>

namespace rast {
namespace algorithm {

struct index_buffer_const_view {
  ui8 m_index_byte_size;
  uimax m_index_count;
  const container::range<ui8> &m_buffer;

  index_buffer_const_view(const container::range<ui8> &p_buffer)
      : m_buffer(p_buffer) {
    m_index_byte_size = sizeof(ui16);
    m_index_count = m_buffer.count() / m_index_byte_size;
  };

  template <typename T> const T &at(uimax p_index) const {
    assert_debug(sizeof(T) == m_index_byte_size);
    return *(T *)(m_buffer.m_begin + (p_index * m_index_byte_size));
  };
};

struct program {
  void *m_vertex;
  void *m_fragment;
};

using screen_polygon = m::polygon<m::vec<i16, 2>, 3>;

struct utils {

  template <typename CallbackFunc>
  static void rasterize_polygon_weighted(const screen_polygon &p_polygon,
                                         m::rect_min_max<i16> &p_bounding_rect,
                                         const CallbackFunc &p_cb) {

    i16 l_area = m::cross(p_polygon.p2() - p_polygon.p0(),
                          p_polygon.p1() - p_polygon.p0());

    if (l_area > 0) {

      m::vec<i16, 2> l_pixel = {p_bounding_rect.min().x(),
                                p_bounding_rect.min().y()};

      const m::vec<i16, 2> d0 = p_polygon.p0() - p_polygon.p2();
      const m::vec<i16, 2> d1 = p_polygon.p1() - p_polygon.p0();
      const m::vec<i16, 2> d2 = p_polygon.p2() - p_polygon.p1();
      i16 ey0 = __ey_calculation(l_pixel, p_polygon.p0(), d0);
      i16 ey1 = __ey_calculation(l_pixel, p_polygon.p1(), d1);
      i16 ey2 = __ey_calculation(l_pixel, p_polygon.p2(), d2);

      for (auto y = p_bounding_rect.min().y(); y <= p_bounding_rect.max().y();
           ++y) {
        i16 ex0 = ey0;
        i16 ex1 = ey1;
        i16 ex2 = ey2;

        for (auto x = p_bounding_rect.min().x(); x <= p_bounding_rect.max().x();
             ++x) {

          if (ex0 >= 0 && ex1 >= 0 && ex2 >= 0) {
            f32 w0 = (f32)ex2 / l_area;
            f32 w1 = (f32)ex0 / l_area;
            f32 w2 = (f32)ex1 / l_area;
            assert_debug(w0 + w1 + w2 <= 1.01f);
            p_cb(x, y, w0, w1, w2);
          }

          ex0 += d0.y();
          ex1 += d1.y();
          ex2 += d2.y();
        }

        ey0 -= d0.x();
        ey1 -= d1.x();
        ey2 -= d2.x();
      }
    }
  };

private:
  static i16 __ey_calculation(const m::vec<i16, 2> &p_pixel,
                              const m::vec<i16, 2> &p_polygon_point,
                              const m::vec<i16, 2> &p_delta) {
    return ((p_pixel.x() - p_polygon_point.x()) * p_delta.y()) -
           ((p_pixel.y() - p_polygon_point.y()) * p_delta.x());
  };
};

// struct

using polygon_vertex_indices = m::polygon<uimax, 3>;
using pixel_coordinates = m::vec<i16, 2>;
using rasterization_weight = m::vec<f32, 3>;

struct rasterize_heap {

  struct per_vertices {
    table_span_meta;
    table_cols_1(pixel_coordinates);
    table_define_span_1;
  } m_per_vertices;

  struct per_polygons {
    table_span_meta;
    table_cols_2(screen_polygon, polygon_vertex_indices);
    table_define_span_2;
  } m_per_polygons;

  container::multi_byte_buffer m_vertex_output;
  container::span<ui8 *> m_vertex_output_send_to_vertex_shader;

  struct visiblity {
    table_span_meta;
    table_cols_3(ui8, rasterization_weight, uimax);
    table_define_span_3;
  } m_visibility_buffer;

  container::multi_byte_buffer m_vertex_output_interpolated;
  container::span<ui8 *> m_vertex_output_interpolated_send_to_fragment_shader;

  void allocate() {
    m_per_vertices.allocate(1024);
    m_per_polygons.allocate(1024);
    m_visibility_buffer.allocate(1024);
    m_vertex_output.allocate();
    m_vertex_output_interpolated.allocate();

    m_vertex_output_send_to_vertex_shader.allocate(128);
    m_vertex_output_interpolated_send_to_fragment_shader.allocate(128);
  };

  void free() {
    m_per_vertices.free();
    m_per_polygons.free();
    m_visibility_buffer.free();
    m_vertex_output.free();
    m_vertex_output_interpolated.free();

    m_vertex_output_send_to_vertex_shader.free();
    m_vertex_output_interpolated_send_to_fragment_shader.free();
  };

  pixel_coordinates &get_pixel_coordinates(ui32 p_index) {
    pixel_coordinates *l_pixel_coordinate;
    m_per_vertices.at(p_index, &l_pixel_coordinate);
    return *l_pixel_coordinate;
  };
};

struct rasterize_unit {

  struct input {
    const program &m_program;
    m::rect_point_extend<ui16> &m_rect;
    const m::mat<f32, 4, 4> &m_proj;
    const m::mat<f32, 4, 4> &m_view;
    const m::mat<f32, 4, 4> &m_transform;
    index_buffer_const_view m_index_buffer;
    bgfx::VertexLayout m_vertex_layout;
    const container::range<ui8> &m_vertex_buffer;
    ui64 m_state;
    ui32 m_rgba;
    image_view m_target_image_view;

    input(const program &p_program, m::rect_point_extend<ui16> &p_rect,
          const m::mat<f32, 4, 4> &p_proj, const m::mat<f32, 4, 4> &p_view,
          const m::mat<f32, 4, 4> &p_transform,
          const container::range<ui8> &p_index_buffer,
          bgfx::VertexLayout p_vertex_layout,
          const container::range<ui8> &p_vertex_buffer, ui64 p_state,
          ui32 p_rgba, const bgfx::TextureInfo &p_target_info,
          container::range<ui8> &p_target_buffer)
        : m_program(p_program), m_rect(p_rect), m_proj(p_proj), m_view(p_view),
          m_transform(p_transform), m_index_buffer(p_index_buffer),
          m_vertex_layout(p_vertex_layout), m_vertex_buffer(p_vertex_buffer),
          m_state(p_state), m_rgba(p_rgba),
          m_target_image_view(p_target_info, p_target_buffer){};

  } m_input;

  rasterize_heap &m_heap;
  m::mat<f32, 4, 4> m_local_to_unit;
  ui16 m_vertex_stride;
  uimax m_vertex_count;
  uimax m_polygon_count;

  m::rect_min_max<ui16> m_rendered_rect;

  rasterize_unit(rasterize_heap &p_heap, const program &p_program,
                 m::rect_point_extend<ui16> &p_rect,
                 const m::mat<f32, 4, 4> &p_proj,
                 const m::mat<f32, 4, 4> &p_view,
                 const m::mat<f32, 4, 4> &p_transform,
                 const container::range<ui8> &p_index_buffer,
                 bgfx::VertexLayout p_vertex_layout,
                 const container::range<ui8> &p_vertex_buffer, ui64 p_state,
                 ui32 p_rgba, const bgfx::TextureInfo &p_target_info,
                 container::range<ui8> &p_target_buffer)
      : m_input(p_program, p_rect, p_proj, p_view, p_transform, p_index_buffer,
                p_vertex_layout, p_vertex_buffer, p_state, p_rgba,
                p_target_info, p_target_buffer),
        m_heap(p_heap){};

  void rasterize() {

    m_vertex_stride = m_input.m_vertex_layout.getStride();
    m_vertex_count = m_input.m_vertex_buffer.count() / m_vertex_stride;
    assert_debug(m_input.m_vertex_layout.getSize(m_vertex_count) ==
                 m_input.m_vertex_buffer.count());

    m_polygon_count = m_input.m_index_buffer.m_index_count / 3;
    m_local_to_unit = m_input.m_proj * m_input.m_view * m_input.m_transform;
    block_debug([&]() {
      ui8 l_position_num;
      bgfx::AttribType::Enum l_position_type;
      bool l_position_normalized;
      bool l_position_as_int;
      m_input.m_vertex_layout.decode(bgfx::Attrib::Enum::Position,
                                     l_position_num, l_position_type,
                                     l_position_normalized, l_position_as_int);
      assert_debug(l_position_type == bgfx::AttribType::Float);
      assert_debug(l_position_num == 3);
      assert_debug(!l_position_normalized);
      assert_debug(!l_position_as_int);
    });

    __vertex_v2();
    __intialize_rendered_rect();
    __extract_screen_polygons();
    __calculate_visibility_buffer();
    __interpolate_vertex_output();
    __fragment();

    __terminate();
  };

private:
  void __terminate(){};

  void __vertex_v2() {
    m_heap.m_per_vertices.resize(m_vertex_count);

    const shader_vertex_runtime_ctx l_ctx = shader_vertex_runtime_ctx(
        m_input.m_proj, m_input.m_view, m_input.m_transform, m_local_to_unit,
        m_input.m_vertex_layout);

    assert_debug(m_input.m_program.m_vertex);
    auto l_shader_view = shader_view((ui8 *)m_input.m_program.m_vertex);
    shader_vertex_function l_vertex_function = *l_shader_view.get_function();
    auto &l_shader_header = l_shader_view.get_vertex_meta().get_header();
    auto l_output_parameters =
        l_shader_view.get_vertex_meta().get_output_parameters();

    m_heap.m_vertex_output.resize_col_capacity(l_shader_header.m_output_count);
    for (auto l_col_it = 0; l_col_it < m_heap.m_vertex_output.m_col_count;
         l_col_it++) {
      m_heap.m_vertex_output.col(l_col_it).resize(
          m_vertex_count,
          l_output_parameters.at(l_col_it).m_single_element_size);
    }

    for (auto i = 0; i < m_vertex_count; ++i) {
      ui8 *l_vertex_bytes =
          m_input.m_vertex_buffer.m_begin + (i * m_vertex_stride);

      for (auto l_output_it = 0; l_output_it < l_shader_header.m_output_count;
           ++l_output_it) {
        m_heap.m_vertex_output_send_to_vertex_shader.at(l_output_it) =
            m_heap.m_vertex_output.at(l_output_it, i);
      }

      m::vec<f32, 4> l_vertex_shader_out;
      l_vertex_function(l_ctx, l_vertex_bytes, l_vertex_shader_out,
                        m_heap.m_vertex_output_send_to_vertex_shader);

      l_vertex_shader_out = l_vertex_shader_out / l_vertex_shader_out.w();

      m::vec<f32, 2> l_pixel_coordinates_f32 =
          m::vec<f32, 2>::make(l_vertex_shader_out);

      l_pixel_coordinates_f32 = (l_pixel_coordinates_f32 + 1) * 0.5;
      l_pixel_coordinates_f32.y() = 1 - l_pixel_coordinates_f32.y();
      l_pixel_coordinates_f32 *= (m_input.m_rect.extend() - 1);

      m_heap.get_pixel_coordinates(i) = l_pixel_coordinates_f32.cast<i16>();
    }
  };

  void __extract_screen_polygons() {
    m_heap.m_per_polygons.resize(m_polygon_count);

    uimax l_index_idx = 0;
    for (auto i = 0; i < m_polygon_count; ++i) {
      polygon_vertex_indices *l_polygon_indices;
      screen_polygon *l_polygon;

      m_heap.m_per_polygons.at(i, &l_polygon, &l_polygon_indices);

      l_polygon_indices->p0() = m_input.m_index_buffer.at<ui16>(l_index_idx);
      l_polygon_indices->p1() =
          m_input.m_index_buffer.at<ui16>(l_index_idx + 1);
      l_polygon_indices->p2() =
          m_input.m_index_buffer.at<ui16>(l_index_idx + 2);

      l_polygon->p0() = m_heap.get_pixel_coordinates(l_polygon_indices->p0());
      l_polygon->p1() = m_heap.get_pixel_coordinates(l_polygon_indices->p1());
      l_polygon->p2() = m_heap.get_pixel_coordinates(l_polygon_indices->p2());

      l_index_idx += 3;
    }
  };

  void __intialize_rendered_rect() {
    pixel_coordinates *l_first_vertex_pixel_coordinates;
    m_heap.m_per_vertices.at(0, &l_first_vertex_pixel_coordinates);
    m_rendered_rect.min() = l_first_vertex_pixel_coordinates->cast<ui16>();
    m_rendered_rect.max() = l_first_vertex_pixel_coordinates->cast<ui16>();
  };

  // TODO -> should apply z clipping
  // TODO -> should apply depth comparison if needed.
  void __calculate_visibility_buffer() {
    m_heap.m_visibility_buffer.resize(
        m_input.m_target_image_view.pixel_count());

    container::range<ui8> l_visibility_range;
    m_heap.m_visibility_buffer.range(&l_visibility_range, orm::none(),
                                     orm::none());
    l_visibility_range.m_count = m_input.m_target_image_view.pixel_count();
    l_visibility_range.zero();

    for (auto l_polygon_it = 0; l_polygon_it < m_polygon_count;
         ++l_polygon_it) {
      screen_polygon *l_polygon;
      m_heap.m_per_polygons.at(l_polygon_it, &l_polygon, orm::none());

      m::rect_min_max<i16> l_bounding_rect = m::bounding_rect(*l_polygon);

      l_bounding_rect = m::fit_into(l_bounding_rect, m_input.m_rect);
      m_rendered_rect = m::extend(m_rendered_rect, l_bounding_rect);

      utils::rasterize_polygon_weighted(
          *l_polygon, l_bounding_rect,
          [&](auto x, auto y, f32 w0, f32 w1, f32 w2) {
            ui8 *l_visibility_boolean;
            rasterization_weight *l_visibility_weight;
            uimax *l_polyton_index;
            auto l_visibility_index =
                (y * m_input.m_target_image_view.m_target_info.width) + x;
            m_heap.m_visibility_buffer.at(
                l_visibility_index, &l_visibility_boolean, &l_visibility_weight,
                &l_polyton_index);
            *l_visibility_boolean = 1;
            *l_visibility_weight = {w0, w1, w2};
            *l_polyton_index = l_polygon_it;
          });
    }
  };

  void __interpolate_vertex_output() {
    auto l_shader_view = shader_view((ui8 *)m_input.m_program.m_vertex);
    auto l_vertex_output_parameters =
        l_shader_view.get_vertex_meta().get_output_parameters();

    m_heap.m_vertex_output_interpolated.resize_col_capacity(
        l_vertex_output_parameters.count());
    for (auto l_col_it = 0;
         l_col_it < m_heap.m_vertex_output_interpolated.m_col_count;
         ++l_col_it) {
      m_heap.m_vertex_output_interpolated.col(l_col_it).resize(
          m_input.m_target_image_view.pixel_count(),
          l_vertex_output_parameters.at(l_col_it).m_single_element_size);
    }

    m_heap.m_vertex_output_interpolated_send_to_fragment_shader.resize(
        l_vertex_output_parameters.count());

    ui8 *l_visibility_boolean;
    rasterization_weight *l_visibility_weight;
    uimax *l_visibility_polygon;

    __for_each_rendered_pixels([&](uimax p_pixel_index) {
      m_heap.m_visibility_buffer.at(p_pixel_index, &l_visibility_boolean,
                                    &l_visibility_weight,
                                    &l_visibility_polygon);
      if (*l_visibility_boolean) {
        polygon_vertex_indices *l_indices_polygon;
        m_heap.m_per_polygons.at(*l_visibility_polygon, orm::none(),
                                 &l_indices_polygon);

        for (auto l_vertex_output_index = 0;
             l_vertex_output_index < l_vertex_output_parameters.count();
             ++l_vertex_output_index) {
          __interpolate_vertex_output_single_value(
              l_vertex_output_parameters, l_vertex_output_index,
              *l_indices_polygon, *l_visibility_weight, p_pixel_index);
        }
      }
    });
  };

  void __fragment() {
    __for_each_rendered_pixels([&](uimax p_pixel_index) {
      ui8 *l_visibility_boolean;
      m_heap.m_visibility_buffer.at(p_pixel_index, &l_visibility_boolean,
                                    orm::none(), orm::none());
      if (*l_visibility_boolean) {

        for (auto j = 0; j < m_heap.m_vertex_output_interpolated.m_col_count;
             ++j) {
          m_heap.m_vertex_output_interpolated_send_to_fragment_shader.at(j) =
              m_heap.m_vertex_output_interpolated.at(j, p_pixel_index);
        }

        // TODO -> use the return of the fragment shader instead.
        m::vec<f32, 3> *l_color_tmp =
            (m::vec<f32, 3> *)m_heap
                .m_vertex_output_interpolated_send_to_fragment_shader.at(0);
        m::vec<ui8, 3> l_color = (*l_color_tmp * 255).cast<ui8>();
        m_input.m_target_image_view.set_pixel(p_pixel_index, l_color);
      }
    });
  };

  template <typename CallbackFunc>
  void __for_each_rendered_pixels(const CallbackFunc &p_callback) {
    for (auto y = m_rendered_rect.min().y(); y <= m_rendered_rect.max().y();
         ++y) {
      for (auto x = m_rendered_rect.min().x(); x <= m_rendered_rect.max().x();
           ++x) {
        uimax l_pixel_index =
            (m_input.m_target_image_view.m_target_info.width * y) + x;
        p_callback(l_pixel_index);
      }
    }
  };

  void __interpolate_vertex_output_single_value(
      const container::range<shader_vertex_meta::output_parameter>
          &p_vertex_shader_outputs_meta,
      uimax p_vertex_output_index,
      const polygon_vertex_indices &p_indices_polygon,
      const rasterization_weight &p_polygon_weight, uimax p_pixel_index) {

    ui8 *l_interpolated_vertex_output = m_heap.m_vertex_output_interpolated.at(
        p_vertex_output_index, p_pixel_index);

    shader_vertex_meta::output_parameter l_output_parameter_meta =
        p_vertex_shader_outputs_meta.at(p_vertex_output_index);

    if (l_output_parameter_meta.m_attrib_type == bgfx::AttribType::Float) {
      if (l_output_parameter_meta.m_attrib_element_count == 3) {

        m::polygon<m::vec<f32, 3>, 3> l_attribute_polygon;

        l_attribute_polygon.p0() = *(m::vec<f32, 3> *)m_heap.m_vertex_output.at(
            p_vertex_output_index, p_indices_polygon.p0());

        l_attribute_polygon.p1() = *(m::vec<f32, 3> *)m_heap.m_vertex_output.at(
            p_vertex_output_index, p_indices_polygon.p1());

        l_attribute_polygon.p2() = *(m::vec<f32, 3> *)m_heap.m_vertex_output.at(
            p_vertex_output_index, p_indices_polygon.p2());

        m::vec<f32, 3> *l_interpolated_vertex_output =
            (m::vec<f32, 3> *)m_heap.m_vertex_output_interpolated.at(
                p_vertex_output_index, p_pixel_index);

        *l_interpolated_vertex_output =
            m::interpolate(l_attribute_polygon, p_polygon_weight);
      }
    }
  };
};

static inline void
rasterize(rasterize_heap &p_heap, const program &p_program,
          m::rect_point_extend<ui16> &p_rect, const m::mat<f32, 4, 4> &p_proj,
          const m::mat<f32, 4, 4> &p_view, const m::mat<f32, 4, 4> &p_transform,
          const container::range<ui8> &p_index_buffer,
          bgfx::VertexLayout p_vertex_layout,
          const container::range<ui8> &p_vertex_buffer, ui64 p_state,
          ui32 p_rgba, const bgfx::TextureInfo &p_target_info,
          container::range<ui8> &p_target_buffer) {
  rasterize_unit(p_heap, p_program, p_rect, p_proj, p_view, p_transform,
                 p_index_buffer, p_vertex_layout, p_vertex_buffer, p_state,
                 p_rgba, p_target_info, p_target_buffer)
      .rasterize();
};

} // namespace algorithm
} // namespace rast
