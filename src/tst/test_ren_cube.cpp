#include <assets/loader/mesh_obj.hpp>
#include <doctest.h>
#include <eng/scene.hpp>
#include <m/const.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/impl/ren_impl.hpp>
#include <tst/test_common.hpp>

inline static constexpr auto TEST_REN_RELATIVE_FOLDER =
    container::arr_literal<ui8>(TEST_RESOURCE_PATH_RAW_PREPROCESS "ren/");

inline static constexpr auto TEST_REN_TMP_FOLDER =
    container::arr_literal<ui8>("/media/loic/SSD/SoftwareProjects/glm/");

static constexpr TestImageAssertionConfig s_resource_config =
    TestImageAssertionConfig::make(TEST_REN_TMP_FOLDER.range(),
                                   TEST_REN_RELATIVE_FOLDER.range());

namespace RasterizerTestToolbox {

template <typename Engine>
inline static ren::shader_handle
load_shader(eng::engine_api<Engine> p_engine,
            const ren::shader_meta &p_shader_meta,
            const container::range<rast::shader_vertex_output_parameter>
                &p_vertex_output,
            rast::shader_vertex_function p_vertex,
            rast::shader_fragment_function p_fragment) {

  api_decltype(rast_api, l_rast, p_engine.rasterizer());
  api_decltype(ren::ren_api, l_ren, p_engine.renderer());
  return l_ren.create_shader(p_shader_meta, p_vertex_output, p_vertex,
                             p_fragment, l_rast);
};

template <typename Shader, typename Engine>
inline ren::shader_handle load_shader(eng::engine_api<Engine> p_engine,
                                      const ren::shader_meta &p_meta) {
  ren::shader_meta l_meta = l_meta.get_default();
  return RasterizerTestToolbox::load_shader(p_engine, p_meta,
                                            Shader::s_vertex_output.range(),
                                            Shader::vertex, Shader::fragment);
};

}; // namespace RasterizerTestToolbox

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

struct BaseRenCubeTest {

  using engine_t =
      eng::details::engine<ren::details::ren_impl, rast_impl_software>;
  using scene_t = eng::scene<engine_t>;
  engine_t __engine;
  scene_t l_scene;

  ui16 m_width;
  ui16 m_height;

  ren::mesh_handle m_mesh_handle;
  ren::shader_handle m_shader;

  container::vector<eng::object_handle> m_cameras;
  container::vector<eng::object_handle> m_mesh_renderers;

  BaseRenCubeTest(ui16 p_width, ui16 p_height) {
    __engine.allocate(p_width, p_height);
    api_decltype(eng::engine_api, l_engine, __engine);
    l_scene = {&__engine};
    l_scene.allocate();
    m_width = p_width;
    m_height = p_height;

    m_cameras.allocate(0);
    m_mesh_renderers.allocate(0);

    auto l_cube_mesh =
        assets::obj_mesh_loader{}.compile(l_cube_mesh_obj.range());
    m_mesh_handle = l_engine.renderer_api().create_mesh(
        l_cube_mesh, l_engine.rasterizer_api());
    l_cube_mesh.free();

    ren::shader_meta l_meta = {
        .m_cull_mode = ren::shader_meta::cull_mode::cclockwise,
        .m_write_depth = 1,
        .m_depth_test = ren::shader_meta::depth_test::less};

    m_shader = RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
        l_engine, l_meta);
  }

  ~BaseRenCubeTest() {
    api_decltype(rast_api, l_rast, __engine.m_rasterizer);
    api_decltype(ren::ren_api, l_ren, __engine.m_renderer);
    l_ren.destroy(m_shader, l_rast);
    l_ren.destroy(m_mesh_handle, l_rast);

    for (auto i = 0; i < m_cameras.count(); ++i) {
      l_scene.camera_destroy(m_cameras.at(i));
    }

    for (auto i = 0; i < m_mesh_renderers.count(); ++i) {
      l_scene.mesh_renderer_destroy(m_mesh_renderers.at(i));
    }
    m_cameras.free();
    m_mesh_renderers.free();

    l_scene.free();
    __engine.free();
  };

  void update() {
    api_decltype(eng::engine_api, l_engine, __engine);
    l_engine.update([&]() { l_scene.update(); });
  };

  eng::object_handle create_perspective_camera() {
    eng::object_handle l_camera = l_scene.camera_create();
    eng::camera_view<scene_t> l_camera_view = l_scene.camera(l_camera);
    l_camera_view.set_width_height(m_width, m_height);
    l_camera_view.set_render_width_height(m_width, m_height);
    l_camera_view.set_perspective(60.0f * m::deg_to_rad, 0.1, 50);
    m_cameras.push_back(l_camera);
    return l_camera;
  };

  eng::object_handle create_orthographic_camera() {
    api_decltype(eng::engine_api, l_engine, __engine);
    eng::object_handle l_camera = l_scene.camera_create();
    eng::camera_view<scene_t> l_camera_view = l_scene.camera(l_camera);
    l_camera_view.set_width_height(m_width, m_height);
    l_camera_view.set_render_width_height(m_width, m_height);
    l_camera_view.set_orthographic(5, 5, 0.1, 50);
    m_cameras.push_back(l_camera);
    return l_camera;
  };

  eng::object_handle create_mesh_renderer() {
    eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
    l_scene.mesh_renderer(l_mesh_renderer).set_mesh(m_mesh_handle);
    l_scene.mesh_renderer(l_mesh_renderer).set_program(m_shader);
    m_mesh_renderers.push_back(l_mesh_renderer);
    return l_mesh_renderer;
  };
};

// look at back face (camera forward is +z)
TEST_CASE("ren.cube.face.back") {
  constexpr ui16 l_width = 64, l_height = 64;

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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

  BaseRenCubeTest l_test = BaseRenCubeTest(l_width, l_height);
  eng::object_handle l_camera = l_test.create_orthographic_camera();
  eng::object_handle l_mesh_renderer = l_test.create_mesh_renderer();

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