#include <assets/loader/mesh_obj.hpp>
#include <doctest.h>
#include <eng/scene.hpp>
#include <m/const.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/impl/ren_impl.hpp>
#include <tst/test_common.hpp>

#define WRITE_OUTPUT_TO_TMP 0
#define WRITE_OUTPUT_TO_RESULT 0

inline static constexpr auto TEST_REN_RELATIVE_FOLDER =
    container::arr_literal<ui8>(TEST_RESOURCE_PATH_RAW "ren/");

inline static constexpr auto TEST_REN_TMP_FOLDER =
    container::arr_literal<ui8>("/media/loic/SSD/SoftwareProjects/glm/");

inline static container::span<ui8>
test_file_path_null_terminated(const container::range<ui8> &p_left,
                               const container::range<ui8> &p_relative_path) {
  container::span<ui8> l_tmp_path_null_terminated;
  l_tmp_path_null_terminated.allocate(p_left.count() + p_relative_path.count() +
                                      1);
  l_tmp_path_null_terminated.range().copy_from(p_left);
  l_tmp_path_null_terminated.range()
      .slide(p_left.size_of())
      .copy_from(p_relative_path);
  l_tmp_path_null_terminated.at(l_tmp_path_null_terminated.count() - 1) = '\0';
  return l_tmp_path_null_terminated;
};

namespace RasterizerTestToolbox {

template <typename Engine>
inline static void
assert_frame_equals(const container::range<ui8> &p_relative_path,
                    eng::engine_api<Engine> p_engine, ui16 p_width,
                    ui16 p_height) {
  container::range<ui8> l_png_frame;
  container::span<rgb_t> l_frame_buffer_rgb;
  {

    container::range<rgba_t> p_frame_buffer_rgba =
        p_engine.window_system()
            .window_get_image_buffer(p_engine.thiz.m_window)
            .m_data.range()
            .template cast_to<rgba_t>();

    l_frame_buffer_rgb.allocate(p_frame_buffer_rgba.count());
    for (auto i = 0; i < p_frame_buffer_rgba.count(); ++i) {
      l_frame_buffer_rgb.at(i) = p_frame_buffer_rgba.at(i).xyz();
    }

    if (WRITE_OUTPUT_TO_TMP) {
      container::span<ui8> l_tmp_path_null_terminated =
          test_file_path_null_terminated(TEST_REN_TMP_FOLDER.range(),
                                         p_relative_path);
      TestUtils::write_png((const ui8 *)l_tmp_path_null_terminated.data(),
                           p_width, p_height, 3,
                           (ui8 *)l_frame_buffer_rgb.data(), 3 * p_width);
      l_tmp_path_null_terminated.free();
    }

    if (WRITE_OUTPUT_TO_RESULT) {
      container::span<ui8> l_tmp_path_null_terminated =
          test_file_path_null_terminated(TEST_REN_RELATIVE_FOLDER.range(),
                                         p_relative_path);
      TestUtils::write_png((const ui8 *)l_tmp_path_null_terminated.data(),
                           p_width, p_height, 3,
                           (ui8 *)l_frame_buffer_rgb.data(), 3 * p_width);
      l_tmp_path_null_terminated.free();
    }

    l_png_frame =
        TestUtils::write_png_to_mem((const ui8 *)l_frame_buffer_rgb.data(),
                                    3 * p_width, p_width, p_height, 3);
  }

  if (!WRITE_OUTPUT_TO_TMP) {

    container::span<ui8> l_expected_path_null_terminated =
        test_file_path_null_terminated(TEST_REN_RELATIVE_FOLDER.range(),
                                       p_relative_path);
    i32 l_width, l_height, l_channel;
    container::range<ui8> l_expected_frame;
    l_expected_frame.m_begin =
        TestUtils::read_png((const ui8 *)l_expected_path_null_terminated.data(),
                            &l_width, &l_height, &l_channel, 0);
    l_expected_frame.m_count = l_width * l_height * l_channel;
    l_expected_path_null_terminated.free();

    // REQUIRE(l_png_frame.count() == l_expected_frame.count());
    REQUIRE(l_expected_frame.is_contained_by(
        l_frame_buffer_rgb.range().cast_to<ui8>()));

    TestUtils::read_free(l_expected_frame.m_begin);
  }
  TestUtils::write_free(l_png_frame.m_begin);

  l_frame_buffer_rgb.free();
};

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

  BaseRenCubeTest(ui16 p_width, ui16 p_height) {
    __engine.allocate(p_width, p_height);
    api_decltype(eng::engine_api, l_engine, __engine);
    l_scene = {&__engine};
    l_scene.allocate();
    m_width = p_width;
    m_height = p_height;

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

  ~BaseRenCubeTest(){

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
    return l_camera;
  };

  eng::object_handle create_orthographic_camera() {
    api_decltype(eng::engine_api, l_engine, __engine);
    eng::object_handle l_camera = l_scene.camera_create();
    eng::camera_view<scene_t> l_camera_view = l_scene.camera(l_camera);
    l_camera_view.set_width_height(m_width, m_height);
    l_camera_view.set_render_width_height(m_width, m_height);
    l_camera_view.set_orthographic(5, 5, 0.1, 50);
    return l_camera;
  };

  eng::object_handle create_mesh_renderer() {
    eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
    l_scene.mesh_renderer(l_mesh_renderer).set_mesh(m_mesh_handle);
    l_scene.mesh_renderer(l_mesh_renderer).set_program(m_shader);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
}

#if 1
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
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
  RasterizerTestToolbox::assert_frame_equals(
      l_tmp_path.range(), eng::engine_api{l_test.__engine}, l_width, l_height);
}
#endif

#include <sys/sys_impl.hpp>