#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <m/mat.hpp>
#include <m/rect.hpp>
#include <m/vec.hpp>
#include <rast/model.hpp>

namespace rast {
namespace algorithm {

struct multi_buffer {

  container::span<container::span<ui8>> m_buffers;
  container::vector<uimax> m_locked_buffers;
  void allocate() {
    m_buffers.allocate(0);
    m_locked_buffers.allocate(0);
  };

  void free() {
    assert_debug(m_locked_buffers.count() == 0);
    __for_each_buffer(
        [](container::span<ui8> &p_buffer, auto) { p_buffer.free(); });
    m_buffers.free();
    m_locked_buffers.free();
  };

  void resize(uimax p_count) {
    uimax l_count_before = m_buffers.count();
    m_buffers.resize(p_count);
    for (auto i = l_count_before; i < m_buffers.count(); ++i) {
      m_buffers.at(i).allocate(0);
    }
  };

  container::span<ui8> &borrow_buffer() {
    uimax l_index = -1;
    for (auto l_buffer_iter = 0; l_buffer_iter < m_buffers.count();
         ++l_buffer_iter) {
      if (!__is_buffer_locked(l_buffer_iter)) {
        l_index = l_buffer_iter;
        m_locked_buffers.push_back(l_index);
        break;
      }
    }
    return m_buffers.at(l_index);
  };

  void release_all() { m_locked_buffers.clear(); };

private:
  template <typename CallbacFunc>
  void __for_each_buffer(const CallbacFunc &p_callback) {
    for (auto i = 0; i < m_buffers.count(); ++i) {
      p_callback(m_buffers.at(i), i);
    }
  };

  ui8 __is_buffer_locked(uimax p_buffer_index) {
    for (auto l_locked_buffer_iter = 0;
         l_locked_buffer_iter < m_locked_buffers.count();
         ++l_locked_buffer_iter) {
      if (m_locked_buffers.at(l_locked_buffer_iter) == p_buffer_index) {
        return 1;
      }
    }
    return 0;
  }
};

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

// TODO -> move to math
struct screen_polygon {
  m::vec<i16, 2> m_0, m_1, m_2;

  m::rect_min_max<i16> calculate_bounding_rect() {
    container::range<m::vec<i16, 2>> l_points;
    l_points.m_begin = &m_0;
    l_points.m_count = 3;
    return m::rect_min_max<i16>::bounding_box(l_points);
  };
};

struct utils {

  template <typename CallbackFunc>
  static void rasterize_polygon(const screen_polygon &p_polygon,
                                m::rect_min_max<i16> &p_bounding_rect,
                                const CallbackFunc &p_cb) {
    m::vec<i16, 2> l_pixel = {p_bounding_rect.min().x(),
                              p_bounding_rect.min().y()};

    const m::vec<i16, 2> d0 = p_polygon.m_0 - p_polygon.m_2;
    const m::vec<i16, 2> d1 = p_polygon.m_1 - p_polygon.m_0;
    const m::vec<i16, 2> d2 = p_polygon.m_2 - p_polygon.m_1;
    i16 ey0 = __ey_calculation(l_pixel, p_polygon.m_0, d0);
    i16 ey1 = __ey_calculation(l_pixel, p_polygon.m_1, d1);
    i16 ey2 = __ey_calculation(l_pixel, p_polygon.m_2, d2);

    for (auto y = p_bounding_rect.min().y(); y <= p_bounding_rect.max().y();
         ++y) {
      i16 ex0 = ey0;
      i16 ex1 = ey1;
      i16 ex2 = ey2;

      for (auto x = p_bounding_rect.min().x(); x <= p_bounding_rect.max().x();
           ++x) {

        if (ex0 >= 0 && ex1 >= 0 && ex2 >= 0) {
          p_cb(x, y);
        }

        ex0 += d0.y();
        ex1 += d1.y();
        ex2 += d2.y();
      }

      ey0 -= d0.x();
      ey1 -= d1.x();
      ey2 -= d2.x();
    }
  };

  template <typename CallbackFunc>
  static void rasterize_polygon_weighted(const screen_polygon &p_polygon,
                                         m::rect_min_max<i16> &p_bounding_rect,
                                         const CallbackFunc &p_cb) {

    i16 l_area =
        m::cross(p_polygon.m_2 - p_polygon.m_0, p_polygon.m_1 - p_polygon.m_0);

    if (l_area > 0) {

      m::vec<i16, 2> l_pixel = {p_bounding_rect.min().x(),
                                p_bounding_rect.min().y()};

      const m::vec<i16, 2> d0 = p_polygon.m_0 - p_polygon.m_2;
      const m::vec<i16, 2> d1 = p_polygon.m_1 - p_polygon.m_0;
      const m::vec<i16, 2> d2 = p_polygon.m_2 - p_polygon.m_1;
      i16 ey0 = __ey_calculation(l_pixel, p_polygon.m_0, d0);
      i16 ey1 = __ey_calculation(l_pixel, p_polygon.m_1, d1);
      i16 ey2 = __ey_calculation(l_pixel, p_polygon.m_2, d2);

      for (auto y = p_bounding_rect.min().y(); y <= p_bounding_rect.max().y();
           ++y) {
        i16 ex0 = ey0;
        i16 ex1 = ey1;
        i16 ex2 = ey2;

        for (auto x = p_bounding_rect.min().x(); x <= p_bounding_rect.max().x();
             ++x) {

          if (ex0 >= 0 && ex1 >= 0 && ex2 >= 0) {
            f32 w0 = (f32)ex1 / l_area;
            f32 w1 = (f32)ex0 / l_area;
            f32 w2 = (f32)ex2 / l_area;
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

  static void rasterize_polygon_to_visiblity(
      const screen_polygon &p_polygon, m::rect_min_max<i16> &p_bounding_rect,
      container::span<ui8> &p_visibility, ui16 p_width) {
    rasterize_polygon(p_polygon, p_bounding_rect, [&](auto x, auto y) {
      p_visibility.at((y * p_width) + x) = 1;
    });
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

struct rasterize_heap {
  container::span<m::vec<i16, 2>> m_screen_vertices;
  container::span<screen_polygon> m_screen_polygons;

  multi_buffer m_vertex_parameter_buffers;
  container::span<ui8 *> m_vertex_output_parameter_references;

  struct visiblity {
    table_span_meta;
    using type_0 = ui8;
    using type_1 = m::vec<f32, 3>;
    type_0 *m_col_0;
    type_1 *m_col_1;
    table_define_span_2;
  } m_visibility_buffer;

  // container::span<ui8> m_visibility_buffer;
  multi_buffer m_interpolated_vertex_output;
  container::span<ui8 *> m_interpolated_vertex_parameter_references;

  void allocate() {
    m_screen_vertices.allocate(1024);
    m_screen_polygons.allocate(1024);
    m_visibility_buffer.allocate(1024);
    m_vertex_parameter_buffers.allocate();
    m_vertex_output_parameter_references.allocate(0);
    m_interpolated_vertex_output.allocate();
    m_interpolated_vertex_parameter_references.allocate(0);
  };

  void free() {
    m_screen_vertices.free();
    m_screen_polygons.free();
    m_visibility_buffer.free();
    m_vertex_parameter_buffers.free();
    m_vertex_output_parameter_references.free();
    m_interpolated_vertex_output.free();
    m_interpolated_vertex_parameter_references.free();
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
    __extract_screen_polygons();
    __calculate_visibility_buffer();
    __interpolate_vertex_output();
    __fragment();

    __terminate();
  };

private:
  void __terminate() {
    m_heap.m_vertex_parameter_buffers.release_all();
    m_heap.m_interpolated_vertex_output.release_all();
  };

  void __vertex_v2() {
    m_heap.m_screen_vertices.resize(m_vertex_count);

    const shader_vertex_runtime_ctx l_ctx = shader_vertex_runtime_ctx(
        m_input.m_rect, m_input.m_proj, m_input.m_view, m_input.m_transform,
        m_local_to_unit, m_input.m_vertex_layout);

    assert_debug(m_input.m_program.m_vertex);
    auto l_shader_view = shader_view((ui8 *)m_input.m_program.m_vertex);
    shader_vertex_function l_vertex_function = *l_shader_view.get_function();
    auto &l_shader_header = l_shader_view.get_vertex_meta().get_header();
    auto l_output_parameters =
        l_shader_view.get_vertex_meta().get_output_parameters();

    m_heap.m_vertex_parameter_buffers.resize(l_shader_header.m_output_count);
    m_heap.m_vertex_output_parameter_references.resize(
        l_shader_header.m_output_count);

    for (auto i = 0; i < l_shader_header.m_output_count; ++i) {
      container::span<ui8> &l_output_buffer =
          m_heap.m_vertex_parameter_buffers.borrow_buffer();
      l_output_buffer.resize(l_output_parameters.at(i).m_single_element_size *
                             m_vertex_count);
      m_heap.m_vertex_output_parameter_references.at(i) =
          l_output_buffer.m_data;
    }

    for (auto i = 0; i < m_vertex_count; ++i) {
      ui8 *l_vertex_bytes =
          m_input.m_vertex_buffer.m_begin + (i * m_vertex_stride);
      m::vec<f32, 4> l_vertex_shader_out;

      l_vertex_function(l_ctx, l_vertex_bytes, l_vertex_shader_out,
                        m_heap.m_vertex_output_parameter_references);

      l_vertex_shader_out = l_vertex_shader_out / l_vertex_shader_out.w();

      m::vec<f32, 2> l_vertex_shader_out_2 =
          m::vec<f32, 2>::make(l_vertex_shader_out);

      l_vertex_shader_out_2 = (l_vertex_shader_out_2 + 1) * 0.5;
      l_vertex_shader_out_2.y() = 1 - l_vertex_shader_out_2.y();
      l_vertex_shader_out_2 *= (m_input.m_rect.extend() - 1);

      m_heap.m_screen_vertices.at(i) = l_vertex_shader_out_2.cast<i16>();

      for (auto l_output_it = 0; l_output_it < l_shader_header.m_output_count;
           ++l_output_it) {
        m_heap.m_vertex_output_parameter_references.at(l_output_it) +=
            l_output_parameters.at(l_output_it).m_single_element_size;
      }
    }
  };

  void __extract_screen_polygons() {
    m_heap.m_screen_polygons.resize(m_polygon_count);

    uimax l_index_idx = 0;
    for (auto i = 0; i < m_polygon_count; ++i) {
      screen_polygon &l_polygon = m_heap.m_screen_polygons.at(i);
      l_polygon.m_0 = m_heap.m_screen_vertices.at(
          m_input.m_index_buffer.at<ui16>(l_index_idx));
      l_polygon.m_1 = m_heap.m_screen_vertices.at(
          m_input.m_index_buffer.at<ui16>(l_index_idx + 1));
      l_polygon.m_2 = m_heap.m_screen_vertices.at(
          m_input.m_index_buffer.at<ui16>(l_index_idx + 2));
      l_index_idx += 3;
    }
  };

  // TODO -> should apply z clipping
  // TODO -> should apply depth comparison if needed.
  void __calculate_visibility_buffer() {
    m_heap.m_visibility_buffer.resize(
        m_input.m_target_image_view.pixel_count());

    container::range<ui8> l_visibility_range;
    m_heap.m_visibility_buffer.range(&l_visibility_range);
    l_visibility_range.m_count = m_input.m_target_image_view.pixel_count();
    l_visibility_range.zero();

    for (auto l_polygon_it = 0; l_polygon_it < m_polygon_count;
         ++l_polygon_it) {
      screen_polygon &l_polygon = m_heap.m_screen_polygons.at(l_polygon_it);

      m::rect_min_max<i16> l_bounding_rect =
          l_polygon.calculate_bounding_rect();

      l_bounding_rect = m::fit_into(l_bounding_rect, m_input.m_rect);

      utils::rasterize_polygon_weighted(
          l_polygon, l_bounding_rect,
          [&](auto x, auto y, f32 w0, f32 w1, f32 w2) {
            ui8 *l_visibility_boolean;
            m::vec<f32, 3> *l_visibility_weight;
            auto l_visibility_index =
                (y * m_input.m_target_image_view.m_target_info.width) + x;
            m_heap.m_visibility_buffer.at(l_visibility_index,
                                          &l_visibility_boolean,
                                          &l_visibility_weight);
            *l_visibility_boolean = 1;
            *l_visibility_weight = {w0, w1, w2};
          });
    }
  };

  void __interpolate_vertex_output() {
    auto l_shader_view = shader_view((ui8 *)m_input.m_program.m_vertex);
    auto l_vertex_output_parameters =
        l_shader_view.get_vertex_meta().get_output_parameters();

    // Vertex output initialization
    m_heap.m_interpolated_vertex_output.resize(
        l_vertex_output_parameters.count());
    m_heap.m_interpolated_vertex_parameter_references.resize(
        l_vertex_output_parameters.count());
    for (auto i = 0; i < l_vertex_output_parameters.count(); ++i) {
      container::span<ui8> &l_vertex_output =
          m_heap.m_interpolated_vertex_output.borrow_buffer();
      l_vertex_output.resize(l_shader_view.get_vertex_meta()
                                 .get_output_parameters()
                                 .at(i)
                                 .m_single_element_size *
                             m_input.m_target_image_view.pixel_count());
      m_heap.m_interpolated_vertex_parameter_references.at(i) =
          l_vertex_output.m_data;
    }

    ui8 *l_visibility_boolean;
    m::vec<f32, 3> *l_visibility_weight;
    for (auto i = 0; i < m_input.m_target_image_view.pixel_count(); ++i) {
      m_heap.m_visibility_buffer.at(i, &l_visibility_boolean,
                                    &l_visibility_weight);
      if (*l_visibility_boolean) {

        for (auto j = 0; j < l_vertex_output_parameters.count(); ++j) {

          ui8 *l_interpolated_vertex_output =
              m_heap.m_vertex_output_parameter_references.at(j);

          // TODO -> perform interpolation
        }
      }
    }

    // TODO
  };

  void __fragment() {
    for (auto i = 0; i < m_input.m_target_image_view.pixel_count(); ++i) {
      ui8 *l_visibility_boolean;
      m_heap.m_visibility_buffer.at(i, &l_visibility_boolean, orm::none());
      if (*l_visibility_boolean) {
        m::vec<ui8, 3> l_color = {255, 255, 255};
        m_input.m_target_image_view.set_pixel(i, l_color);
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
