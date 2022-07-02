
#include <tst/test_engine_common.hpp>

inline static constexpr auto TEST_RAST_RELATIVE_FOLDER =
    container::arr_literal<ui8>(TEST_RESOURCE_PATH_RAW_PREPROCESS "rast/");

inline static constexpr auto TEST_RAST_TMP_FOLDER =
    container::arr_literal<ui8>("/media/loic/SSD/SoftwareProjects/glm/");

static constexpr TestImageAssertionConfig s_resource_config =
    TestImageAssertionConfig::make(TEST_RAST_TMP_FOLDER.range(),
                                   TEST_RAST_RELATIVE_FOLDER.range());

TEST_CASE("rast.single_triangle.visibility") {
  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
f 1 2 3
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<WhiteShader>());

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.single_triangle.visibility.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("rast.single_triangle.vertex_color_interpolation") {
  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
vc 0 0 0
vc 0 255 0
vc 255 255 0
f 1/1 2/2 3/3
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>(
      "rast.single_triangle.vertex_color_interpolation.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("rast.cull.clockwise.counterclockwise") {

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
v 0.0 0.0 0.0
v 0.0 -1.0 0.0
v -1.0 0.0 0.0
f 1 2 3
f 4 5 6
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  ren::shader_meta l_c_meta = ren::shader_meta::get_default();
  l_c_meta.m_cull_mode = ren::shader_meta::cull_mode::clockwise;

  ren::shader_meta l_cc_meta = ren::shader_meta::get_default();
  l_cc_meta.m_cull_mode = ren::shader_meta::cull_mode::cclockwise;

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<WhiteShader>(l_c_meta));

  l_test.update();

  auto l_c_path = container::arr_literal<ui8>("rast.cull.clockwise.png");
  TestUtils::assert_frame_equals(l_c_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);

  // l_test.destroy_mesh_renderer(l_mesh_renderer);

  l_test.l_scene.mesh_renderer(l_mesh_renderer)
      .set_program(l_test.create_shader<WhiteShader>(l_cc_meta));

  l_test.update();

  auto l_cc_path = container::arr_literal<ui8>("rast.cull.cclockwise.png");
  TestUtils::assert_frame_equals(l_cc_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

#if 0

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
      l_view = m::look_at(eye, at, position_t::up);
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

#endif

#include <sys/sys_impl.hpp>