
#include <doctest.h>
#include <m/const.hpp>
#include <rast/rast.hpp>

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

  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                   "rast.single_triangle.visibility.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  container::arr<ui8, 90> l_frame_expected = {
      137, 80,  78,  71,  13,  10, 26, 10,  0,   0,  0,   13,  73,  72,  68,
      82,  0,   0,   0,   8,   0,  0,  0,   8,   8,  2,   0,   0,   0,   75,
      109, 41,  220, 0,   0,   0,  33, 73,  68,  65, 84,  120, 94,  99,  128,
      131, 255, 255, 255, 51,  32, 1,  38,  100, 14, 178, 28,  138, 4,   80,
      17,  92,  142, 17,  151, 81, 12, 116, 0,   0,  252, 42,  11,  250, 52,
      43,  182, 211, 0,   0,   0,  0,  73,  69,  78, 68,  174, 66,  96,  130};

  REQUIRE(l_png_frame.is_contained_by(l_frame_expected.range()));

  STBIW_FREE(l_png_frame.m_begin);

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

  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                   "rast.single_triangle.vertex_color_interpolation.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  container::arr<ui8, 107> l_frame_expected = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,   0,   13,  73,  72,
      68,  82,  0,   0,   0,   8,   0,   0,   0,   8,   8,   2,   0,   0,
      0,   75,  109, 41,  220, 0,   0,   0,   50,  73,  68,  65,  84,  120,
      94,  99,  128, 131, 255, 255, 25,  144, 1,   19,  156, 179, 122, 53,
      195, 170, 151, 8,   41,  102, 100, 85,  215, 190, 50,  104, 119, 49,
      92,  155, 6,   18,  99,  68,  209, 111, 207, 192, 224, 0,   67,  12,
      180, 7,   0,   206, 182, 9,   217, 87,  197, 143, 9,   0,   0,   0,
      0,   73,  69,  78,  68,  174, 66,  96,  130};

  REQUIRE(l_png_frame.is_contained_by(l_frame_expected.range()));

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

  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                   "rast.depth.comparison.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  container::arr<ui8, 92> l_frame_expected = {
      137, 80,  78,  71,  13,  10, 26,  10,  0,   0,   0,   13,  73,  72,
      68,  82,  0,   0,   0,   8,  0,   0,   0,   8,   8,   2,   0,   0,
      0,   75,  109, 41,  220, 0,  0,   0,   35,  73,  68,  65,  84,  120,
      94,  99,  128, 131, 255, 12, 40,  128, 9,   153, 135, 44,  7,   147,
      128, 137, 193, 229, 88,  64, 58,  80,  77,  65,  51,  147, 129, 118,
      0,   0,   88,  29,  6,   3,  246, 197, 161, 253, 0,   0,   0,   0,
      73,  69,  78,  68,  174, 66, 96,  130};

  REQUIRE(l_png_frame.is_contained_by(l_frame_expected.range()));

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

  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                   "rast.depth.comparison.large_framebuffer.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  container::arr<ui8, 8752> l_frame_expected = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,   0,   13,  73,  72,  68,
      82,  0,   0,   1,   144, 0,   0,   1,   144, 8,   2,   0,   0,   0,   15,
      221, 161, 155, 0,   0,   33,  247, 73,  68,  65,  84,  120, 94,  99,  24,
      5,   163, 33,  64,  131, 16,  248, 63,  26,  170, 163, 33,  64,  139, 16,
      96,  26,  13,  214, 209, 16,  160, 122, 8,   48,  50,  252, 99,  96,  252,
      59,  26,  176, 163, 33,  64,  245, 16,  24,  45,  176, 70,  19,  21,  181,
      67,  224, 63,  3,   35,  195, 127, 32,  98,  96,  250, 51,  26,  184, 163,
      33,  64,  221, 16,  24,  45,  176, 70,  83,  20,  3,   13,  90,  88,  255,
      161, 101, 22,  243, 239, 209, 240, 29,  13,  1,   42,  134, 192, 104, 129,
      53,  154, 156, 168, 26,  2,   224, 193, 43,  38,  134, 127, 140, 48,  196,
      192, 250, 115, 52,  136, 71,  67,  128, 90,  33,  48,  90,  96,  141, 166,
      37,  154, 180, 176, 152, 24,  160, 141, 44,  80,  223, 144, 237, 199, 104,
      40,  143, 134, 0,   85,  66,  96,  180, 192, 26,  77,  72,  212, 11,  1,
      216, 220, 32,  164, 63,  136, 82,  102, 177, 127, 31,  13,  232, 209, 16,
      160, 60,  4,   70,  11,  172, 209, 84,  68,  147, 22,  22,  150, 50,  139,
      227, 219, 104, 88,  143, 134, 0,   133, 33,  48,  90,  96,  141, 38,  33,
      42,  133, 0,   210, 210, 43,  228, 49,  44,  148, 241, 44,  174, 47,  163,
      193, 61,  26,  2,   148, 132, 192, 104, 129, 53,  154, 126, 104, 210, 194,
      66,  238, 15,  162, 244, 13,  185, 63,  143, 134, 248, 104, 8,   144, 29,
      2,   163, 5,   214, 104, 226, 161, 70,  8,   160, 174, 108, 199, 236, 15,
      162, 148, 89,  60,  159, 70,  3,   125, 52,  4,   200, 11,  129, 209, 2,
      107, 52,  229, 80,  63,  4,   224, 221, 64,  156, 125, 67,  190, 15,  163,
      225, 62,  26,  2,   100, 132, 192, 104, 129, 53,  154, 108, 40,  14,  1,
      140, 141, 131, 192, 22,  22,  188, 73,  5,   100, 32,  179, 25,  97,  203,
      29,  24,  248, 223, 143, 6,   253, 104, 8,   144, 26,  2,   163, 5,   214,
      104, 154, 161, 126, 8,   224, 233, 18,  162, 244, 13,  5,   222, 141, 134,
      254, 104, 8,   144, 20,  2,   163, 5,   214, 104, 130, 161, 44,  4,   176,
      157, 203, 0,   111, 70,  97,  29,  122, 71,  41,  179, 4,   223, 142, 70,
      192, 104, 8,   16,  31,  2,   163, 5,   214, 104, 106, 161, 126, 8,   224,
      26,  186, 194, 58,  182, 197, 32,  252, 122, 52,  14,  70,  67,  128, 200,
      16,  24,  45,  176, 70,  147, 10,  5,   33,  128, 227, 216, 43,  228, 49,
      44,  92,  108, 148, 118, 150, 200, 171, 209, 104, 24,  13,  1,   98,  66,
      96,  180, 192, 26,  77,  39,  212, 15,  1,   252, 203, 26,  176, 143, 193,
      139, 190, 28,  141, 137, 209, 16,  32,  24,  2,   163, 5,   214, 104, 34,
      33,  55,  4,   112, 159, 42,  74,  112, 89,  3,   246, 190, 161, 216, 243,
      209, 200, 24,  13,  1,   252, 33,  48,  90,  96,  141, 166, 16,  154, 180,
      176, 176, 54,  163, 8,   143, 193, 75,  60,  27,  141, 143, 209, 16,  192,
      19,  2,   163, 5,   214, 104, 242, 32,  43,  4,   240, 30,  218, 142, 214,
      37,  196, 181, 77,  7,   123, 223, 80,  242, 233, 104, 148, 140, 134, 0,
      174, 16,  24,  45,  176, 70,  211, 6,   77,  90,  88,  20,  149, 89,  82,
      79,  70,  99,  101, 52,  4,   176, 134, 192, 104, 129, 53,  154, 48,  72,
      15,  1,   66,  119, 226, 96,  93,  214, 64,  218, 90,  7,   153, 71,  163,
      17,  51,  26,  2,   152, 33,  48,  90,  96,  141, 166, 10,  154, 180, 176,
      8,   14,  87,  17,  30,  207, 146, 125, 56,  26,  55,  163, 33,  128, 22,
      2,   163, 5,   214, 104, 146, 32,  49,  4,   136, 184, 114, 16,  255, 214,
      28,  252, 227, 241, 40,  235, 179, 228, 30,  140, 70,  207, 104, 8,   32,
      135, 192, 104, 129, 53,  154, 30,  168, 31,  2,   228, 45,  107, 192, 190,
      214, 65,  254, 222, 104, 12,  141, 134, 0,   60,  4,   70,  11,  172, 209,
      196, 64,  74,  8,   16,  119, 163, 51,  242, 234, 118, 146, 166, 8,   177,
      207, 27,  42,  222, 29,  141, 164, 209, 16,  128, 132, 192, 104, 129, 53,
      154, 18,  168, 31,  2,   148, 76,  17,  98,  47,  179, 148, 238, 140, 198,
      211, 104, 8,   0,   67,  96,  180, 192, 26,  77,  6,   68,  135, 192, 127,
      98,  85,  98,  158, 214, 64,  118, 59,  11,  174, 145, 65,  249, 246, 104,
      84,  141, 134, 192, 104, 129, 53,  154, 6,   168, 31,  2,   148, 47,  107,
      192, 106, 2,   131, 202, 205, 209, 216, 26,  225, 33,  48,  90,  96,  141,
      102, 1,   226, 66,  224, 63,  9,   1,   69,  204, 9,   13,  228, 173, 123,
      96,  80,  187, 49,  26,  97,  35,  57,  4,   70,  11,  172, 209, 244, 79,
      253, 16,  32,  242, 196, 81,  50,  203, 44,  245, 235, 163, 113, 54,  98,
      67,  96,  180, 192, 26,  77,  252, 68,  132, 192, 127, 210, 66,  9,   255,
      178, 6,   146, 150, 188, 99,  239, 27,  106, 92,  29,  141, 182, 145, 25,
      2,   163, 5,   214, 104, 202, 167, 73,  11,  139, 248, 213, 161, 100, 182,
      179, 180, 174, 140, 198, 220, 8,   12,  129, 209, 2,   107, 52,  217, 19,
      10,  129, 255, 36,  7,   17,  49,  203, 26,  200, 155, 55,  68,  30,  29,
      99,  208, 190, 60,  26,  121, 35,  45,  4,   70,  11,  172, 209, 52,  79,
      147, 22,  22,  157, 202, 44,  157, 75,  163, 241, 55,  162, 66,  96,  180,
      192, 26,  77,  240, 120, 67,  224, 63,  57,  225, 67,  252, 178, 6,   160,
      74,  10,  7,   188, 24,  116, 47,  140, 70,  225, 200, 9,   129, 209, 2,
      107, 52,  181, 211, 164, 133, 69,  249, 105, 13,  248, 71,  193, 80,  250,
      134, 250, 231, 71,  99,  113, 132, 132, 192, 104, 129, 53,  154, 212, 113,
      135, 192, 127, 50,  3,   135, 234, 203, 26,  8,   14,  120, 49,  24,  156,
      27,  141, 200, 145, 16,  2,   163, 5,   214, 104, 58,  167, 73,  11,  107,
      0,   202, 44,  195, 179, 163, 113, 57,  236, 67,  96,  180, 192, 26,  77,
      228, 56,  66,  224, 63,  249, 33,  67,  249, 74,  43,  92,  163, 96,  248,
      7,   188, 24,  140, 79,  143, 70,  231, 240, 14,  129, 209, 2,   107, 52,
      133, 211, 164, 133, 69,  249, 170, 5,   242, 76,  96,  48,  57,  53,  26,
      163, 195, 56,  4,   70,  11,  172, 209, 228, 141, 45,  4,   254, 83,  20,
      44,  120, 250, 131, 148, 31,  149, 69,  208, 4,   6,   211, 147, 163, 145,
      58,  92,  67,  96,  180, 192, 26,  77,  219, 212, 15,  1,   228, 142, 27,
      133, 171, 22,  200, 235, 93,  50,  152, 29,  31,  141, 215, 97,  25,  2,
      163, 5,   214, 104, 194, 198, 8,   129, 255, 148, 134, 9,   73,  167, 53,
      208, 168, 243, 200, 96,  113, 108, 52,  106, 135, 95,  8,   140, 22,  88,
      163, 169, 154, 250, 33,  64,  234, 20,  33,  173, 202, 44,  203, 163, 163,
      177, 59,  204, 66,  96,  180, 192, 26,  77,  210, 168, 33,  240, 159, 10,
      1,   2,   41,  176, 6,   69,  59,  203, 234, 200, 104, 4,   15,  167, 16,
      24,  45,  176, 70,  211, 51,  245, 67,  128, 188, 129, 39,  202, 183, 233,
      96,  63,  139, 198, 250, 208, 104, 28,  15,  155, 16,  24,  45,  176, 70,
      19,  51,  82,  8,   252, 167, 78,  104, 224, 106, 91,  81,  62,  69,  72,
      230, 90,  7,   219, 131, 163, 209, 60,  60,  66,  96,  180, 192, 26,  77,
      201, 212, 15,  1,   154, 46,  107, 32,  179, 204, 178, 59,  48,  26,  211,
      195, 32,  4,   70,  11,  172, 209, 100, 12,  11,  129, 255, 84,  11,  10,
      252, 75,  25,  24,  25,  254, 81,  168, 128, 204, 181, 14,  246, 251, 70,
      35,  123, 168, 135, 192, 104, 129, 53,  154, 134, 105, 210, 194, 34,  254,
      172, 5,   242, 90,  76,  64,  93,  248, 173, 192, 106, 44,  131, 227, 222,
      209, 248, 30,  210, 33,  48,  90,  96,  141, 38,  96,  112, 8,   252, 167,
      102, 56,  16,  115, 122, 31,  29,  150, 188, 99,  47,  179, 156, 246, 140,
      70,  249, 208, 13,  129, 209, 2,   107, 52,  245, 210, 164, 133, 53,  168,
      203, 44,  231, 221, 163, 177, 62,  68,  67,  96,  180, 192, 26,  77,  186,
      12,  12,  255, 169, 28,  8,   88,  199, 152, 200, 27,  120, 162, 209, 128,
      23,  131, 203, 206, 209, 136, 31,  138, 33,  48,  90,  96,  141, 166, 91,
      154, 180, 176, 176, 118, 199, 200, 27,  120, 162, 202, 98,  8,   204, 1,
      47,  6,   183, 29,  163, 113, 63,  228, 66,  96,  180, 192, 26,  241, 137,
      246, 63,  173, 186, 132, 100, 143, 166, 147, 177, 80,  158, 188, 66,  141,
      193, 125, 251, 104, 153, 53,  180, 66,  96,  180, 192, 26,  77,  177, 212,
      15,  129, 1,   89,  181, 64,  94,  231, 145, 193, 99,  235, 104, 10,  24,
      66,  33,  48,  90,  96,  141, 236, 228, 250, 159, 38,  222, 31,  168, 21,
      237, 100, 182, 179, 60,  183, 140, 150, 89,  67,  37,  4,   70,  11,  172,
      209, 180, 74,  253, 16,  160, 202, 20,  33,  61,  7,   188, 24,  188, 55,
      143, 166, 131, 33,  17,  2,   163, 5,   214, 8,   78,  168, 255, 105, 229,
      119, 204, 65,  40,  242, 198, 179, 8,   182, 152, 168, 104, 44,  131, 207,
      166, 209, 50,  107, 240, 135, 192, 104, 129, 53,  154, 74,  169, 31,  2,
      244, 92,  214, 64,  197, 213, 18,  12,  190, 27,  70,  83,  195, 32,  15,
      129, 209, 2,   107, 164, 38,  209, 255, 52,  244, 56,  114, 203, 136, 242,
      241, 44,  186, 182, 179, 252, 215, 143, 150, 89,  131, 57,  4,   70,  11,
      172, 209, 244, 73,  253, 16,  64,  27,  195, 162, 79,  153, 69,  198, 214,
      66,  172, 14,  99,  8,   88,  55,  154, 38,  6,   109, 8,   140, 22,  88,
      35,  50,  113, 254, 167, 173, 175, 7,   228, 226, 9,   130, 203, 26,  136,
      239, 60,  50,  4,   174, 25,  45,  179, 6,   103, 8,   140, 22,  88,  163,
      41,  147, 38,  45,  44,  106, 181, 119, 168, 219, 187, 36,  126, 144, 158,
      33,  104, 245, 104, 202, 24,  132, 33,  48,  90,  96,  141, 188, 100, 249,
      159, 230, 94,  38,  102, 89,  3,   141, 230, 13,  169, 56,  224, 197, 16,
      178, 106, 180, 204, 26,  108, 33,  48,  90,  96,  141, 166, 73,  154, 180,
      176, 136, 44,  179, 6,   164, 33,  70,  252, 10,  47,  134, 208, 149, 163,
      233, 99,  80,  133, 192, 104, 129, 53,  194, 18,  228, 127, 122, 248, 151,
      138, 203, 26,  6,   252, 140, 7,   134, 176, 229, 163, 101, 214, 224, 9,
      129, 209, 2,   107, 52,  53,  210, 164, 133, 133, 181, 199, 55,  248, 183,
      67,  99,  159, 55,  12,  95,  54,  154, 74,  6,   73,  8,   140, 22,  88,
      35,  41,  41,  254, 167, 147, 103, 169, 190, 172, 97,  192, 7,   188, 24,
      34,  151, 142, 150, 89,  131, 33,  4,   70,  11,  172, 209, 116, 72,  147,
      22,  22,  45,  202, 172, 129, 29,  240, 98,  136, 90,  50,  154, 86,  6,
      60,  4,   70,  11,  172, 17,  147, 8,   73,  109, 94,  225, 87,  143, 87,
      118, 80,  29,  46,  74,  249, 153, 165, 112, 19,  24,  162, 23,  141, 150,
      89,  3,   27,  2,   163, 5,   214, 104, 10,  196, 17,  2,   140, 120, 67,
      6,   175, 44,  174, 197, 83,  67,  107, 155, 14,  86,  95,  48,  196, 46,
      28,  77,  49,  3,   24,  2,   163, 5,   214, 200, 72,  126, 255, 233, 234,
      77,  60,  253, 193, 225, 80,  102, 197, 45,  24,  45,  179, 6,   42,  4,
      70,  11,  172, 209, 180, 71,  253, 16,  64,  222, 154, 51,  32,  219, 116,
      152, 8,   221, 213, 74,  97,  167, 149, 33,  126, 222, 104, 186, 25,  144,
      16,  24,  45,  176, 70,  64,  194, 251, 79,  111, 63,  226, 234, 18,  82,
      113, 173, 3,   229, 45,  53,  10,  103, 30,  25,  18,  230, 142, 150, 89,
      244, 15,  129, 209, 2,   107, 52,  213, 81,  63,  4,   72,  157, 34,  28,
      252, 219, 116, 176, 143, 103, 37,  205, 25,  77,  61,  116, 14,  129, 209,
      2,   107, 184, 39,  185, 255, 3,   224, 65,  72,  129, 53,  34,  218, 89,
      201, 179, 71,  203, 44,  122, 134, 192, 104, 129, 53,  154, 222, 168, 31,
      2,   228, 141, 16,  209, 122, 224, 9,   121, 125, 3,   218, 90,  7,   74,
      6,   218, 24,  82,  102, 142, 166, 33,  186, 133, 192, 104, 129, 53,  172,
      19,  219, 255, 129, 241, 29,  174, 182, 213, 128, 15,  60,  145, 237, 48,
      252, 157, 86,  134, 180, 25,  163, 101, 22,  125, 66,  96,  180, 192, 26,
      77,  105, 212, 15,  1,   154, 46,  107, 24,  156, 3,   94,  12,  233, 211,
      71,  83,  18,  29,  66,  96,  180, 192, 26,  190, 201, 236, 255, 128, 121,
      13,  127, 15,  139, 242, 165, 231, 131, 179, 243, 200, 144, 49,  117, 180,
      204, 162, 117, 8,   140, 22,  88,  163, 105, 140, 38,  45,  44,  252, 251,
      254, 134, 193, 242, 81,  236, 243, 134, 153, 83,  70,  211, 19,  77,  67,
      96,  180, 192, 26,  166, 9,   236, 255, 64,  250, 11,  173, 75,  56,  68,
      87,  45,  144, 55,  224, 197, 144, 61,  121, 180, 204, 162, 93,  8,   140,
      22,  88,  163, 169, 139, 218, 33,  240, 159, 1,   82,  96,  209, 161, 25,
      69,  208, 138, 1,   41,  43,  25,  114, 38,  141, 166, 42,  26,  133, 192,
      104, 129, 53,  28,  147, 214, 128, 54,  175, 128, 197, 21,  174, 19,  71,
      135, 229, 54,  29,  172, 171, 37,  24,  114, 39,  140, 150, 89,  180, 8,
      129, 209, 2,   107, 52,  93,  81,  63,  4,   104, 180, 122, 96,  104, 173,
      150, 96,  200, 235, 31,  77,  91,  84,  15,  129, 209, 2,   107, 216, 37,
      170, 255, 3,   239, 35,  74,  150, 53,  12,  167, 1,   47,  134, 130, 190,
      209, 50,  139, 186, 33,  48,  90,  96,  141, 166, 40,  6,   26,  84,  131,
      255, 128, 29,  37,  92,  43,  203, 25,  105, 124, 148, 194, 160, 178, 151,
      161, 176, 103, 52,  133, 81,  49,  4,   70,  11,  172, 225, 149, 156, 254,
      15,  10,  239, 80,  101, 69,  251, 32,  191, 1,   140, 248, 157, 146, 12,
      69,  221, 163, 101, 22,  181, 66,  96,  180, 192, 26,  77,  75,  212, 15,
      1,   180, 46,  33,  141, 122, 121, 67,  168, 243, 200, 80,  210, 53,  154,
      206, 168, 18,  2,   163, 5,   214, 48,  74,  72,  255, 7,   139, 95,  232,
      182, 172, 97,  40,  149, 89,  165, 157, 163, 101, 22,  229, 33,  48,  90,
      96,  141, 166, 34,  234, 135, 0,   21,  47,  82,  29,  78,  3,   94,  12,
      101, 237, 163, 169, 141, 194, 16,  24,  45,  176, 134, 75,  18,  250, 63,
      136, 60,  66,  252, 248, 14,  85,  58,  143, 192, 118, 214, 80,  25,  240,
      98,  40,  111, 27,  45,  179, 40,  9,   129, 209, 2,   107, 52,  253, 80,
      63,  4,   240, 44,  107, 24,  81,  219, 116, 176, 22,  220, 12,  149, 173,
      163, 105, 142, 236, 16,  24,  45,  176, 134, 69,  226, 249, 63,  184, 124,
      49,  32,  43,  218, 135, 80,  231, 145, 161, 170, 121, 180, 204, 34,  47,
      4,   70,  11,  172, 209, 148, 67,  147, 22,  214, 128, 244, 209, 6,   231,
      214, 66,  236, 237, 172, 234, 166, 209, 148, 71,  70,  8,   140, 22,  88,
      67,  63,  217, 12,  72,  243, 10,  175, 165, 196, 140, 76,  13,  161, 129,
      39,  26,  13,  180, 49,  212, 52,  142, 150, 89,  164, 134, 192, 104, 129,
      53,  154, 102, 200, 10,  1,   66,  55,  63,  83,  43,  147, 99,  174, 144,
      32,  216, 140, 162, 92,  1,   221, 6,   218, 24,  234, 26,  70,  211, 31,
      73,  33,  48,  90,  96,  13,  241, 4,   243, 127, 48,  186, 159, 138, 203,
      26,  176, 26,  69,  249, 153, 165, 131, 103, 192, 139, 161, 190, 110, 180,
      204, 34,  62,  4,   70,  11,  172, 209, 212, 66,  253, 16,  160, 238, 178,
      134, 97,  63,  177, 200, 208, 80,  59,  154, 10,  137, 12,  129, 209, 2,
      107, 40,  39,  149, 255, 131, 212, 241, 84,  95,  214, 48,  236, 7,   188,
      24,  154, 106, 70,  203, 44,  98,  66,  96,  180, 192, 26,  77,  39,  52,
      105, 97,  81,  189, 204, 34,  56,  50,  53,  212, 27,  98,  12,  205, 213,
      163, 105, 145, 96,  8,   140, 22,  88,  67,  54,  145, 252, 31,  188, 46,
      31,  246, 3,   79,  104, 135, 231, 80,  203, 191, 12,  45,  149, 163, 101,
      22,  254, 16,  24,  45,  176, 70,  83,  8,   77,  90,  88,  163, 43,  218,
      201, 155, 39,  101, 104, 173, 24,  77,  145, 120, 66,  96,  180, 192, 26,
      154, 201, 227, 255, 160, 118, 54,  60,  175, 14,  155, 51,  173, 232, 185,
      178, 140, 161, 189, 124, 180, 204, 194, 21,  2,   163, 5,   214, 104, 218,
      160, 126, 8,   16,  191, 172, 97,  180, 243, 136, 181, 119, 201, 208, 81,
      58,  154, 46,  177, 134, 192, 104, 129, 53,  4,   19,  198, 63,  6,   134,
      255, 96,  68,  182, 219, 241, 52,  208, 240, 75,  225, 177, 23,  73,  138,
      164, 101, 13,  163, 157, 71,  172, 205, 55,  134, 206, 146, 209, 50,  11,
      51,  4,   70,  11,  172, 33,  152, 42,  128, 171, 204, 33,  136, 108, 183,
      227, 89,  167, 142, 95,  10,  143, 189, 72,  82,  164, 78,  17,  142, 150,
      89,  216, 203, 172, 174, 226, 209, 50,  11,  45,  4,   70,  11,  172, 161,
      150, 36,  254, 15,  1,   7,   67,  178, 31,  169, 237, 172, 209, 1,   47,
      204, 165, 27,  12,  61,  69,  163, 101, 22,  114, 8,   176, 140, 6,   199,
      104, 8,   80,  61,  4,   128, 35,  83,  255, 96,  134, 2,   171, 68,  76,
      54,  86,  65,  160, 14,  184, 56,  21,  117, 17,  52,  118, 144, 219, 251,
      191, 183, 128, 161, 120, 244, 90,  86,  104, 122, 26,  109, 97,  13,  169,
      242, 234, 255, 208, 112, 45,  174, 182, 21,  193, 197, 159, 195, 126, 117,
      40,  121, 157, 101, 134, 190, 252, 209, 106, 21,  18,  2,   163, 5,   214,
      176, 72,  9,   255, 41,  30,  134, 199, 31,  12,  36,  22,  148, 120, 178,
      229, 104, 153, 69,  102, 153, 53,  33,  111, 180, 204, 2,   134, 192, 104,
      151, 112, 232, 36,  3,   60,  165, 6,   35,  141, 125, 65,  162, 249, 240,
      46,  33,  121, 93,  63,  160, 103, 8,   246, 13,  129, 129, 1,   233, 105,
      210, 179, 243, 200, 4,   174, 23,  6,   204, 222, 137, 57,  12,  249, 83,
      70,  120, 177, 53,  218, 194, 26,  173, 183, 168, 31,  2,   148, 55,  163,
      70,  27,  98,  88,  27,  98,  12,  147, 178, 71,  11,  172, 209, 28,  59,
      20,  66,  224, 255, 80,  138, 38,  172, 147, 244, 164, 118, 133, 70,  203,
      44,  236, 101, 214, 228, 172, 145, 156, 99,  71,  91,  88,  163, 229, 53,
      77,  90,  88,  163, 101, 22,  36,  4,   144, 139, 93,  106, 53,  60,  25,
      166, 102, 142, 216, 84,  59,  58,  134, 53,  20,  162, 254, 31,  89,  142,
      132, 52,  202, 24,  7,   192, 131, 88,  151, 53,  12,  240, 0,   16,  56,
      24,  134, 207, 128, 215, 180, 116, 134, 172, 153, 35,  176, 216, 26,  109,
      97,  13,  133, 72,  39,  175, 208, 161, 112, 53,  60,  5,   1,   131, 171,
      89,  65,  176, 137, 49,  186, 228, 157, 248, 150, 41,  195, 244, 180, 209,
      2,   107, 180, 127, 52,  248, 66,  224, 255, 208, 139, 20,  60,  195, 85,
      163, 101, 22,  21,  59,  203, 12,  51,  83,  71,  90,  142, 29,  237, 18,
      14,  181, 24,  135, 151, 95,  36,  53,  187, 200, 211, 69,  110, 216, 80,
      184, 172, 97,  180, 243, 200, 128, 218, 129, 197, 183, 58,  100, 86,  50,
      67,  218, 220, 145, 83,  108, 141, 118, 9,   7,   119, 92,  99,  54,  175,
      200, 219, 249, 76,  249, 126, 105, 82,  194, 137, 90,  163, 203, 244, 159,
      88,  28,  138, 13,  64,  134, 217, 73,  163, 5,   22,  195, 40,  24,  158,
      33,  64,  151, 14,  38,  125, 166, 8,   71,  7,   188, 224, 225, 204, 48,
      39,  113, 132, 100, 216, 209, 22,  214, 32,  142, 104, 90,  20,  46,  116,
      153, 52,  132, 100, 164, 209, 118, 22,  213, 91,  136, 120, 202, 104, 134,
      121, 9,   35,  161, 204, 26,  29,  195, 26,  109, 74,  82,  63,  4,   112,
      45,  107, 160, 209, 17,  14,  35,  113, 155, 14,  56,  210, 208, 199, 182,
      230, 199, 49,  36,  46,  26,  222, 9,   122, 180, 133, 53,  88,  227, 151,
      164, 230, 213, 127, 114, 15,  32,  165, 77,  15,  17,  185, 109, 69,  135,
      118, 214, 232, 204, 35,  162, 111, 184, 32,  118, 180, 192, 26,  109, 131,
      12,  250, 16,  32,  123, 201, 21,  109, 122, 136, 84,  239, 10,  81,  94,
      234, 141, 156, 1,   47,  134, 133, 49,  195, 56,  199, 142, 118, 9,   7,
      101, 228, 254, 31,  218, 73,  14,  255, 178, 6,   124, 147, 244, 88,  123,
      58,  176, 192, 128, 107, 36,  207, 132, 17,  100, 239, 226, 40,  134, 216,
      101, 195, 178, 216, 26,  237, 18,  142, 182, 31,  169, 31,  2,   196, 244,
      209, 70,  15,  68,  166, 105, 103, 153, 97,  73,  228, 104, 129, 53,  154,
      183, 233, 18,  2,   255, 135, 124, 56,  83,  101, 89,  195, 232, 170, 5,
      10,  131, 145, 97,  105, 196, 240, 203, 177, 163, 45,  172, 209, 82,  152,
      38,  45,  172, 209, 50,  11,  115, 109, 7,   49,  13,  79,  50,  116, 225,
      49,  150, 97,  121, 248, 48,  75,  223, 163, 99,  88,  131, 44,  66,  255,
      15,  135, 4,   70,  197, 101, 13,  163, 3,   94,  144, 4,   65,  254, 248,
      221, 138, 80,  134, 136, 213, 195, 166, 216, 26,  109, 97,  141, 182, 176,
      104, 210, 194, 194, 218, 161, 35,  175, 151, 7,   212, 53,  58,  224, 69,
      201, 128, 23,  195, 202, 144, 209, 2,   107, 52,  159, 211, 32,  4,   136,
      105, 94,  253, 135, 221, 55,  65,  106, 91,  140, 66,  141, 16,  239, 226,
      183, 20,  38,  75,  255, 101, 13,  163, 3,   94,  248, 251, 224, 12,  171,
      130, 135, 71,  142, 29,  109, 97,  13,  181, 120, 132, 111, 99,  38,  117,
      9,   21,  133, 26,  33,  225, 132, 223, 82,  152, 44,  117, 7,   98,  48,
      77,  163, 98,  243, 141, 146, 150, 11,  158, 114, 153, 114, 99,  169, 62,
      224, 197, 176, 38,  104, 24,  148, 89,  163, 99,  88,  131, 38,  18,  255,
      51,  12,  163, 118, 251, 63,  90,  236, 194, 1,   134, 15,  254, 161, 156,
      209, 1,   47,  72,  18,  194, 25,  74,  107, 3,   24,  130, 55,  12,  233,
      100, 54,  218, 194, 98,  24,  5,   84,  15,  1,   72,  235, 96,  116, 224,
      137, 62,  237, 44,  146, 194, 153, 97,  157, 255, 144, 78,  240, 163, 45,
      172, 193, 17,  125, 255, 135, 85,  177, 9,   204, 168, 4,   170, 122, 176,
      244, 104, 131, 136, 152, 80,  34,  216, 174, 36,  185, 225, 185, 222, 143,
      33,  112, 211, 16,  77,  112, 163, 5,   214, 32,  139, 56,  72,  201, 69,
      198, 22,  63,  242, 52,  34,  23,  148, 212, 219, 87,  72,  252, 178, 134,
      209, 50,  107, 96,  202, 172, 141, 62,  12,  254, 91,  134, 98,  153, 53,
      90,  96,  13,  130, 88,  163, 74,  169, 65,  94,  113, 67,  179, 205, 207,
      88,  75,  34,  226, 5,   41,  111, 86,  144, 220, 238, 128, 37,  4,   242,
      87,  60,  17,  103, 194, 96,  41,  163, 55,  121, 51,  248, 109, 29,  114,
      101, 214, 104, 129, 197, 48,  10,  168, 30,  2,   144, 46,  33,  73,  101,
      22,  176, 208, 30,  113, 87,  207, 131, 195, 157, 158, 133, 56,  19,  218,
      85,  251, 155, 189, 24,  124, 183, 13,  173, 244, 63,  90,  96,  13,  116,
      124, 225, 25,  189, 34,  187, 229, 5,   215, 72,  106, 3,   138, 236, 14,
      41,  106, 40,  98,  142, 97,  81,  222, 98,  26,  237, 60,  50,  160, 22,
      112, 212, 9,   144, 173, 158, 12,  222, 219, 135, 80,  153, 53,  90,  96,
      13,  226, 200, 34,  187, 191, 70,  127, 141, 168, 161, 136, 117, 12,  107,
      180, 204, 98,  32,  177, 73,  69,  167, 50,  122, 155, 59,  131, 215, 206,
      161, 82,  102, 141, 22,  88,  3,   26,  83,  68,  78,  14,  2,   149, 145,
      87,  6,   81,  162, 17,  24,  48,  228, 22,  124, 224, 233, 124, 6,   26,
      29,  136, 60,  186, 194, 139, 242, 162, 31,  221, 132, 237, 110, 12,  158,
      187, 134, 68,  153, 53,  90,  96,  13,  133, 104, 26,  106, 77,  45,  60,
      99,  88,  84,  105, 103, 141, 14,  120, 81,  191, 204, 218, 225, 202, 224,
      177, 123, 240, 103, 134, 209, 2,   107, 224, 226, 104, 120, 173, 189, 66,
      14,  71,  10,  47,  82,  165, 74,  161, 54,  218, 16,  35,  57,  24,  119,
      57,  51,  184, 237, 29,  228, 101, 214, 104, 129, 53,  208, 5,   22,  121,
      75,  174, 232, 166, 139, 172, 224, 65,  238, 18,  142, 14,  150, 67,  130,
      112, 104, 172, 150, 216, 237, 196, 224, 186, 111, 48,  151, 89,  163, 5,
      214, 0,   197, 14,  37,  205, 171, 193, 180, 228, 10,  107, 240, 161, 117,
      9,   71,  203, 172, 161, 84,  102, 237, 113, 100, 112, 217, 63,  104, 203,
      172, 209, 2,   139, 97,  196, 1,   218, 15,  225, 99,  221, 154, 51,  58,
      240, 4,   76,  105, 116, 104, 103, 81,  33,  156, 247, 58,  48,  56,  31,
      24,  156, 249, 98,  180, 192, 26,  136, 120, 193, 108, 94,  97,  138, 16,
      217, 140, 194, 218, 82,  195, 175, 23,  34,  139, 171, 137, 135, 71,  47,
      30,  141, 168, 186, 200, 91,  214, 48,  218, 16,  27,  68,  13,  177, 253,
      118, 12,  142, 135, 6,   97,  153, 53,  90,  96,  13,  142, 72,  25,  66,
      243, 128, 68,  56,  149, 236, 101, 13,  163, 101, 214, 32,  42,  179, 14,
      216, 50,  56,  28,  30,  108, 101, 214, 104, 129, 69,  247, 24,  193, 63,
      122, 69,  118, 127, 109, 48,  165, 44,  74,  150, 53,  140, 150, 89,  131,
      168, 204, 58,  104, 195, 96,  127, 100, 80,  149, 89,  163, 5,   214, 32,
      171, 66,  24,  135, 195, 144, 26,  133, 203, 26,  208, 183, 188, 193, 130,
      100, 104, 12,  0,   97,  184, 22,  40,  128, 223, 229, 131, 218, 191, 135,
      173, 24,  108, 143, 13,  158, 68,  57,  90,  96,  209, 55,  46,  254, 51,
      140, 4,   64,  135, 101, 13,  163, 13,  49,  250, 53,  196, 142, 88,  50,
      216, 28,  31,  36,  233, 118, 180, 192, 162, 123, 129, 69,  82,  27,  10,
      173, 128, 35,  123, 36,  158, 190, 26,  233, 179, 172, 97,  180, 204, 162,
      95,  153, 117, 212, 130, 193, 250, 196, 96,  40,  179, 70,  11,  44,  58,
      198, 2,   25,  205, 43,  242, 122, 136, 3,   61,  132, 143, 117, 89,  195,
      232, 214, 66,  130, 125, 67,  98,  58,  143, 3,   182, 130, 255, 152, 57,
      131, 213, 201, 1,   47,  179, 70,  11,  44,  134, 81,  64,  245, 16,  160,
      226, 69,  170, 196, 228, 225, 209, 21,  94,  148, 23,  133, 68,  133, 243,
      9,   211, 255, 22,  167, 7,   54,  191, 140, 22,  88,  244, 10,  127, 120,
      243, 138, 90,  75,  174, 136, 111, 70,  33,  219, 72,  118, 135, 148, 20,
      235, 112, 45,  107, 24,  237, 196, 209, 175, 19,  7,   75,  215, 88,  195,
      156, 236, 136, 96,  56,  105, 242, 223, 252, 204, 0,   150, 89,  163, 5,
      22,  221, 3,   127, 120, 45,  185, 194, 18,  124, 140, 12,  164, 158, 56,
      74,  159, 6,   194, 232, 118, 104, 170, 132, 51,  195, 41,  227, 255, 102,
      103, 7,   170, 204, 26,  45,  176, 232, 18,  242, 88,  71,  175, 224, 130,
      164, 22,  97,  148, 107, 4,   122, 154, 150, 77,  45,  252, 203, 26,  70,
      87,  45,  208, 173, 157, 69,  163, 206, 50,  195, 25,  195, 255, 38,  231,
      7,   164, 204, 26,  45,  176, 6,   174, 121, 59,  124, 155, 90,  148, 47,
      107, 24,  237, 60,  14,  242, 206, 35,  195, 89,  131, 255, 198, 23,  232,
      159, 121, 70,  11,  44,  218, 135, 249, 200, 88,  123, 133, 28,  142, 84,
      89,  214, 48,  90,  102, 13,  246, 50,  235, 156, 254, 127, 163, 139, 116,
      46,  179, 70,  11,  44,  134, 193, 2,   200, 27,  26,  167, 124, 64,  157,
      188, 14,  41,  94,  93,  212, 90,  214, 48,  90,  102, 13,  246, 50,  235,
      188, 222, 127, 195, 75,  244, 204, 65,  163, 5,   22,  141, 67,  155, 248,
      230, 213, 208, 92,  114, 133, 53,  248, 168, 184, 172, 97,  116, 192, 139,
      110, 101, 22,  121, 3,   94,  12,  23,  117, 254, 235, 95,  161, 91,  153,
      53,  90,  96,  49,  12,  58,  0,   76,  56,  100, 20,  94,  3,   59,  132,
      15,  12,  68,  36,  55,  211, 121, 89,  195, 104, 67,  108, 96,  27,  98,
      12,  151, 180, 255, 235, 93,  165, 79,  62,  26,  45,  176, 104, 25,  206,
      228, 141, 94,  13,  253, 166, 22,  253, 151, 53,  140, 150, 89,  3,   92,
      102, 93,  214, 250, 175, 123, 141, 14,  101, 214, 104, 129, 197, 48,  10,
      168, 30,  2,   152, 99,  88,  64,  43,  224, 101, 10,  141, 10,  151, 209,
      50,  107, 128, 203, 172, 43,  154, 255, 117, 174, 211, 58,  55,  141, 22,
      88,  52,  11,  97,  228, 230, 21,  133, 253, 53,  242, 122, 136, 140, 3,
      86,  20,  3,   199, 176, 104, 180, 2,   8,   127, 169, 55,  58,  224, 69,
      183, 50,  11,  107, 252, 50,  92,  83,  255, 175, 117, 147, 166, 201, 110,
      180, 192, 162, 75,  174, 30,  66,  75,  174, 168, 17,  30,  196, 44,  107,
      24,  109, 16,  13,  108, 131, 136, 70,  13,  94,  134, 235, 106, 255, 53,
      111, 209, 46,  83,  141, 22,  88,  180, 9,   219, 255, 12,  131, 5,   144,
      55,  132, 79,  182, 235, 193, 30,  39,  114, 89,  195, 104, 153, 53,  60,
      203, 172, 27,  170, 255, 53,  110, 211, 40,  253, 143, 22,  88,  3,   81,
      176, 96,  22,  103, 68,  54,  193, 200, 208, 8,   52,  25,  87,  233, 73,
      208, 82,  60,  26,  113, 149, 131, 96,  51,  137, 95,  214, 48,  90,  102,
      13,  207, 50,  235, 150, 242, 127, 181, 187, 180, 200, 90,  163, 5,   22,
      13,  66,  149, 96,  243, 138, 206, 61,  68,  90,  88,  135, 215, 76,  146,
      150, 53,  140, 14,  60,  209, 173, 204, 194, 58,  240, 68,  171, 190, 225,
      109, 165, 255, 170, 247, 168, 158, 187, 70,  11,  44,  134, 161, 7,   232,
      220, 203, 35,  61,  128, 168, 190, 172, 97,  180, 33,  54,  20,  27,  98,
      12,  119, 20,  255, 171, 220, 167, 110, 254, 26,  45,  176, 168, 93,  94,
      209, 97,  244, 138, 113, 176, 151, 177, 180, 88,  214, 48,  90,  102, 13,
      201, 50,  235, 174, 194, 127, 229, 7,   84,  76,  175, 163, 5,   22,  195,
      40,  32,  45,  4,   136, 104, 223, 145, 119, 145, 42,  208, 25,  248, 187,
      39,  163, 101, 214, 144, 44,  179, 238, 203, 253, 87,  124, 68,  173, 92,
      54,  90,  96,  81,  181, 188, 2,   102, 102, 180, 22,  22,  241, 173, 33,
      106, 105, 4,   122, 136, 118, 67,  248, 144, 208, 194, 213, 138, 132, 217,
      139, 107, 12,  139, 96,  145, 68,  76,  153, 69,  207, 129, 24,  228, 2,
      116, 212, 94,  178, 163, 143, 225, 129, 236, 127, 133, 199, 84,  201, 105,
      163, 5,   22,  3,   149, 193, 16,  90,  114, 69,  179, 61,  64,  120, 198,
      176, 40,  47,  179, 70,  27,  98,  244, 105, 103, 81,  55,  156, 25,  30,
      202, 252, 151, 127, 66,  121, 94,  27,  45,  176, 168, 87,  94,  209, 97,
      244, 138, 97,  104, 0,   10,  47,  82,  165, 188, 80,  27,  237, 60,  210,
      167, 80,  35,  41,  156, 25,  30,  73,  253, 151, 123, 70,  97,  10,  30,
      45,  176, 104, 92,  4,   96,  150, 98,  228, 245, 215, 104, 170, 11,  24,
      6,   20,  118, 72,  81,  157, 135, 220, 37,  28,  45,  59,  6,   97,  217,
      129, 53,  82,  200, 139, 41,  210, 202, 172, 39,  146, 255, 101, 158, 83,
      146, 229, 70,  11,  44,  42,  21,  88,  132, 134, 117, 72,  182, 134, 102,
      253, 53,  236, 46,  161, 106, 79,  22,  115, 107, 206, 232, 0,   16,  229,
      205, 70,  98,  250, 104, 131, 63,  156, 25,  158, 74,  252, 151, 126, 65,
      118, 174, 27,  45,  176, 24,  232, 4,   224, 37,  26,  169, 69,  3,   121,
      26,  169, 53,  132, 79,  214, 164, 1,   25,  203, 26,  70,  27,  98,  35,
      167, 33,  198, 240, 76,  252, 191, 212, 75,  242, 242, 221, 104, 129, 69,
      141, 242, 138, 152, 209, 171, 97,  176, 186, 29,  127, 80,  33,  121, 144,
      188, 101, 13,  163, 101, 214, 8,   42,  179, 94,  136, 254, 151, 120, 77,
      70,  222, 27,  45,  176, 24,  70,  1,   213, 67,  128, 236, 101, 13,  163,
      101, 214, 8,   42,  179, 94,  138, 252, 23,  127, 67,  106, 218, 27,  45,
      176, 40,  206, 173, 192, 251, 57,  33,  45,  44,  178, 122, 79,  12,  36,
      181, 188, 40,  236, 87,  146, 215, 27,  37,  189, 109, 72,  201, 178, 6,
      96,  153, 53,  58,  224, 69,  112, 184, 138, 114, 5,   131, 33,  156, 25,
      94,  9,   255, 23,  123, 75,  82,  14,  28,  45,  176, 40,  46,  176, 200,
      232, 235, 209, 185, 123, 8,   244, 34,  125, 135, 240, 105, 189, 172, 97,
      180, 33,  54,  108, 26,  98,  12,  175, 5,   255, 139, 190, 39,  62,  19,
      142, 22,  88,  148, 21,  88,  144, 38,  15,  144, 36,  175, 68,  32,  181,
      105, 6,   119, 236, 192, 14,  225, 19,  42,  1,   233, 176, 172, 97,  180,
      204, 26,  62,  101, 214, 91,  129, 255, 194, 31,  136, 204, 135, 163, 5,
      22,  3,   21,  0,   164, 180, 194, 53,  244, 142, 167, 44,  195, 175, 17,
      79,  185, 0,   212, 8,   47,  43,  113, 121, 0,   171, 189, 4,   53,  226,
      210, 5,   180, 5,   217, 131, 120, 61,  139, 185, 172, 225, 31,  204, 145,
      88,  11,  26,  58,  172, 0,   26,  181, 23,  24,  3,   131, 54,  156, 25,
      222, 241, 255, 23,  250, 72,  76,  78,  28,  45,  176, 40,  40,  175, 208,
      50,  45,  35,  185, 70,  209, 183, 191, 198, 64,  123, 119, 98,  46,  107,
      160, 81,  131, 104, 116, 192, 139, 110, 237, 44,  90,  15,  44,  50,  188,
      231, 251, 47,  248, 137, 96,  22,  26,  45,  176, 24,  168, 15,  40,  236,
      175, 145, 90,  160, 80,  210, 33,  101, 100, 160, 5,   160, 226, 69,  170,
      200, 237, 130, 209, 134, 24,  49,  197, 211, 208, 237, 44,  51,  124, 224,
      249, 47,  240, 5,   127, 130, 28,  45,  176, 200, 205, 176, 255, 113, 107,
      28,  246, 77,  45,  66,  97,  134, 107, 89,  195, 232, 192, 19,  49,  37,
      206, 72,  46,  163, 25,  62,  113, 255, 231, 251, 138, 39,  125, 141, 22,
      88,  12,  195, 7,   80,  50,  132, 79,  201, 164, 1,   48,  4,   81,  181,
      227, 89,  214, 48,  90,  102, 141, 150, 89,  248, 7,   52,  25,  62,  115,
      253, 231, 253, 134, 43,  87,  142, 22,  88,  100, 149, 87,  144, 181, 87,
      196, 100, 114, 50,  70,  226, 33,  46,  34,  163, 5,   7,   113, 15,  25,
      54,  226, 215, 8,   113, 15,  174, 193, 120, 136, 117, 168, 150, 226, 95,
      214, 48,  58,  240, 68,  183, 50,  139, 214, 3,   79,  184, 166, 50,  40,
      180, 151, 225, 11,  199, 127, 158, 31,  88,  115, 230, 104, 129, 69,  86,
      129, 69,  124, 123, 132, 206, 221, 67,  140, 198, 14,  9,   222, 163, 222,
      216, 63,  229, 203, 26,  70,  27,  98,  35,  188, 33,  198, 240, 141, 253,
      63,  215, 79,  204, 212, 59,  90,  96,  145, 94,  96,  253, 35,  171, 140,
      27,  124, 67,  227, 12,  52,  3,   84,  89,  214, 48,  90,  102, 141, 244,
      50,  235, 59,  219, 127, 206, 95,  104, 137, 116, 180, 192, 34,  61,  215,
      2,   91,  34,  212, 237, 118, 225, 111, 218, 224, 178, 142, 236, 14,  41,
      169, 26,  73,  106, 121, 129, 67,  134, 90,  203, 26,  70,  203, 172, 145,
      94,  102, 253, 96,  253, 207, 241, 27,  57,  139, 142, 22,  88,  36,  22,
      88,  144, 162, 138, 206, 29,  189, 1,   233, 87,  146, 231, 83,  112, 241,
      74,  197, 101, 13,  163, 3,   94,  116, 43,  179, 6,   231, 128, 23,  195,
      79,  230, 255, 236, 127, 225, 185, 116, 180, 192, 162, 172, 95,  132, 214,
      212, 34,  190, 100, 33,  79,  35,  214, 150, 29,  141, 154, 90,  16,  99,
      201, 26,  251, 167, 243, 178, 134, 209, 134, 216, 240, 110, 136, 49,  252,
      102, 250, 207, 10,  29,  136, 25,  45,  176, 72,  41,  176, 48,  115, 239,
      72,  104, 106, 145, 211, 105, 6,   133, 20,  174, 41,  164, 209, 109, 58,
      192, 16,  133, 7,   14,  121, 165, 237, 72,  43,  163, 25,  254, 48,  254,
      103, 1,   37,  170, 209, 2,   139, 232, 236, 248, 159, 97,  20,  16,  25,
      2,   76,  12,  255, 33,  165, 18,  157, 203, 172, 193, 217, 169, 161, 81,
      225, 50,  210, 58,  203, 12,  127, 25,  254, 51,  141, 102, 193, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  129, 209, 16,
      24,  13,  129, 209, 16,  24,  13,  129, 209, 16,  24,  13,  1,   6,   6,
      6,   0,   80,  239, 41,  193, 158, 154, 228, 108, 0,   0,   0,   0,   73,
      69,  78,  68,  174, 66,  96,  130};

  REQUIRE(l_png_frame.is_contained_by(l_frame_expected.range()));

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

  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                   "rast.depth.comparison.readonly.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  container::arr<ui8, 90> l_frame_expected = {
      137, 80,  78,  71,  13,  10, 26,  10,  0,  0,   0,   13,  73, 72, 68,
      82,  0,   0,   0,   8,   0,  0,   0,   8,  8,   2,   0,   0,  0,  75,
      109, 41,  220, 0,   0,   0,  33,  73,  68, 65,  84,  120, 94, 99, 64,
      128, 255, 12,  200, 128, 9,  133, 135, 36, 199, 8,   149, 64, 86, 14,
      22,  3,   19,  168, 134, 48, 208, 13,  0,  0,   101, 117, 4,  2,  164,
      193, 226, 131, 0,   0,   0,  0,   73,  69, 78,  68,  174, 66, 96, 130};

  REQUIRE(l_png_frame.is_contained_by(l_frame_expected.range()));

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

  container::range<ui8> l_png_frame;
  {
    auto *l_frame_texture =
        s_bgfx_impl.proxy().FrameBuffer(l_frame_buffer).RGBTexture().value();

#if WRITE_OUTPUT
    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/"
                   "rast.depth.comparison.outofbounds.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
#endif

    i32 l_length;
    l_png_frame.m_begin = stbi_write_png_to_mem(
        l_frame_texture->range().m_begin,
        l_frame_texture->info.bitsPerPixel * l_frame_texture->info.width,
        l_frame_texture->info.width, l_frame_texture->info.height, 3,
        &l_length);
    l_png_frame.m_count = l_length;
  }

  container::arr<ui8, 114> l_frame_expected = {
      137, 80,  78,  71,  13,  10,  26,  10,  0,   0,  0,  13,  73,  72,  68,
      82,  0,   0,   0,   8,   0,   0,   0,   8,   8,  2,  0,   0,   0,   75,
      109, 41,  220, 0,   0,   0,   57,  73,  68,  65, 84, 120, 94,  99,  100,
      128, 129, 255, 12,  8,   0,   20,  197, 46,  1,  84, 194, 4,   85,  134,
      172, 30,  44,  196, 196, 128, 3,   128, 141, 66, 86, 14,  100, 131, 21,
      51,  50,  160, 25,  2,   179, 148, 5,   197, 41, 72, 198, 194, 236, 96,
      68,  183, 10,  0,   241, 4,   6,   15,  98,  30, 98, 190, 0,   0,   0,
      0,   73,  69,  78,  68,  174, 66,  96,  130};

  REQUIRE(l_png_frame.is_contained_by(l_frame_expected.range()));

  bgfx::destroy(l_index_buffer);
  bgfx::destroy(l_vertex_buffer);
  bgfx::destroy(l_program);

  l_triangle_vertices.free();

  bgfx::shutdown();
}

#undef WRITE_OUTPUT