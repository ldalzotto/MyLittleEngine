
#include <doctest.h>
#include <m/const.hpp>
#include <rast/impl/rast_impl.hpp>
#include <rast/rast.hpp>
#include <tst/test_rasterizer_assets.hpp>

#define WRITE_OUTPUT 0

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

namespace RasterizerTestToolbox {

template <typename Rasterizer>
inline static container::range<rgb_t>
getFrameBuffer(bgfx::FrameBufferHandle p_frame_buffer,
               rast_api<Rasterizer> p_rast) {
  return p_rast.fetchTextureSync(p_rast.getTexture(p_frame_buffer))
      .template cast_to<rgb_t>();
};

template <typename ExpectedFrameType, typename Rasterizer>
inline static void assert_frame_equals(
    const i8 *p_save_path, bgfx::FrameBufferHandle p_frame_buffer,
    const rast::image_view &p_frame_buffer_view,
    const ExpectedFrameType &p_expected_frame, rast_api<Rasterizer> p_rast) {
  container::range<ui8> l_png_frame;
  {
    container::range<ui8> l_frame_texture =
        p_rast.fetchTextureSync(p_rast.getTexture(p_frame_buffer));

#if WRITE_OUTPUT
    stbi_write_png(
        p_save_path, l_frame_texture->info.width, l_frame_texture->info.height,
        3, l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture.data(),
        p_frame_buffer_view.m_bits_per_pixel * p_frame_buffer_view.m_width,
        p_frame_buffer_view.m_width, p_frame_buffer_view.m_height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  REQUIRE(l_png_frame.count() == p_expected_frame.count());
  REQUIRE(l_png_frame.is_contained_by(p_expected_frame.range()));

  STBIW_FREE(l_png_frame.m_begin);
};

template <typename Rasterizer>
inline static bgfx::ProgramHandle
load_program(rast_api<Rasterizer> p_rast,
             const container::range<rast::shader_vertex_output_parameter>
                 &p_vertex_output,
             rast::shader_vertex_function p_vertex,
             rast::shader_fragment_function p_fragment,
             bgfx::ShaderHandle *out_vertex, bgfx::ShaderHandle *out_fragment) {

  uimax l_vertex_shader_size = rast::shader_vertex_bytes::byte_size(1);
  const bgfx::Memory *l_vertex_shader_memory =
      p_rast.alloc(l_vertex_shader_size);
  rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
      p_vertex_output, p_vertex);

  const bgfx::Memory *l_fragment_shader_memory =
      p_rast.alloc(rast::shader_fragment_bytes::byte_size());
  rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
      p_fragment);

  *out_vertex = p_rast.createShader(l_vertex_shader_memory);
  *out_fragment = p_rast.createShader(l_fragment_shader_memory);
  return p_rast.createProgram(*out_vertex, *out_fragment);
};

template <typename Rasterizer>
inline static void loadVertexIndex(rast_api<Rasterizer> p_rast,
                                   const bgfx::VertexLayout &p_vertex_layout,
                                   const container::range<ui8> &p_vertices,
                                   const container::range<ui8> &p_indicex,
                                   bgfx::VertexBufferHandle *out_vertex,
                                   bgfx::IndexBufferHandle *out_index) {
  const bgfx::Memory *l_vertex_memory =
      p_rast.makeRef(p_vertices.data(), p_vertices.count());
  const bgfx::Memory *l_index_memory =
      p_rast.makeRef(p_indicex.data(), p_indicex.count());

  *out_vertex = p_rast.createVertexBuffer(l_vertex_memory, p_vertex_layout);
  *out_index = p_rast.createIndexBuffer(l_index_memory);
};
}; // namespace RasterizerTestToolbox

struct WhiteShader {

  inline static container::arr<rast::shader_vertex_output_parameter, 0>
      s_vertex_output = {};

  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                     const ui8 *p_vertex, m::vec<fix32, 4> &out_screen_position,
                     ui8 **out_vertex) {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    out_screen_position =
        p_ctx.m_local_to_unit * m::vec<fix32, 4>::make(l_vertex_pos, 1);
  };

  static void fragment(ui8 **p_vertex_output_interpolated, rgbf_t &out_color) {
    out_color = {1, 1, 1};
  };

  template <typename Rasterizer>
  inline static bgfx::ProgramHandle
  load_program(rast_api<Rasterizer> p_rast, bgfx::ShaderHandle *out_vertex,
               bgfx::ShaderHandle *out_fragment) {
    return RasterizerTestToolbox::load_program(p_rast, s_vertex_output.range(),
                                               vertex, fragment, out_vertex,
                                               out_fragment);
  };
};

struct ColorInterpolationShader {
  inline static container::arr<rast::shader_vertex_output_parameter, 1>
      s_vertex_output = {
          rast::shader_vertex_output_parameter(bgfx::AttribType::Float, 3)};

  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                     const ui8 *p_vertex, m::vec<fix32, 4> &out_screen_position,
                     ui8 **out_vertex) {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    const rgb_t &l_color =
        l_shader.get_vertex<rgb_t>(bgfx::Attrib::Enum::Color0, p_vertex);
    out_screen_position =
        p_ctx.m_local_to_unit * m::vec<fix32, 4>::make(l_vertex_pos, 1);

    rgbf_t *l_vertex_color = (rgbf_t *)out_vertex[0];
    (*l_vertex_color) = l_color.cast<fix32>() / 255;
  };

  static void fragment(ui8 **p_vertex_output_interpolated, rgbf_t &out_color) {
    rgbf_t *l_vertex_color = (position_t *)p_vertex_output_interpolated[0];
    out_color = *l_vertex_color;
  };

  template <typename Rasterizer>
  inline static bgfx::ProgramHandle
  load_program(rast_api<Rasterizer> p_rast, bgfx::ShaderHandle *out_vertex,
               bgfx::ShaderHandle *out_fragment) {
    return RasterizerTestToolbox::load_program(p_rast, s_vertex_output.range(),
                                               vertex, fragment, out_vertex,
                                               out_fragment);
  };
};

TEST_CASE("rast.single_triangle.visibility") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin();
  l_vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

  container::arr<position_t, 3> l_triangle_vertices = {
      .m_data = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}}};
  container::arr<vindex_t, 3> l_triangle_indices = {0, 1, 2};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_frame_buffer_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(l_width, l_height, l_frame_buffer_format);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      WhiteShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);
  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(0);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.visibility.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_single_triangle_visibility(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_rast.shutdown();
}

TEST_CASE("rast.single_triangle.vertex_color_interpolation") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  // l_vertex_layout.getStride();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3));

  {
    l_triangle_vertices.range()
        .stream(position_t{0.0, 0.0, 0.0})
        .stream(rgb_t{0, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(position_t{1.0, 0.0, 0.0})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(position_t{0.0, 1.0, 0.0})
        .stream(rgb_t{255, 255, 0});
  }

  container::arr<vindex_t, 3> l_triangle_indices = {0, 1, 2};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_frame_buffer_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(l_width, l_height, l_frame_buffer_format);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      ColorInterpolationShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);

  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(0);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.vertex_color_interpolation.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_single_triangle_vertex_color_interpolation(),
      l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_triangle_vertices.free();

  l_rast.shutdown();
}

TEST_CASE("rast.cull.clockwise.counterclockwise") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin();
  l_vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

  container::arr<position_t, 6> l_triangle_vertices = {
      .m_data = {
          {0.0, 0.0, 0.0},
          {1.0, 0.0, 0.0},
          {0.0, 1.0, 0.0}, // ccw
          {0.0, 0.0, 0.0},
          {0.0, -1.0, 0.0},
          {-1.0, 0.0, 0.0} // cc
      }};
  container::arr<vindex_t, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_frame_buffer_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(l_width, l_height, l_frame_buffer_format);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      WhiteShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);
  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(BGFX_STATE_CULL_CW);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.cull.clockwise.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_cull_clockwise(), l_rast);

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);
  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(BGFX_STATE_CULL_CCW);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.cull.counterclockwise.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_cull_counterclockwise(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_rast.shutdown();
}

TEST_CASE("rast.depth.comparison") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(position_t{0.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(position_t{1.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(position_t{0.0, 1.0, 0.0})
        .stream(rgb_t{255, 0, 0});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(position_t{-0.5, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(position_t{1.0, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(position_t{0.0, 1.0, 0.1})
        .stream(rgb_t{0, 255, 0});
  }

  container::arr<vindex_t, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_frame_buffer_rgb_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(0, l_width, l_height, l_frame_buffer_rgb_format,
                               bgfx::TextureFormat::D32F);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_rgb_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      ColorInterpolationShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);

  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_depth_comparison(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_triangle_vertices.free();

  l_rast.shutdown();
}

TEST_CASE("rast.depth.comparison.large_framebuffer") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(position_t{0.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(position_t{1.0, 0.0, 0.0})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(position_t{0.0, 1.0, 0.0})
        .stream(rgb_t{0, 0, 255});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(position_t{-0.5, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(position_t{1.0, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(position_t{0.0, 1.0, 0.1})
        .stream(rgb_t{0, 255, 0});
  }

  container::arr<vindex_t, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 400, l_height = 400;
  auto l_frame_buffer_rgb_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(0, l_width, l_height, l_frame_buffer_rgb_format,
                               bgfx::TextureFormat::D32F);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_rgb_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      ColorInterpolationShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);

  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.large_framebuffer.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_depth_comparison_large_framebuffer(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_triangle_vertices.free();

  l_rast.shutdown();
}

TEST_CASE("rast.depth.comparison.readonly") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(position_t{0.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(position_t{1.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(position_t{0.0, 1.0, 0.0})
        .stream(rgb_t{255, 0, 0});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(position_t{-0.5, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(position_t{1.0, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(position_t{0.0, 1.0, 0.1})
        .stream(rgb_t{0, 255, 0});
  }

  container::arr<vindex_t, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_frame_buffer_rgb_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(0, l_width, l_height, l_frame_buffer_rgb_format,
                               bgfx::TextureFormat::D32F);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_rgb_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      ColorInterpolationShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);

  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(BGFX_STATE_DEPTH_TEST_LESS);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.readonly.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_depth_comparison_readonly(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_triangle_vertices.free();

  l_rast.shutdown();
}

// top left-right out of bounds
TEST_CASE("rast.depth.comparison.outofbounds") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(position_t{0.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(position_t{2.0, 0.0, 0.0})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(position_t{0.0, 2.0, 0.0})
        .stream(rgb_t{255, 0, 0});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(position_t{-2, -2, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(position_t{1.0, 0.0, 0.1})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(position_t{0.0, 1.0, 0.1})
        .stream(rgb_t{0, 255, 0});
  }

  container::arr<vindex_t, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_frame_buffer_rgb_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(0, l_width, l_height, l_frame_buffer_rgb_format,
                               bgfx::TextureFormat::D32F);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_rgb_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      ColorInterpolationShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);

  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.outofbounds.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_depth_comparison_outofbounds(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_triangle_vertices.free();

  l_rast.shutdown();
}

TEST_CASE("rast.3Dcube") {

  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(8));

  {
    l_triangle_vertices.range()
        .stream(position_t{-1.0f, 1.0f, 1.0f})
        .stream(rgb_t{0, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(position_t{1.0f, 1.0f, 1.0f})
        .stream(rgb_t{0, 0, 255});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(position_t{-1.0f, -1.0f, 1.0f})
        .stream(rgb_t{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(position_t{1.0f, -1.0f, 1.0f})
        .stream(rgb_t{0, 255, 255});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(position_t{-1.0f, 1.0f, -1.0f})
        .stream(rgb_t{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(position_t{1.0f, 1.0f, -1.0f})
        .stream(rgb_t{255, 0, 255});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(6))
        .stream(position_t{-1.0f, -1.0f, -1.0f})
        .stream(rgb_t{255, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(7))
        .stream(position_t{1.0f, -1.0f, -1.0f})
        .stream(rgb_t{255, 255, 255});
  }

  container::arr<vindex_t, 36> l_triangle_indices = {0, 1, 2,          // 0
                                                     1, 3, 2, 4, 6, 5, // 2
                                                     5, 6, 7, 0, 2, 4, // 4
                                                     4, 2, 6, 1, 5, 3, // 6
                                                     5, 7, 3, 0, 4, 1, // 8
                                                     4, 5, 1, 2, 3, 6, // 10
                                                     6, 3, 7};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 128, l_height = 128;
  auto l_frame_buffer_rgb_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(0, l_width, l_height, l_frame_buffer_rgb_format,
                               bgfx::TextureFormat::D32F);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_rgb_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      ColorInterpolationShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_view, l_proj;
  {
    const m::vec<fix32, 3> at = {0.0f, 0.0f, 0.0f};
    const m::vec<fix32, 3> eye = {0.0f, 0.0f, -35.0f};

    // Set view and projection matrix for view 0.
    {
      l_view = m::look_at(eye, at, {0, 1, 0});
      l_proj =
          m::perspective(fix32(60.0f) * m::deg_to_rad,
                         fix32(l_width) / l_height, fix32(0.1f), fix32(100.0f));
    }
  }

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_view.m_data, l_proj.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  // Submit 11x11 cubes.
  for (uint32_t yy = 0; yy < 11; ++yy) {
    for (uint32_t xx = 0; xx < 11; ++xx) {
      m::mat<fix32, 4, 4> l_transform = m::mat<fix32, 4, 4>::getIdentity();
      l_transform.at(3, 0) = -15.0f + xx * 3.0f;
      l_transform.at(3, 1) = -15.0f + yy * 3.0f;
      l_transform.at(3, 2) = 0.0f;

      // Set model matrix for rendering.
      l_rast.setTransform(l_transform.m_data);

      l_rast.setIndexBuffer(l_index_buffer);
      l_rast.setVertexBuffer(0, l_vertex_buffer);
      l_rast.setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z |
                      BGFX_STATE_CULL_CW);

      // Submit primitive for rendering to view 0.
      l_rast.submit(0, l_program);
    }
  }

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.3Dcube.png",
      l_frame_buffer, l_frame_buffer_view, frame_expected::rast_3Dcube(),
      l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_triangle_vertices.free();

  l_rast.shutdown();
}

#undef WRITE_OUTPUT

#include <sys/sys_impl.hpp>