#pragma once

#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <m/mat.hpp>
#include <m/rect.hpp>
#include <m/vec.hpp>

namespace rast {
namespace algorithm {

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

struct screen_polygon {
  m::vec<i16, 2> m_0, m_1, m_2;

  m::rect_min_max<i16> calculate_bounding_rect() {
    container::range<m::vec<i16, 2>> l_points;
    l_points.m_begin = &m_0;
    l_points.m_count = 3;
    return m::rect_min_max<i16>::bounding_box(l_points);
  };

  i32 edge_0(const m::vec<i16, 2> &p) {
    return (m_0.at(0) - m_1.at(0)) * (p.at(1) - m_0.at(1)) -
           (m_0.at(1) - m_1.at(1)) * (p.at(0) - m_0.at(0));
  };

  i32 edge_1(const m::vec<i16, 2> &p) {
    return (m_1.at(0) - m_2.at(0)) * (p.at(1) - m_1.at(2)) -
           (m_1.at(1) - m_2.at(1)) * (p.at(0) - m_1.at(0));
  };

  i32 edge_2(const m::vec<i16, 2> &p) {
    return (m_2.at(0) - m_0.at(0)) * (p.at(1) - m_2.at(1)) -
           (m_2.at(1) - m_0.at(1)) * (p.at(0) - m_2.at(0));
  };
};

struct utils {
  static void rasterize_polygon_to_visiblity(
      const screen_polygon &p_polygon, m::rect_min_max<i16> &p_bounding_rect,
      container::span<ui8> &p_visibility, ui32 p_width) {

    m::vec<i16, 2> l_pixel = {p_bounding_rect.min().x(),
                              p_bounding_rect.min().y()};

    const m::vec<i16, 2> d0 = p_polygon.m_0 - p_polygon.m_2;
    const m::vec<i16, 2> d1 = p_polygon.m_1 - p_polygon.m_0;
    const m::vec<i16, 2> d2 = p_polygon.m_2 - p_polygon.m_1;
    i32 ey0 = ((l_pixel.x() - p_polygon.m_0.x()) * d0.y()) -
              ((l_pixel.y() - p_polygon.m_0.y()) * d0.x());
    i32 ey1 = ((l_pixel.x() - p_polygon.m_1.x()) * d1.y()) -
              ((l_pixel.y() - p_polygon.m_1.y()) * d1.x());
    i32 ey2 = ((l_pixel.x() - p_polygon.m_2.x()) * d2.y()) -
              ((l_pixel.y() - p_polygon.m_2.y()) * d2.x());

    for (auto y = p_bounding_rect.min().y(); y <= p_bounding_rect.max().y();
         ++y) {
      i32 ex0 = ey0;
      i32 ex1 = ey1;
      i32 ex2 = ey2;

      for (auto x = p_bounding_rect.min().x(); x <= p_bounding_rect.max().x();
           ++x) {

        if (ex0 >= 0 && ex1 >= 0 && ex2 >= 0) {
          p_visibility.at((y * p_width) + x) = 1;
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

  ui16 m_vertex_stride;
  uimax m_vertex_count;
  uimax m_polygon_count;
  container::span<m::vec<i16, 2>> m_screen_vertices;

  container::span<screen_polygon> m_screen_polygons;

  container::span<ui8> m_visibility_buffer;

  rasterize_unit(const program &p_program, m::rect_point_extend<ui16> &p_rect,
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
                p_target_info, p_target_buffer){};

  void rasterize() {

    m_vertex_stride = m_input.m_vertex_layout.getStride();
    m_vertex_count = m_input.m_vertex_buffer.count() / m_vertex_stride;
    assert_debug(m_input.m_vertex_layout.getSize(m_vertex_count) ==
                 m_input.m_vertex_buffer.count());

    m_polygon_count = m_input.m_index_buffer.m_index_count / 3;

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

    __calculate_screen_vertices();
    __extract_screen_polygons();
    __calculate_visibility_buffer();
    __fragment();

    __free();
  };

private:
  void __free() {
    m_screen_vertices.free();
    m_screen_polygons.free();
    m_visibility_buffer.free();
  };

  void __calculate_screen_vertices() {

    m::mat<f32, 4, 4> l_local_to_unit =
        m_input.m_proj * m_input.m_view * m_input.m_transform;

    m_screen_vertices.allocate(m_vertex_count);

    for (auto i = 0; i < m_vertex_count; ++i) {
      ui8 *l_vertex_bytes =
          m_input.m_vertex_buffer.m_begin + (i * m_vertex_stride);
      m::vec<f32, 3> l_vertex_vec = *(m::vec<f32, 3> *)l_vertex_bytes;
      m::vec<f32, 2> l_vertex_screen_2;
      {
        m::vec<f32, 3> l_vertex_screen = l_local_to_unit * l_vertex_vec;
        l_vertex_screen_2 = m::vec<f32, 2>::make(l_vertex_screen);
      }

      l_vertex_screen_2 = (l_vertex_screen_2 + 1) * 0.5;
      l_vertex_screen_2.y() = 1 - l_vertex_screen_2.y();
      l_vertex_screen_2 *= (m_input.m_rect.extend() - 1);
      m_screen_vertices.at(i) = l_vertex_screen_2.cast<i16>();
    }
  };

  void __extract_screen_polygons() {
    m_screen_polygons.allocate(m_polygon_count);

    uimax l_index_idx = 0;
    for (auto i = 0; i < m_screen_polygons.count(); ++i) {
      screen_polygon &l_polygon = m_screen_polygons.at(i);
      l_polygon.m_0 =
          m_screen_vertices.at(m_input.m_index_buffer.at<ui16>(l_index_idx));
      l_polygon.m_1 = m_screen_vertices.at(
          m_input.m_index_buffer.at<ui16>(l_index_idx + 1));
      l_polygon.m_2 = m_screen_vertices.at(
          m_input.m_index_buffer.at<ui16>(l_index_idx + 2));
      l_index_idx += 3;
    }
  };

  void __calculate_visibility_buffer() {
    m_visibility_buffer.allocate(m_input.m_target_image_view.pixel_count());
    m_visibility_buffer.range().zero();

    for (auto l_polygon_it = 0; l_polygon_it < m_screen_polygons.count();
         ++l_polygon_it) {
      screen_polygon &l_polygon = m_screen_polygons.at(l_polygon_it);

      auto l_bounding_rect = l_polygon.calculate_bounding_rect();

      // TODO -> utility function for this
      if (l_bounding_rect.min().at(0) < m_input.m_rect.point().at(0)) {
        l_bounding_rect.min().at(0) = m_input.m_rect.point().at(0);
      }

      if (l_bounding_rect.min().at(1) < m_input.m_rect.point().at(1)) {
        l_bounding_rect.min().at(1) = m_input.m_rect.point().at(1);
      }

      if (l_bounding_rect.max().at(0) >= m_input.m_rect.extend().at(0)) {
        l_bounding_rect.max().at(0) = m_input.m_rect.extend().at(0) - 1;
      }

      if (l_bounding_rect.max().at(1) >= m_input.m_rect.extend().at(1)) {
        l_bounding_rect.max().at(1) = m_input.m_rect.extend().at(1) - 1;
      }

      utils::rasterize_polygon_to_visiblity(
          l_polygon, l_bounding_rect, m_visibility_buffer,
          m_input.m_target_image_view.m_target_info.width);
    }
  };

  void __fragment() {
    for (auto i = 0; i < m_input.m_target_image_view.pixel_count(); ++i) {
      if (m_visibility_buffer.at(i)) {
        m::vec<ui8, 3> l_color = {255, 255, 255};
        m_input.m_target_image_view.set_pixel(i, l_color);
      }
    }
  };
};

static inline void
rasterize(const program &p_program, m::rect_point_extend<ui16> &p_rect,
          const m::mat<f32, 4, 4> &p_proj, const m::mat<f32, 4, 4> &p_view,
          const m::mat<f32, 4, 4> &p_transform,
          const container::range<ui8> &p_index_buffer,
          bgfx::VertexLayout p_vertex_layout,
          const container::range<ui8> &p_vertex_buffer, ui64 p_state,
          ui32 p_rgba, const bgfx::TextureInfo &p_target_info,
          container::range<ui8> &p_target_buffer) {
  rasterize_unit(p_program, p_rect, p_proj, p_view, p_transform, p_index_buffer,
                 p_vertex_layout, p_vertex_buffer, p_state, p_rgba,
                 p_target_info, p_target_buffer)
      .rasterize();
};

} // namespace algorithm
} // namespace rast
