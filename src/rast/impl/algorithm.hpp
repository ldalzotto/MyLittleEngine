#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/orm.hpp>
#include <m/geom.hpp>
#include <m/polygon.hpp>
#include <m/rect.hpp>
#include <rast/model.hpp>
#include <shared/types.hpp>

#define TODO_NEAR_FAR_CLIPPING 0

namespace rast {

namespace algorithm {

using screen_coord_t = i16;
using polygon_vertex_indices = m::polygon<uimax, 3>;
using pixel_coordinates = m::vec<screen_coord_t, 2>;
using homogeneous_coordinates = position_t;
using rasterization_weight = m::vec<fix32, 3>;
using screen_polygon = m::polygon<m::vec<screen_coord_t, 2>, 3>;
using screen_polygon_bounding_box = m::rect_min_max<screen_coord_t>;
using screen_polygon_area = i32;

enum class CullMode { None = 0, Clockwise = 1, CounterClockwise = 2 };

struct render_state {
  enum class depth_test { Undefined = 0, Less = 1 } m_depth;

  ui8 m_depth_write;
  ui8 m_depth_read;
  CullMode m_cull_mode;

  static render_state from_int(ui64 p_state) {
    render_state l_state;

    if (p_state & BGFX_STATE_DEPTH_TEST_LESS) {
      l_state.m_depth = depth_test::Less;
      l_state.m_depth_read = 1;
    } else {
      l_state.m_depth = depth_test::Undefined;
      l_state.m_depth_read = 0;
    }

    if (p_state & BGFX_STATE_WRITE_Z) {
      l_state.m_depth_write = 1;
    } else {
      l_state.m_depth_write = 0;
    }

    l_state.m_depth_read = l_state.m_depth_read || l_state.m_depth_write;

    auto l_cull_state =
        ((p_state & BGFX_STATE_CULL_MASK) >> BGFX_STATE_CULL_SHIFT);
    if (l_cull_state == 1) {
      l_state.m_cull_mode = CullMode::Clockwise;
    } else if (l_cull_state == 2) {
      l_state.m_cull_mode = CullMode::CounterClockwise;
    } else {
      l_state.m_cull_mode = CullMode::None;
    }
    return l_state;
  };
};

struct index_buffer_const_view {
  ui8 m_index_byte_size;
  uimax m_index_count;
  const container::range<ui8> &m_buffer;

  index_buffer_const_view(const container::range<ui8> &p_buffer)
      : m_buffer(p_buffer) {
    m_index_byte_size = sizeof(vindex_t);
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

using program_uniforms = container::range<void *>;

struct utils {

  template <typename CallbackFunc>
  static void
  rasterize_polygon_weighted(const screen_polygon &p_polygon,
                             const screen_polygon_area &p_polygon_area,
                             m::rect_min_max<screen_coord_t> &p_bounding_rect,
                             const CallbackFunc &p_cb) {
    assert_debug(p_polygon_area > 0);

    pixel_coordinates l_pixel = {p_bounding_rect.min().x(),
                                 p_bounding_rect.min().y()};

    const pixel_coordinates d0 = p_polygon.p0() - p_polygon.p2();
    const pixel_coordinates d1 = p_polygon.p1() - p_polygon.p0();
    const pixel_coordinates d2 = p_polygon.p2() - p_polygon.p1();
    screen_polygon_area ey0 = __ey_calculation(l_pixel, p_polygon.p0(), d0);
    screen_polygon_area ey1 = __ey_calculation(l_pixel, p_polygon.p1(), d1);
    screen_polygon_area ey2 = __ey_calculation(l_pixel, p_polygon.p2(), d2);

    for (auto y = p_bounding_rect.min().y(); y < p_bounding_rect.max().y();
         ++y) {
      screen_polygon_area ex0 = ey0;
      screen_polygon_area ex1 = ey1;
      screen_polygon_area ex2 = ey2;

      for (auto x = p_bounding_rect.min().x(); x < p_bounding_rect.max().x();
           ++x) {

        if (ex0 >= 0 && ex1 >= 0 && ex2 >= 0) {
          fix32 w0 = (fix32)ex2 / p_polygon_area;
          fix32 w1 = (fix32)ex0 / p_polygon_area;
          fix32 w2 = (fix32)ex1 / p_polygon_area;
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
  };

  static void swap_polygon_winding(screen_polygon *p_polygon,
                                   polygon_vertex_indices *p_polygon_indices,
                                   screen_polygon_area *p_area) {
    {
      auto l_tmp = p_polygon->p1();
      p_polygon->p1() = p_polygon->p0();
      p_polygon->p0() = l_tmp;
    }
    {
      auto l_tmp = p_polygon_indices->p1();
      p_polygon_indices->p1() = p_polygon_indices->p0();
      p_polygon_indices->p0() = l_tmp;
    }

    *p_area = *p_area * -1;
  };

private:
  static screen_polygon_area
  __ey_calculation(const pixel_coordinates &p_pixel,
                   const pixel_coordinates &p_polygon_point,
                   const pixel_coordinates &p_delta) {
    return (screen_polygon_area(p_pixel.x() - p_polygon_point.x()) *
            p_delta.y()) -
           (screen_polygon_area(p_pixel.y() - p_polygon_point.y()) *
            p_delta.x());
  };
};

using per_vertices_t =
    orm::table_span_v2<pixel_coordinates, homogeneous_coordinates>;

using per_polygons_t =
    orm::table_span_v2<screen_polygon, polygon_vertex_indices,
                       screen_polygon_bounding_box, screen_polygon_area>;

using visibility_bool_t = ui8;
using visibility_polygon_index_t = uimax;
using visibility = orm::table_span_v2<visibility_bool_t, rasterization_weight,
                                      visibility_polygon_index_t>;

struct rasterize_heap {

  per_vertices_t m_per_vertices;
  per_polygons_t m_per_polygons;

  container::multi_byte_buffer m_vertex_output;
  container::span<ui8 *> m_vertex_output_send_to_vertex_shader;

  visibility m_visibility_buffer;
  visibility m_rasterizationrect_visibility_buffer;

  struct vertex_output_layout {

    struct layout {
      ui16 m_element_size;
      bgfx::AttribType::Enum m_attrib_type;
      ui8 m_attrib_element_count;
    };

    container::span<layout> m_layout;
    ui8 m_col_count;
  } m_vertex_output_layout;

  container::multi_byte_buffer m_vertex_output_interpolated;
  container::span<ui8 *> m_vertex_output_interpolated_send_to_fragment_shader;

  void allocate() {
    m_per_vertices.allocate(0);
    m_per_polygons.allocate(0);
    m_visibility_buffer.allocate(0);
    m_rasterizationrect_visibility_buffer.allocate(0);
    m_vertex_output.allocate();
    m_vertex_output_interpolated.allocate();

    m_vertex_output_send_to_vertex_shader.allocate(128);
    m_vertex_output_interpolated_send_to_fragment_shader.allocate(128);
    m_vertex_output_layout.m_layout.allocate(128);
  };

  void free() {
    m_per_vertices.free();
    m_per_polygons.free();
    m_visibility_buffer.free();
    m_rasterizationrect_visibility_buffer.free();
    m_vertex_output.free();
    m_vertex_output_interpolated.free();

    m_vertex_output_send_to_vertex_shader.free();
    m_vertex_output_interpolated_send_to_fragment_shader.free();
    m_vertex_output_layout.m_layout.free();
  };

  pixel_coordinates &get_pixel_coordinates(ui32 p_index) {
    pixel_coordinates *l_pixel_coordinate;
    m_per_vertices.at(p_index, &l_pixel_coordinate, none());
    return *l_pixel_coordinate;
  };

  homogeneous_coordinates &get_vertex_homogenous(ui32 p_index) {
    homogeneous_coordinates *l_homogeneous_coordinates;
    m_per_vertices.at(p_index, none(), &l_homogeneous_coordinates);
    return *l_homogeneous_coordinates;
  };
};

struct rasterize_unit {

  struct input {
    const program &m_program;
    m::rect_point_extend<ui16> &m_rect;
    const m::mat<fix32, 4, 4> &m_proj;
    const m::mat<fix32, 4, 4> &m_view;
    const m::mat<fix32, 4, 4> &m_transform;
    index_buffer_const_view m_index_buffer;
    bgfx::VertexLayout m_vertex_layout;
    const container::range<ui8> &m_vertex_buffer;
    program_uniforms &m_vertex_uniforms;
    program_uniforms &m_fragment_uniforms;
    ui64 m_state;
    ui32 m_rgba;

    image_view m_target_image_view;
    image_view m_target_depth_view;

    input(const program &p_program, m::rect_point_extend<ui16> &p_rect,
          const m::mat<fix32, 4, 4> &p_proj, const m::mat<fix32, 4, 4> &p_view,
          const m::mat<fix32, 4, 4> &p_transform,
          const container::range<ui8> &p_index_buffer,
          bgfx::VertexLayout p_vertex_layout,
          const container::range<ui8> &p_vertex_buffer,
          program_uniforms &p_vertex_uniforms,
          program_uniforms &p_fragment_uniforms, ui64 p_state, ui32 p_rgba,
          const bgfx::TextureInfo &p_target_info,
          container::range<ui8> &p_target_buffer,
          const bgfx::TextureInfo &p_depth_info,
          container::range<ui8> &p_depth_buffer)
        : m_program(p_program), m_rect(p_rect), m_proj(p_proj), m_view(p_view),
          m_transform(p_transform), m_index_buffer(p_index_buffer),
          m_vertex_layout(p_vertex_layout), m_vertex_buffer(p_vertex_buffer),
          m_vertex_uniforms(p_vertex_uniforms),
          m_fragment_uniforms(p_fragment_uniforms), m_state(p_state),
          m_rgba(p_rgba),
          m_target_image_view(p_target_info.width, p_target_info.height,
                              p_target_info.bitsPerPixel, p_target_buffer),
          m_target_depth_view(p_depth_info.width, p_depth_info.height,
                              p_depth_info.bitsPerPixel, p_depth_buffer){};

  } m_input;

  rasterize_heap &m_heap;
  render_state m_state;
  m::mat<fix32, 4, 4> m_local_to_unit;
  ui16 m_vertex_stride;
  uimax m_vertex_count;

  uimax m_polygon_count;

  m::rect_min_max<ui16> m_rendered_rect;

  rasterize_unit(rasterize_heap &p_heap, const program &p_program,
                 m::rect_point_extend<ui16> &p_rect,
                 const m::mat<fix32, 4, 4> &p_proj,
                 const m::mat<fix32, 4, 4> &p_view,
                 const m::mat<fix32, 4, 4> &p_transform,
                 const container::range<ui8> &p_index_buffer,
                 bgfx::VertexLayout p_vertex_layout,
                 const container::range<ui8> &p_vertex_buffer,
                 program_uniforms &p_vertex_uniforms,
                 program_uniforms &p_fragment_uniforms, ui64 p_state,
                 ui32 p_rgba, const bgfx::TextureInfo &p_target_info,
                 container::range<ui8> &p_target_buffer,
                 const bgfx::TextureInfo &p_depth_info,
                 container::range<ui8> &p_depth_buffer)
      : m_input(p_program, p_rect, p_proj, p_view, p_transform, p_index_buffer,
                p_vertex_layout, p_vertex_buffer, p_vertex_uniforms,
                p_fragment_uniforms, p_state, p_rgba, p_target_info,
                p_target_buffer, p_depth_info, p_depth_buffer),
        m_heap(p_heap){};

  void rasterize() {
    m_state = render_state::from_int(m_input.m_state);
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

    __initialize_layouts();
    __resize_buffers();
    __vertex_v2();

    __extract_polygons();
    __initialize_rendered_rect();

    // TODO -> should apply z clipping

    __calculate_visibility_buffer();

    __interpolate_vertex_output();
    __fragment();

    __terminate();
  };

private:
  void __terminate(){};

  void __initialize_layouts() {

    assert_debug(m_input.m_program.m_vertex);
    auto l_shader_view =
        rast::shader_vertex_bytes::view{(ui8 *)m_input.m_program.m_vertex};
    auto l_output_parameters = l_shader_view.output_parameters();

    uimax l_vertex_output_col_count = l_output_parameters.count();
    m_heap.m_vertex_output_layout.m_col_count = l_output_parameters.count();

    for (auto l_col_it = 0;
         l_col_it < m_heap.m_vertex_output_layout.m_col_count; l_col_it++) {
      const auto &l_input_meta = l_output_parameters.at(l_col_it);
      auto &l_layout = m_heap.m_vertex_output_layout.m_layout.at(l_col_it);
      l_layout.m_element_size = l_input_meta.m_single_element_size;
      l_layout.m_attrib_type = l_input_meta.m_attrib_type;
      l_layout.m_attrib_element_count = l_input_meta.m_attrib_element_count;
    }
  };

  void __resize_buffers() {

    m_heap.m_vertex_output.resize_col_capacity(
        m_heap.m_vertex_output_layout.m_col_count);

    for (auto l_col_it = 0;
         l_col_it < m_heap.m_vertex_output_layout.m_col_count; l_col_it++) {
      m_heap.m_vertex_output.col(l_col_it).resize(
          m_vertex_count,
          m_heap.m_vertex_output_layout.m_layout.at(l_col_it).m_element_size);
    }

    m_heap.m_per_polygons.resize(m_polygon_count);

    m_heap.m_visibility_buffer.resize(
        m_input.m_target_image_view.pixel_count());
    m_heap.m_rasterizationrect_visibility_buffer.resize(
        m_input.m_target_image_view.pixel_count());

    m_heap.m_vertex_output_interpolated.resize_col_capacity(
        m_heap.m_vertex_output_layout.m_col_count);

    for (auto l_col_it = 0;
         l_col_it < m_heap.m_vertex_output_layout.m_col_count; ++l_col_it) {
      m_heap.m_vertex_output_interpolated.col(l_col_it).resize(
          m_input.m_target_image_view.pixel_count(),
          m_heap.m_vertex_output_layout.m_layout.at(l_col_it).m_element_size);
    }

    m_heap.m_vertex_output_interpolated_send_to_fragment_shader.resize(
        m_heap.m_vertex_output_layout.m_col_count);
  };

  void __vertex_v2() {
    m_heap.m_per_vertices.resize(m_vertex_count);

    const shader_vertex_runtime_ctx l_ctx = shader_vertex_runtime_ctx(
        m_input.m_proj, m_input.m_view, m_input.m_transform, m_local_to_unit,
        m_input.m_vertex_layout);

    assert_debug(m_input.m_program.m_vertex);
    auto l_shader_view =
        rast::shader_vertex_bytes::view{(ui8 *)m_input.m_program.m_vertex};
    auto l_output_parameters = l_shader_view.output_parameters();
    shader_vertex_function l_vertex_function = l_shader_view.function();

    for (auto i = 0; i < m_vertex_count; ++i) {
      ui8 *l_vertex_bytes =
          m_input.m_vertex_buffer.m_begin + (i * m_vertex_stride);

      for (auto l_output_it = 0;
           l_output_it < m_heap.m_vertex_output_layout.m_col_count;
           ++l_output_it) {
        m_heap.m_vertex_output_send_to_vertex_shader.at(l_output_it) =
            m_heap.m_vertex_output.at(l_output_it, i);
      }

      m::vec<fix32, 4> l_vertex_shader_out;
      l_vertex_function(l_ctx, l_vertex_bytes,
                        (ui8 **)m_input.m_vertex_uniforms.data(),
                        l_vertex_shader_out,
                        m_heap.m_vertex_output_send_to_vertex_shader.m_data);

      l_vertex_shader_out = l_vertex_shader_out / l_vertex_shader_out.w();

      // [-1, 1] to [0, 1] range
      l_vertex_shader_out = (l_vertex_shader_out + 1) * 0.5;

#if TODO_NEAR_FAR_CLIPPING
      if (l_vertex_shader_out.z() > 1.0f || l_vertex_shader_out.z() < 0.0f) {
      }
#endif

      uv_t l_pixel_coordinates_fix32 = uv_t::make(l_vertex_shader_out);
      l_pixel_coordinates_fix32 *= (m_input.m_rect.extend() - 1);

      auto l_pixel_coordinate_i16 = l_pixel_coordinates_fix32.cast<i16>();
      m_heap.get_pixel_coordinates(i) = l_pixel_coordinate_i16;

      if (m_state.m_depth_read) {
        m_heap.m_per_vertices.set(
            i, none(), homogeneous_coordinates::make(l_vertex_shader_out));
      }
    }
  };

  void __extract_polygons() {

    switch (m_state.m_cull_mode) {
    case CullMode::Clockwise:
      __extract_polygons_internal<CullMode::Clockwise>();
      break;
    case CullMode::CounterClockwise:
      __extract_polygons_internal<CullMode::CounterClockwise>();
      break;
    case CullMode::None:
      __extract_polygons_internal<CullMode::None>();
      break;
    }
  };

  template <CullMode CullModeValue> void __extract_polygons_internal() {
    uimax l_index_idx = 0;
    for (auto i = 0; i < m_polygon_count; ++i) {
      polygon_vertex_indices *l_polygon_indices;
      screen_polygon *l_polygon;
      screen_polygon_bounding_box *l_bounding_rect;
      screen_polygon_area *l_area;

      m_heap.m_per_polygons.at(i, &l_polygon, &l_polygon_indices,
                               &l_bounding_rect, &l_area);

      l_polygon_indices->p0() =
          m_input.m_index_buffer.at<vindex_t>(l_index_idx);
      l_polygon_indices->p1() =
          m_input.m_index_buffer.at<vindex_t>(l_index_idx + 1);
      l_polygon_indices->p2() =
          m_input.m_index_buffer.at<vindex_t>(l_index_idx + 2);

      l_polygon->p0() = m_heap.get_pixel_coordinates(l_polygon_indices->p0());
      l_polygon->p1() = m_heap.get_pixel_coordinates(l_polygon_indices->p1());
      l_polygon->p2() = m_heap.get_pixel_coordinates(l_polygon_indices->p2());

      *l_area = m::cross(
          (l_polygon->p2() - l_polygon->p0()).cast<screen_polygon_area>(),
          (l_polygon->p1() - l_polygon->p0()).cast<screen_polygon_area>());

      if constexpr (CullModeValue == CullMode::Clockwise) {
        if (*l_area >= 0) {
          i -= 1;
          m_polygon_count -= 1;
          goto next;
        } else {
          utils::swap_polygon_winding(l_polygon, l_polygon_indices, l_area);
        }
      } else if constexpr (CullModeValue == CullMode::CounterClockwise) {
        if (*l_area <= 0) {
          i -= 1;
          m_polygon_count -= 1;
          goto next;
        }
      } else if constexpr (CullModeValue == CullMode::None) {
        if (*l_area < 0) {
          utils::swap_polygon_winding(l_polygon, l_polygon_indices, l_area);
        } else if (*l_area == 0) {
          i -= 1;
          m_polygon_count -= 1;
          goto next;
        }
      }

      *l_bounding_rect = m::bounding_rect(*l_polygon);
      l_bounding_rect->max() = l_bounding_rect->max() + 1;
      *l_bounding_rect = m::fit_into(*l_bounding_rect, m_input.m_rect);

      assert_debug(l_bounding_rect->is_valid());
      assert_debug(l_bounding_rect->max().x() <=
                   m_input.m_target_image_view.m_width);
      assert_debug(l_bounding_rect->max().y() <=
                   m_input.m_target_image_view.m_height);

    next:
      l_index_idx += 3;
    }
  };

  void __initialize_rendered_rect() {
    if (m_polygon_count == 0) {
      m_rendered_rect.min() = {0, 0};
      m_rendered_rect.max() = {0, 0};
    } else {
      container::range<screen_polygon_bounding_box> l_polygon_rects =
          container::range<screen_polygon_bounding_box>::make(
              m_heap.m_per_polygons.cols().m_col_2.m_data, m_polygon_count);
      m::rect_min_max<screen_coord_t> l_rendered_rect =
          m::bounding_rect(l_polygon_rects);

      m_rendered_rect.min() = l_rendered_rect.min().cast<ui16>();
      m_rendered_rect.max() = l_rendered_rect.max().cast<ui16>();
    }

    assert_debug(m_rendered_rect.is_valid());
    assert_debug(m_rendered_rect.max().x() <=
                 m_input.m_target_image_view.m_width);
    assert_debug(m_rendered_rect.max().y() <=
                 m_input.m_target_image_view.m_height);
  };

  void __calculate_visibility_buffer() {
    container::range<ui8> l_visibility_range;
    m_heap.m_visibility_buffer.range(&l_visibility_range, none(), none());
    l_visibility_range =
        l_visibility_range.shrink_to(m_input.m_target_image_view.pixel_count());
    l_visibility_range.zero();

    for (auto l_polygon_it = 0; l_polygon_it < m_polygon_count;
         ++l_polygon_it) {
      screen_polygon *l_polygon;
      polygon_vertex_indices *l_polygon_indices;
      screen_polygon_bounding_box *l_bounding_rect;
      screen_polygon_area *l_area;
      m_heap.m_per_polygons.at(l_polygon_it, &l_polygon, &l_polygon_indices,
                               &l_bounding_rect, &l_area);

      // Resetting the bouding rect visibility buffer
      container::range<ui8> l_boudingrect_visibility_range;
      m_heap.m_rasterizationrect_visibility_buffer.range(
          &l_boudingrect_visibility_range, none(), none());
      l_boudingrect_visibility_range = l_boudingrect_visibility_range.shrink_to(
          m_input.m_target_image_view.pixel_count());

      pixel_coordinates l_rasterization_rect_offset = l_bounding_rect->min();
      pixel_coordinates l_rasterization_rect_dimensions =
          (l_bounding_rect->max() - l_bounding_rect->min());

      l_boudingrect_visibility_range = l_boudingrect_visibility_range.shrink_to(
          l_rasterization_rect_dimensions.x() *
          l_rasterization_rect_dimensions.y());
      l_boudingrect_visibility_range.zero();

      utils::rasterize_polygon_weighted(
          *l_polygon, *l_area, *l_bounding_rect,
          [&](screen_coord_t x, screen_coord_t y, fix32 w0, fix32 w1,
              fix32 w2) {
            ui8 *l_visibility_boolean;
            rasterization_weight *l_visibility_weight;
            uimax *l_polygon_index;

            pixel_coordinates l_point = {x, y};
            assert_debug(l_point.x() >= l_bounding_rect->min().x());
            assert_debug(l_point.y() >= l_bounding_rect->min().y());
            l_point -= l_bounding_rect->min();

            auto l_boudingrect_visibility_index =
                (l_point.y() * l_rasterization_rect_dimensions.x()) +
                l_point.x();

            assert_debug(l_boudingrect_visibility_index <
                         l_boudingrect_visibility_range.count());

            m_heap.m_rasterizationrect_visibility_buffer.at(
                l_boudingrect_visibility_index, &l_visibility_boolean,
                &l_visibility_weight, &l_polygon_index);

            rasterization_weight l_weight = {w0, w1, w2};

            *l_visibility_boolean = 1;
            *l_visibility_weight = l_weight;
            *l_polygon_index = l_polygon_it;
          });

      if (m_state.m_depth_read) {
        m::polygon<fix32, 3> l_attribute_polygon;

        l_attribute_polygon.p0() =
            m_heap.get_vertex_homogenous(l_polygon_indices->p0()).z();
        l_attribute_polygon.p1() =
            m_heap.get_vertex_homogenous(l_polygon_indices->p1()).z();
        l_attribute_polygon.p2() =
            m_heap.get_vertex_homogenous(l_polygon_indices->p2()).z();

        if (m_state.m_depth_write) {
          __for_each_bounding_rasterized_pixels_v2(
              l_rasterization_rect_dimensions, l_bounding_rect->min(),
              [&](ui8 *l_boundingrect_visibility_boolean,
                  rasterization_weight *l_boundingrect_visibility_weight,
                  uimax *l_boundingrect_polygon_index,
                  uimax l_visibility_index) {
                visibility_bool_t *l_visibility_boolean;
                rasterization_weight *l_visibility_weight;
                visibility_polygon_index_t *l_polygon_index;
                m_heap.m_visibility_buffer.at(
                    l_visibility_index, &l_visibility_boolean,
                    &l_visibility_weight, &l_polygon_index);

                fix32 l_interpolated_depth = m::interpolate(
                    l_attribute_polygon, *l_boundingrect_visibility_weight);
                fix32 *l_buffer_depth =
                    (fix32 *)m_input.m_target_depth_view.at(l_visibility_index);

                if (l_interpolated_depth < *l_buffer_depth) {
                  *l_visibility_boolean = 1;
                  *l_visibility_weight = *l_boundingrect_visibility_weight;
                  *l_polygon_index = *l_boundingrect_polygon_index;

                  // Write to buffer
                  *l_buffer_depth = l_interpolated_depth;
                }
              });
        } else {
          __for_each_bounding_rasterized_pixels_v2(
              l_rasterization_rect_dimensions, l_bounding_rect->min(),
              [&](ui8 *l_boundingrect_visibility_boolean,
                  rasterization_weight *l_boundingrect_visibility_weight,
                  uimax *l_boundingrect_polygon_index,
                  uimax l_visibility_index) {
                visibility_bool_t *l_visibility_boolean;
                rasterization_weight *l_visibility_weight;
                visibility_polygon_index_t *l_polygon_index;
                m_heap.m_visibility_buffer.at(
                    l_visibility_index, &l_visibility_boolean,
                    &l_visibility_weight, &l_polygon_index);

                fix32 l_interpolated_depth = m::interpolate(
                    l_attribute_polygon, *l_boundingrect_visibility_weight);
                fix32 *l_buffer_depth =
                    (fix32 *)m_input.m_target_depth_view.at(l_visibility_index);

                if (l_interpolated_depth < *l_buffer_depth) {
                  *l_visibility_boolean = 1;
                  *l_visibility_weight = *l_boundingrect_visibility_weight;
                  *l_polygon_index = *l_boundingrect_polygon_index;
                }
              });
        }
      } else {
        __for_each_bounding_rasterized_pixels_v2(
            l_rasterization_rect_dimensions, l_bounding_rect->min(),
            [&](ui8 *l_boundingrect_visibility_boolean,
                rasterization_weight *l_boundingrect_visibility_weight,
                uimax *l_boundingrect_polygon_index, uimax l_visibility_index) {
              visibility_bool_t *l_visibility_boolean;
              rasterization_weight *l_visibility_weight;
              visibility_polygon_index_t *l_polygon_index;
              m_heap.m_visibility_buffer.at(
                  l_visibility_index, &l_visibility_boolean,
                  &l_visibility_weight, &l_polygon_index);

              *l_visibility_boolean = 1;
              *l_visibility_weight = *l_boundingrect_visibility_weight;
              *l_polygon_index = *l_boundingrect_polygon_index;
            });
      }
    }
  };

  template <typename CallbackFunc>
  void __for_each_bounding_rasterized_pixels_v2(
      const pixel_coordinates &p_bounding_rect_extend,
      const pixel_coordinates &p_bounding_rect_offset,
      const CallbackFunc &p_callback) {

    for (auto y = 0; y < p_bounding_rect_extend.y(); ++y) {
      for (auto x = 0; x < p_bounding_rect_extend.x(); ++x) {
        auto l_boudingrect_visibility_index =
            (y * p_bounding_rect_extend.x()) + x;

        visibility_bool_t *l_boundingrect_visibility_boolean;
        rasterization_weight *l_boundingrect_visibility_weight;
        visibility_polygon_index_t *l_boundingrect_polygon_index;

        m_heap.m_rasterizationrect_visibility_buffer.at(
            l_boudingrect_visibility_index, &l_boundingrect_visibility_boolean,
            &l_boundingrect_visibility_weight, &l_boundingrect_polygon_index);

        if (*l_boundingrect_visibility_boolean) {

          auto l_visibility_index = ((y + p_bounding_rect_offset.y()) *
                                     m_input.m_target_image_view.m_width) +
                                    (x + p_bounding_rect_offset.x());

          p_callback(l_boundingrect_visibility_boolean,
                     l_boundingrect_visibility_weight,
                     l_boundingrect_polygon_index, l_visibility_index);
        }
      }
    }
  };

  void __interpolate_vertex_output() {
    __interpolate_vertex_output_range(
        0, m_heap.m_vertex_output_layout.m_col_count);
  };

  void __fragment() {
    assert_debug(m_input.m_program.m_fragment);

    shader_fragment_function l_fragment =
        shader_fragment_bytes::view{(ui8 *)m_input.m_program.m_fragment}
            .fonction();

    rgbf_t l_color_buffer;

    __for_each_rendered_pixels<1>(
        [&](uimax p_pixel_index, const pixel_coordinates &p_pixel_coordinates) {
          visibility_bool_t *l_visibility_boolean;
          m_heap.m_visibility_buffer.at(p_pixel_index, &l_visibility_boolean,
                                        none(), none());
          if (*l_visibility_boolean) {

            for (auto j = 0;
                 j < m_heap.m_vertex_output_interpolated.m_col_count; ++j) {
              m_heap.m_vertex_output_interpolated_send_to_fragment_shader.at(
                  j) = m_heap.m_vertex_output_interpolated.at(j, p_pixel_index);
            }

            l_fragment(
                p_pixel_coordinates,
                m_heap.m_vertex_output_interpolated_send_to_fragment_shader
                    .m_data,
                (ui8 **)m_input.m_fragment_uniforms.data(), l_color_buffer);

            rgb_t l_color = (l_color_buffer * 255).cast<ui8>();
            m_input.m_target_image_view.set_pixel(p_pixel_index, l_color);
          }
        });
  };

  template <ui8 GetCoords = 0, typename CallbackFunc>
  void __for_each_rendered_pixels(const CallbackFunc &p_callback) {
    for (auto y = m_rendered_rect.min().y(); y < m_rendered_rect.max().y();
         ++y) {
      for (auto x = m_rendered_rect.min().x(); x < m_rendered_rect.max().x();
           ++x) {
        auto l_pixel_index = (m_input.m_target_image_view.m_width * y) + x;
        if constexpr (GetCoords) {
          pixel_coordinates l_pixel_coordinates = {screen_coord_t(x),
                                                   screen_coord_t(y)};
          p_callback(l_pixel_index, l_pixel_coordinates);
        } else {
          p_callback(l_pixel_index);
        }
      }
    }
  };

  void __interpolate_vertex_output_range(ui8 p_begin_index, ui8 p_end_index) {
    visibility_bool_t *l_visibility_boolean;
    rasterization_weight *l_visibility_weight;
    visibility_polygon_index_t *l_visibility_polygon;

    __for_each_rendered_pixels([&](uimax p_pixel_index) {
      m_heap.m_visibility_buffer.at(p_pixel_index, &l_visibility_boolean,
                                    &l_visibility_weight,
                                    &l_visibility_polygon);
      if (*l_visibility_boolean) {
        polygon_vertex_indices *l_indices_polygon;
        m_heap.m_per_polygons.at(*l_visibility_polygon, none(),
                                 &l_indices_polygon);

        for (auto l_vertex_output_index = p_begin_index;
             l_vertex_output_index < p_end_index; ++l_vertex_output_index) {
          __interpolate_vertex_output_single_value(
              m_heap.m_vertex_output_layout.m_layout.range(),
              l_vertex_output_index, *l_indices_polygon, *l_visibility_weight,
              p_pixel_index);
        }
      }
    });
  };

  void __interpolate_vertex_output_single_value(
      const container::range<rasterize_heap::vertex_output_layout::layout>
          &p_vertex_shader_outputs_meta,
      uimax p_vertex_output_index,
      const polygon_vertex_indices &p_indices_polygon,
      const rasterization_weight &p_polygon_weight, uimax p_pixel_index) {

    const rasterize_heap::vertex_output_layout::layout
        &l_output_parameter_meta =
            p_vertex_shader_outputs_meta.at(p_vertex_output_index);

    void *l_0 = m_heap.m_vertex_output.at(p_vertex_output_index,
                                          p_indices_polygon.p0());
    void *l_1 = m_heap.m_vertex_output.at(p_vertex_output_index,
                                          p_indices_polygon.p1());
    void *l_2 = m_heap.m_vertex_output.at(p_vertex_output_index,
                                          p_indices_polygon.p2());

    if (l_output_parameter_meta.m_attrib_type == bgfx::AttribType::Float) {
      if (l_output_parameter_meta.m_attrib_element_count == 3) {
        m::vec<fix32, 3> *l_interpolated_vertex_output =
            (m::vec<fix32, 3> *)m_heap.m_vertex_output_interpolated.at(
                p_vertex_output_index, p_pixel_index);

        *l_interpolated_vertex_output =
            __interpolate(p_polygon_weight, *(m::vec<fix32, 3> *)l_0,
                          *(m::vec<fix32, 3> *)l_1, *(m::vec<fix32, 3> *)l_2);

      } else if (l_output_parameter_meta.m_attrib_element_count == 2) {
        m::vec<fix32, 2> *l_interpolated_vertex_output =
            (m::vec<fix32, 2> *)m_heap.m_vertex_output_interpolated.at(
                p_vertex_output_index, p_pixel_index);
        *l_interpolated_vertex_output =
            __interpolate(p_polygon_weight, *(m::vec<fix32, 2> *)l_0,
                          *(m::vec<fix32, 2> *)l_1, *(m::vec<fix32, 2> *)l_2);
      } else if (l_output_parameter_meta.m_attrib_element_count == 1) {
        fix32 *l_interpolated_vertex_output =
            (fix32 *)m_heap.m_vertex_output_interpolated.at(
                p_vertex_output_index, p_pixel_index);

        *l_interpolated_vertex_output = __interpolate(
            p_polygon_weight, *(fix32 *)l_0, *(fix32 *)l_1, *(fix32 *)l_2);
      };
    }
  };

  template <typename T>
  T __interpolate(const rasterization_weight &p_weights, const T &p_0,
                  const T &p_1, const T &p_2) {
    m::polygon<T, 3> l_attribute_polygon;

    l_attribute_polygon.p0() = p_0;
    l_attribute_polygon.p1() = p_1;
    l_attribute_polygon.p2() = p_2;

    return m::interpolate(l_attribute_polygon, p_weights);
  };
};

} // namespace algorithm
} // namespace rast

#undef TODO_NEAR_FAR_CLIPPING