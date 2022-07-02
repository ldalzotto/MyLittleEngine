#include <assets/loader/mesh_obj.hpp>
#include <doctest.h>
#include <eng/scene.hpp>
#include <m/const.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/impl/ren_impl.hpp>
#include <tst/test_engine_common.hpp>

inline static constexpr auto TEST_REN_RELATIVE_FOLDER =
    container::arr_literal<ui8>(TEST_RESOURCE_PATH_RAW_PREPROCESS "ren/");

inline static constexpr auto TEST_REN_TMP_FOLDER =
    container::arr_literal<ui8>("/media/loic/SSD/SoftwareProjects/glm/");

static constexpr TestImageAssertionConfig s_resource_config =
    TestImageAssertionConfig::make(TEST_REN_TMP_FOLDER.range(),
                                   TEST_REN_RELATIVE_FOLDER.range());

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
};

static constexpr auto l_cube_mesh_obj = container::arr_literal<ui8>(R""""(
v -1.0 1.0 1.0
v 1.0 1.0 1.0
v -1.0 -1.0 1.0
v 1.0 -1.0 1.0
v -1.0 1.0 -1.0
v 1.0 1.0 -1.0
v -1.0 -1.0 -1.0
v 1.0 -1.0 -1.0
vc 0 0 0
vc 0 0 255
vc 0 255 0
vc 0 255 255
vc 255 0 0
vc 255 0 255
vc 255 255 0
vc 255 255 255
f 1/1 2/2 3/3
f 2/2 4/4 3/3
f 5/5 7/7 6/6
f 6/6 7/7 8/8
f 1/1 3/3 5/5
f 5/5 3/3 7/7
f 2/2 6/6 4/4
f 6/6 8/8 4/4
f 1/1 5/5 2/2
f 5/5 6/6 2/2
f 3/3 4/4 7/7
f 7/7 4/4 8/8
  )"""");

// look at back face (camera forward is +z)
TEST_CASE("ren.cube.face.back") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -10});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::quat<fix32>::getIdentity());

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.faces.back.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

// look at front face (camera forward is -z)
TEST_CASE("ren.cube.face.front") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({0, 0, 10});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi<fix32>(), position_t::up));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.faces.front.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

// look at right face (camera forward is +x)
TEST_CASE("ren.cube.face.right") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({-10, 0, 0});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_2<fix32>(), position_t::up));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.faces.right.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

// look at left face (camera forward is -x)
TEST_CASE("ren.cube.face.left") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({10, 0, 0});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_2<fix32>(), position_t::up));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.faces.left.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

// look at down face (camera forward is +y)
TEST_CASE("ren.cube.face.down") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({0, -10, 0});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_2<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.faces.down.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

// look at up face (camera forward is -y)
TEST_CASE("ren.cube.face.up") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({0, 10, 0});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_2<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.faces.up.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.up.0") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({-5, 7.5, -5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.up.0.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.up.1") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({5, 7.5, -5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.up.1.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.up.2") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({5, 7.5, 5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_2<fix32>() - m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.up.2.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.up.3") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({-5, 7.5, 5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_2<fix32>() + m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.up.3.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.down.0") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({-5, -7.5, -5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(-m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.down.0.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.down.1") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({5, -7.5, -5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(-m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.down.1.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.down.2") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({5, -7.5, 5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_2<fix32>() - m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(-m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.down.2.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

TEST_CASE("ren.cube.corner.down.3") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_cube_mesh_obj.range()),
      l_test.create_shader<ColorInterpolationShader>());

  l_test.l_scene.camera(l_camera).set_local_position({-5, -7.5, 5});
  l_test.l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_2<fix32>() + m::pi_4<fix32>(), position_t::up) *
      m::rotate_around(-m::pi_4<fix32>(), position_t::left));

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("ren.cube.corner.down.3.png");
  TestUtils::assert_frame_equals(l_tmp_path.range(),
                                 eng::engine_api{l_test.__engine}, l_width,
                                 l_height, s_resource_config);
}

#include <sys/sys_impl.hpp>