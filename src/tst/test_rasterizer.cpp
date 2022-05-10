
#include <doctest.h>
#include <m/const.hpp>
#include <rast/rast.hpp>
#include <tst/test_rasterizer_assets.hpp>

#define WRITE_OUTPUT 0

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

namespace RasterizerTestToolbox {

inline static container::range<m::vec<ui8, 3>>
getFrameBuffer(bgfx::FrameBufferHandle p_frame_buffer) {
  return s_bgfx_impl.proxy()
      .FrameBuffer(p_frame_buffer)
      .RGBTexture()
      .value()
      ->range()
      .cast_to<m::vec<ui8, 3>>();
};

template <typename ExpectedFrameType>
inline static void
assert_frame_equals(const i8 *p_save_path,
                    bgfx::FrameBufferHandle p_frame_buffer,
                    const ExpectedFrameType &p_expected_frame) {
  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(p_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png(
        p_save_path, l_frame_texture->info.width, l_frame_texture->info.height,
        3, l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  REQUIRE(l_png_frame.count() == p_expected_frame.count());
  REQUIRE(l_png_frame.is_contained_by(p_expected_frame.range()));

  STBIW_FREE(l_png_frame.m_begin);
};

inline static bgfx::ProgramHandle
load_program(const container::range<rast::shader_vertex_output_parameter>
                 &p_vertex_output,
             rast::shader_vertex_function p_vertex,
             rast::shader_fragment_function p_fragment) {

  uimax l_vertex_shader_size = rast::shader_vertex_bytes::byte_size(1);
  const bgfx::Memory *l_vertex_shader_memory =
      bgfx::alloc(l_vertex_shader_size);
  rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
      p_vertex_output, p_vertex);

  const bgfx::Memory *l_fragment_shader_memory =
      bgfx::alloc(rast::shader_fragment_bytes::byte_size());
  rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
      p_fragment);

  bgfx::ShaderHandle l_vertex = bgfx::createShader(l_vertex_shader_memory);
  bgfx::ShaderHandle l_fragment = bgfx::createShader(l_fragment_shader_memory);
  return bgfx::createProgram(l_vertex, l_fragment);
};

inline static void loadVertexIndex(const bgfx::VertexLayout &p_vertex_layout,
                                   const container::range<ui8> &p_vertices,
                                   const container::range<ui8> &p_indicex,
                                   bgfx::VertexBufferHandle *out_vertex,
                                   bgfx::IndexBufferHandle *out_index) {
  const bgfx::Memory *l_vertex_memory =
      bgfx::makeRef(p_vertices.data(), p_vertices.count());
  const bgfx::Memory *l_index_memory =
      bgfx::makeRef(p_indicex.data(), p_indicex.count());

  *out_vertex = bgfx::createVertexBuffer(l_vertex_memory, p_vertex_layout);
  *out_index = bgfx::createIndexBuffer(l_index_memory);
};
}; // namespace RasterizerTestToolbox

struct WhiteShader {

  inline static container::arr<rast::shader_vertex_output_parameter, 0>
      s_vertex_output = {};

  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                     const ui8 *p_vertex, m::vec<f32, 4> &out_screen_position,
                     ui8 **out_vertex) {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos = l_shader.get_vertex<m::vec<f32, 3>>(
        bgfx::Attrib::Enum::Position, p_vertex);
    out_screen_position =
        p_ctx.m_local_to_unit * m::vec<f32, 4>::make(l_vertex_pos, 1);
  };

  static void fragment(ui8 **p_vertex_output_interpolated,
                       m::vec<f32, 3> &out_color) {
    out_color = {1, 1, 1};
  };

  inline static bgfx::ProgramHandle load_program() {
    return RasterizerTestToolbox::load_program(s_vertex_output.range(), vertex,
                                               fragment);
  };
};

struct ColorInterpolationShader {
  inline static container::arr<rast::shader_vertex_output_parameter, 1>
      s_vertex_output = {
          rast::shader_vertex_output_parameter(bgfx::AttribType::Float, 3)};

  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                     const ui8 *p_vertex, m::vec<f32, 4> &out_screen_position,
                     ui8 **out_vertex) {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos = l_shader.get_vertex<m::vec<f32, 3>>(
        bgfx::Attrib::Enum::Position, p_vertex);
    const m::vec<ui8, 3> &l_color = l_shader.get_vertex<m::vec<ui8, 3>>(
        bgfx::Attrib::Enum::Color0, p_vertex);
    out_screen_position =
        p_ctx.m_local_to_unit * m::vec<f32, 4>::make(l_vertex_pos, 1);

    m::vec<f32, 3> *l_vertex_color = (m::vec<f32, 3> *)out_vertex[0];
    (*l_vertex_color) = l_color.cast<f32>() / 255;
  };

  static void fragment(ui8 **p_vertex_output_interpolated,
                       m::vec<f32, 3> &out_color) {
    m::vec<f32, 3> *l_vertex_color =
        (m::vec<f32, 3> *)p_vertex_output_interpolated[0];
    out_color = *l_vertex_color;
  };

  inline static bgfx::ProgramHandle load_program() {
    return RasterizerTestToolbox::load_program(s_vertex_output.range(), vertex,
                                               fragment);
  };
};

TEST_CASE("rast.single_triangle.visibility") {

  bgfx::init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin();
  l_vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

  container::arr<m::vec<f32, 3>, 3> l_triangle_vertices = {
      .m_data = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}}};
  container::arr<ui16, 3> l_triangle_indices = {0, 1, 2};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(l_width, l_height, bgfx::TextureFormat::RGB8);

  bgfx::ProgramHandle l_program = WhiteShader::load_program();

  m::mat<f32, 4, 4> l_indentity = l_indentity.getIdentity();

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  bgfx::setTransform(l_indentity.m_data);
  bgfx::setIndexBuffer(l_index_buffer);
  bgfx::setVertexBuffer(0, l_vertex_buffer);
  bgfx::setState(0);

  bgfx::submit(0, l_program);

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.visibility.png",
      l_frame_buffer, frame_expected::rast_single_triangle_visibility());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  bgfx::shutdown();
}

TEST_CASE("rast.single_triangle.vertex_color_interpolation") {

  bgfx::init();

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
        .stream(m::vec<f32, 3>{0.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{0, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 255, 0});
  }

  container::arr<ui16, 3> l_triangle_indices = {0, 1, 2};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(l_width, l_height, bgfx::TextureFormat::RGB8);

  bgfx::ProgramHandle l_program = ColorInterpolationShader::load_program();

  m::mat<f32, 4, 4> l_indentity = l_indentity.getIdentity();

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  bgfx::setTransform(l_indentity.m_data);

  bgfx::setIndexBuffer(l_index_buffer);
  bgfx::setVertexBuffer(0, l_vertex_buffer);
  bgfx::setState(0);

  bgfx::submit(0, l_program);

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.vertex_color_interpolation.png",
      l_frame_buffer,
      frame_expected::rast_single_triangle_vertex_color_interpolation());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

TEST_CASE("rast.depth.comparison") {

  bgfx::init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(m::vec<f32, 3>{0.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(m::vec<f32, 3>{-0.5, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
  }

  container::arr<ui16, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(0, l_width, l_height, bgfx::TextureFormat::RGB8,
                              bgfx::TextureFormat::D32F);

  bgfx::ProgramHandle l_program = ColorInterpolationShader::load_program();

  m::mat<f32, 4, 4> l_indentity = l_indentity.getIdentity();

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  bgfx::setTransform(l_indentity.m_data);

  bgfx::setIndexBuffer(l_index_buffer);
  bgfx::setVertexBuffer(0, l_vertex_buffer);
  bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

  bgfx::submit(0, l_program);

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.png",
      l_frame_buffer, frame_expected::rast_depth_comparison());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

TEST_CASE("rast.depth.comparison.large_framebuffer") {

  bgfx::init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(m::vec<f32, 3>{0.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.0})
        .stream(m::vec<ui8, 3>{0, 0, 255});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(m::vec<f32, 3>{-0.5, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
  }

  container::arr<ui16, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 400, l_height = 400;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(0, l_width, l_height, bgfx::TextureFormat::RGB8,
                              bgfx::TextureFormat::D32F);

  bgfx::ProgramHandle l_program = ColorInterpolationShader::load_program();

  m::mat<f32, 4, 4> l_indentity = l_indentity.getIdentity();

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  bgfx::setTransform(l_indentity.m_data);

  bgfx::setIndexBuffer(l_index_buffer);
  bgfx::setVertexBuffer(0, l_vertex_buffer);
  bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

  bgfx::submit(0, l_program);

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.large_framebuffer.png",
      l_frame_buffer,
      frame_expected::rast_depth_comparison_large_framebuffer());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

TEST_CASE("rast.depth.comparison.readonly") {

  bgfx::init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(m::vec<f32, 3>{0.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(m::vec<f32, 3>{-0.5, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
  }

  container::arr<ui16, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(0, l_width, l_height, bgfx::TextureFormat::RGB8,
                              bgfx::TextureFormat::D32F);

  bgfx::ProgramHandle l_program = ColorInterpolationShader::load_program();

  m::mat<f32, 4, 4> l_indentity = l_indentity.getIdentity();

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  bgfx::setTransform(l_indentity.m_data);

  bgfx::setIndexBuffer(l_index_buffer);
  bgfx::setVertexBuffer(0, l_vertex_buffer);
  bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS);

  bgfx::submit(0, l_program);

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.readonly.png",
      l_frame_buffer, frame_expected::rast_depth_comparison_readonly());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

// top left-right out of bounds
TEST_CASE("rast.depth.comparison.outofbounds") {

  bgfx::init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(3) * 2);

  {
    l_triangle_vertices.range()
        .stream(m::vec<f32, 3>{0.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(m::vec<f32, 3>{2.0, 0.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(m::vec<f32, 3>{0.0, 2.0, 0.0})
        .stream(m::vec<ui8, 3>{255, 0, 0});

    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(m::vec<f32, 3>{-2, -2, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(m::vec<f32, 3>{1.0, 0.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(m::vec<f32, 3>{0.0, 1.0, 0.1})
        .stream(m::vec<ui8, 3>{0, 255, 0});
  }

  container::arr<ui16, 6> l_triangle_indices = {0, 1, 2, 3, 4, 5};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 8, l_height = 8;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(0, l_width, l_height, bgfx::TextureFormat::RGB8,
                              bgfx::TextureFormat::D32F);

  bgfx::ProgramHandle l_program = ColorInterpolationShader::load_program();

  m::mat<f32, 4, 4> l_indentity = l_indentity.getIdentity();

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  bgfx::setTransform(l_indentity.m_data);

  bgfx::setIndexBuffer(l_index_buffer);
  bgfx::setVertexBuffer(0, l_vertex_buffer);
  bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

  bgfx::submit(0, l_program);

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.outofbounds.png",
      l_frame_buffer, frame_expected::rast_depth_comparison_outofbounds());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

TEST_CASE("rast.3Dcube") {
  bgfx::init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
      .end();

  container::span<ui8> l_triangle_vertices;
  l_triangle_vertices.allocate(l_vertex_layout.getSize(8));

  {
    l_triangle_vertices.range()
        .stream(m::vec<f32, 3>{-1.0f, 1.0f, 1.0f})
        .stream(m::vec<ui8, 3>{0, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(1))
        .stream(m::vec<f32, 3>{1.0f, 1.0f, 1.0f})
        .stream(m::vec<ui8, 3>{0, 0, 255});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(2))
        .stream(m::vec<f32, 3>{-1.0f, -1.0f, 1.0f})
        .stream(m::vec<ui8, 3>{0, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(3))
        .stream(m::vec<f32, 3>{1.0f, -1.0f, 1.0f})
        .stream(m::vec<ui8, 3>{0, 255, 255});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(4))
        .stream(m::vec<f32, 3>{-1.0f, 1.0f, -1.0f})
        .stream(m::vec<ui8, 3>{255, 0, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(5))
        .stream(m::vec<f32, 3>{1.0f, 1.0f, -1.0f})
        .stream(m::vec<ui8, 3>{255, 0, 255});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(6))
        .stream(m::vec<f32, 3>{-1.0f, -1.0f, -1.0f})
        .stream(m::vec<ui8, 3>{255, 255, 0});
    l_triangle_vertices.range()
        .slide(l_vertex_layout.getSize(7))
        .stream(m::vec<f32, 3>{1.0f, -1.0f, -1.0f})
        .stream(m::vec<ui8, 3>{255, 255, 255});
  }

  container::arr<ui16, 36> l_triangle_indices = {0, 1, 2,          // 0
                                                 1, 3, 2, 4, 6, 5, // 2
                                                 5, 6, 7, 0, 2, 4, // 4
                                                 4, 2, 6, 1, 5, 3, // 6
                                                 5, 7, 3, 0, 4, 1, // 8
                                                 4, 5, 1, 2, 3, 6, // 10
                                                 6, 3, 7};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  constexpr ui16 l_width = 128, l_height = 128;

  bgfx::FrameBufferHandle l_frame_buffer =
      bgfx::createFrameBuffer(0, l_width, l_height, bgfx::TextureFormat::RGB8,
                              bgfx::TextureFormat::D32F);

  bgfx::ProgramHandle l_program = ColorInterpolationShader::load_program();

  m::mat<f32, 4, 4> l_view, l_proj;
  {
    const m::vec<f32, 3> at = {0.0f, 0.0f, 0.0f};
    const m::vec<f32, 3> eye = {0.0f, 0.0f, -35.0f};

    // Set view and projection matrix for view 0.
    {
      l_view = m::look_at(eye, at, {0, 1, 0});
      l_proj = m::perspective(60.0f * m::deg_to_rad,
                              float(l_width) / float(l_height), 0.1f, 100.0f);
    }
  }

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
  bgfx::setViewRect(0, 0, 0, l_width, l_height);
  bgfx::setViewTransform(0, l_view.m_data, l_proj.m_data);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);

  bgfx::touch(0);

  // Submit 11x11 cubes.
  for (uint32_t yy = 0; yy < 11; ++yy) {
    for (uint32_t xx = 0; xx < 11; ++xx) {
      m::mat<f32, 4, 4> l_transform = m::mat<f32, 4, 4>::getIdentity();
      l_transform.at(3, 0) = -15.0f + xx * 3.0f;
      l_transform.at(3, 1) = -15.0f + yy * 3.0f;
      l_transform.at(3, 2) = 0.0f;

      // Set model matrix for rendering.
      bgfx::setTransform(l_transform.m_data);

      bgfx::setIndexBuffer(l_index_buffer);
      bgfx::setVertexBuffer(0, l_vertex_buffer);
      bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z);

      // Submit primitive for rendering to view 0.
      bgfx::submit(0, l_program);
    }
  }

  bgfx::frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.3Dcube.png",
      l_frame_buffer, frame_expected::rast_3Dcube());

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

#undef WRITE_OUTPUT