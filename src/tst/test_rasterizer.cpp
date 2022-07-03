
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
v 0.0 1.0 0.0
v 1.0 0.0 0.0
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
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

TEST_CASE("rast.single_triangle.vertex_color_interpolation") {
  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 0.0 1.0 0.0
v 1.0 0.0 0.0
vc 0 0 0
vc 255 255 0
vc 0 255 0
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
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
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

  ren::program_meta l_c_meta = ren::program_meta::get_default();
  l_c_meta.m_cull_mode = ren::program_meta::cull_mode::clockwise;

  ren::program_meta l_cc_meta = ren::program_meta::get_default();
  l_cc_meta.m_cull_mode = ren::program_meta::cull_mode::cclockwise;

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<WhiteShader>(l_c_meta));

  l_test.update();

  auto l_c_path = container::arr_literal<ui8>("rast.cull.clockwise.png");
  l_test.assert_frame_equals(l_c_path.range(), s_resource_config);

  l_test.l_scene.mesh_renderer(l_mesh_renderer)
      .set_program(l_test.create_shader<WhiteShader>(l_cc_meta));

  l_test.update();

  auto l_cc_path = container::arr_literal<ui8>("rast.cull.cclockwise.png");
  l_test.assert_frame_equals(l_cc_path.range(), s_resource_config);
}

TEST_CASE("rast.depth.comparison") {

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 0.0 1.0 0.0
v 1.0 0.0 0.0
v -0.5 0.0 0.1
v 0.0 1.0 0.1
v 1.0 0.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("rast.depth.comparison.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

TEST_CASE("rast.depth.comparison.large_framebuffer") {
  constexpr ui16 l_width = 400, l_height = 400;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 0.0 1.0 0.0
v 1.0 0.0 0.0
v -0.5 0.0 0.1
v 0.0 1.0 0.1
v 1.0 0.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>(
      "rast.depth.comparison.large_framebuffer.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

TEST_CASE("rast.depth.comparison.readonly") {
  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 0.0 1.0 0.0
v 1.0 0.0 0.0
v -0.5 0.0 0.1
v 0.0 1.0 0.1
v 1.0 0.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  ren::program_meta l_meta = l_meta.get_default();
  l_meta.m_write_depth = 0;

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<ColorInterpolationShader>(l_meta));

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.depth.comparison.readonly.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

// top left-right out of bounds
TEST_CASE("rast.depth.comparison.outofbounds") {

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 0.0 2.0 0.0
v 2.0 0.0 0.0
v -2.0 -2.0 0.1
v 0.0 1.0 0.1
v 1.0 0.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.depth.comparison.outofbounds.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

#include <sys/sys_impl.hpp>