
#include <doctest.h>
#include <m/const.hpp>
#include <rast/rast.hpp>

#define WRITE_OUTPUT 1

#if WRITE_OUTPUT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>
#endif

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

  container::range<m::vec<ui8, 3>> l_frame_buffer_memory =
      RasterizerTestToolbox::getFrameBuffer(l_frame_buffer);
  REQUIRE(l_frame_buffer_memory.count() == l_width * l_height);

#if WRITE_OUTPUT
  auto *l_frame_texture =
      s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

  stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                 "rast.single_triangle.visibility.png",
                 l_frame_texture->info.width, l_frame_texture->info.height, 3,
                 l_frame_texture->range().m_begin,
                 l_frame_texture->info.bitsPerPixel *
                     l_frame_texture->info.width);
#endif

  container::arr<ui8, 192> l_frame_expected = {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   255, 255, 255, 0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   255, 255, 255, 255, 255, 255, 0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   255, 255, 255,
      255, 255, 255, 255, 255, 255, 0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

  REQUIRE(l_frame_buffer_memory.is_contained_by(
      l_frame_expected.range().cast_to<m::vec<ui8, 3>>()));

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

  container::range<m::vec<ui8, 3>> l_frame_buffer_memory =
      RasterizerTestToolbox::getFrameBuffer(l_frame_buffer);
  REQUIRE(l_frame_buffer_memory.count() == l_width * l_height);

#if WRITE_OUTPUT
  auto *l_frame_texture =
      s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

  stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                 "rast.single_triangle.vertex_color_interpolation.png",
                 l_frame_texture->info.width, l_frame_texture->info.height, 3,
                 l_frame_texture->range().m_begin,
                 l_frame_texture->info.bitsPerPixel *
                     l_frame_texture->info.width);
#endif

  container::arr<ui8, 192> l_frame_expected = {
      0,   0,   0, 0,  0,   0, 0,  0,   0, 255, 255, 0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 170, 170, 0,
      170, 233, 0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 85, 85,  0, 85, 148, 0, 85,  212, 0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 63, 0, 0,   127, 0,
      0,   191, 0, 0,  255, 0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0, 0, 0,  0, 0,   0,   0,
      0,   0,   0, 0,  0,   0, 0,  0,   0, 0,   0,   0};

  REQUIRE(l_frame_buffer_memory.is_contained_by(
      l_frame_expected.range().cast_to<m::vec<ui8, 3>>()));

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

  container::range<m::vec<ui8, 3>> l_frame_buffer_memory =
      RasterizerTestToolbox::getFrameBuffer(l_frame_buffer);
  REQUIRE(l_frame_buffer_memory.count() == l_width * l_height);

#if WRITE_OUTPUT
  auto *l_frame_texture =
      s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

  stbi_write_png(
      "/media/loic/SSD/SoftwareProjects/glm/rast.depth.comparison.png",
      l_frame_texture->info.width, l_frame_texture->info.height, 3,
      l_frame_texture->range().m_begin,
      l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width);
#endif

  container::arr<ui8, 192> l_frame_expected = {
      0,   0,   0, 0,   0,   0, 0,   0,   0, 255, 0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 255, 0, 0,
      255, 0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   255, 0, 255, 0,   0, 255, 0,   0, 255, 0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   255, 0, 0,   255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0,
      255, 0,   0, 255, 0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0};

  REQUIRE(l_frame_buffer_memory.is_contained_by(
      l_frame_expected.range().cast_to<m::vec<ui8, 3>>()));

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

  container::range<m::vec<ui8, 3>> l_frame_buffer_memory =
      RasterizerTestToolbox::getFrameBuffer(l_frame_buffer);
  REQUIRE(l_frame_buffer_memory.count() == l_width * l_height);

#if WRITE_OUTPUT
  auto *l_frame_texture =
      s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

  stbi_write_png(
      "/media/loic/SSD/SoftwareProjects/glm/rast.depth.comparison.large_framebuffer.png",
      l_frame_texture->info.width, l_frame_texture->info.height, 3,
      l_frame_texture->range().m_begin,
      l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width);
#endif

  container::arr<ui8, 192> l_frame_expected = {
      0,   0,   0, 0,   0,   0, 0,   0,   0, 255, 0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 255, 0, 0,
      255, 0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   255, 0, 255, 0,   0, 255, 0,   0, 255, 0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   255, 0, 0,   255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0,
      255, 0,   0, 255, 0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0, 0,   0, 0, 0,   0, 0,
      0,   0,   0, 0,   0,   0, 0,   0,   0, 0,   0, 0};

  REQUIRE(l_frame_buffer_memory.is_contained_by(
      l_frame_expected.range().cast_to<m::vec<ui8, 3>>()));

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

  container::range<m::vec<ui8, 3>> l_frame_buffer_memory =
      RasterizerTestToolbox::getFrameBuffer(l_frame_buffer);
  REQUIRE(l_frame_buffer_memory.count() == l_width * l_height);

#if WRITE_OUTPUT
  auto *l_frame_texture =
      s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

  stbi_write_png(
      "/media/loic/SSD/SoftwareProjects/glm/rast.depth.comparison.readonly.png",
      l_frame_texture->info.width, l_frame_texture->info.height, 3,
      l_frame_texture->range().m_begin,
      l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width);
#endif

  container::arr<ui8, 192> l_frame_expected = {
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 255, 0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 255, 0,
      0, 255, 0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0,
      0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0,
      0, 255, 0, 0, 255, 0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0,
      0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0};

  REQUIRE(l_frame_buffer_memory.is_contained_by(
      l_frame_expected.range().cast_to<m::vec<ui8, 3>>()));

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

  container::range<m::vec<ui8, 3>> l_frame_buffer_memory =
      RasterizerTestToolbox::getFrameBuffer(l_frame_buffer);
  REQUIRE(l_frame_buffer_memory.count() == l_width * l_height);

#if WRITE_OUTPUT
  auto *l_frame_texture =
      s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

  stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                 "rast.depth.comparison.outofbounds.png",
                 l_frame_texture->info.width, l_frame_texture->info.height, 3,
                 l_frame_texture->range().m_begin,
                 l_frame_texture->info.bitsPerPixel *
                     l_frame_texture->info.width);
#endif

  container::arr<ui8, 192> l_frame_expected = {
      0,   0,   0,   0, 0,   0,   0, 0,   0, 255, 0,   0, 255, 0,   0, 255,
      0,   0,   255, 0, 0,   0,   0, 0,   0, 0,   0,   0, 0,   0,   0, 0,
      0,   255, 0,   0, 255, 0,   0, 255, 0, 0,   255, 0, 0,   255, 0, 0,
      0,   0,   0,   0, 0,   0,   0, 255, 0, 255, 0,   0, 255, 0,   0, 255,
      0,   0,   255, 0, 0,   255, 0, 0,   0, 0,   0,   0, 0,   0,   0, 255,
      0,   255, 0,   0, 255, 0,   0, 255, 0, 0,   255, 0, 0,   255, 0, 0,
      0,   0,   0,   0, 255, 0,   0, 255, 0, 0,   255, 0, 0,   255, 0, 0,
      254, 0,   0,   0, 0,   0,   0, 0,   0, 255, 0,   0, 255, 0,   0, 255,
      0,   0,   255, 0, 0,   255, 0, 0,   0, 0,   0,   0, 0,   0,   0, 0,
      0,   255, 0,   0, 255, 0,   0, 255, 0, 0,   0,   0, 0,   0,   0, 0,
      0,   0,   0,   0, 0,   0,   0, 0,   0, 255, 0,   0, 255, 0,   0, 0,
      0,   0,   0,   0, 0,   0,   0, 0,   0, 0,   0,   0, 0,   0,   0, 0};

  REQUIRE(l_frame_buffer_memory.is_contained_by(
      l_frame_expected.range().cast_to<m::vec<ui8, 3>>()));

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

#undef WRITE_OUTPUT