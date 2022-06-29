#include <assets/loader/mesh_obj.hpp>
#include <doctest.h>
#include <eng/scene.hpp>
#include <m/const.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/impl/ren_impl.hpp>
#include <tst/test_common.hpp>

#define WRITE_OUTPUT 1

namespace RasterizerTestToolbox {

template <typename ExpectedFrameType, typename Engine>
inline static void
assert_frame_equals(const i8 *p_save_path, eng::engine_api<Engine> p_engine,
                    ui16 p_width, ui16 p_height,
                    const ExpectedFrameType &p_expected_frame) {
  container::range<ui8> l_png_frame;
  {

    container::range<rgba_t> p_frame_buffer_rgba =
        p_engine.window_system()
            .window_get_image_buffer(p_engine.thiz.m_window)
            .m_data.range()
            .template cast_to<rgba_t>();

    container::span<rgb_t> l_frame_buffer_rgb;
    l_frame_buffer_rgb.allocate(p_frame_buffer_rgba.count());
    for (auto i = 0; i < p_frame_buffer_rgba.count(); ++i) {
      l_frame_buffer_rgb.at(i) = p_frame_buffer_rgba.at(i).xyz();
    }

#if WRITE_OUTPUT
    TestUtils::write_png((const ui8 *)p_save_path, p_width, p_height, 3,
                         (ui8 *)l_frame_buffer_rgb.data(), 3 * p_width);
#endif

    l_png_frame =
        TestUtils::write_png_to_mem((const ui8 *)l_frame_buffer_rgb.data(),
                                    3 * p_width, p_width, p_height, 3);

    l_frame_buffer_rgb.free();
  }

#if !WRITE_OUTPUT
  REQUIRE(l_png_frame.count() == p_expected_frame.count());
#endif
  TestUtils::write_free(l_png_frame.m_begin);
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

TEST_CASE("ren.cube.faces") {
  constexpr ui16 l_width = 64, l_height = 64;

  using engine_t =
      eng::details::engine<ren::details::ren_impl, rast_impl_software>;
  engine_t __engine;
  __engine.allocate(l_width, l_height);
  eng::engine_api<engine_t> l_engine = {__engine};

  using scene_t = eng::scene<engine_t>;
  scene_t l_scene{&__engine};
  l_scene.allocate();

  eng::object_handle l_camera = l_scene.camera_create();
  eng::camera_view<scene_t> l_camera_view = l_scene.camera(l_camera);
  l_camera_view.set_width_height(l_width, l_height);
  l_camera_view.set_render_width_height(l_width, l_height);
  l_camera_view.set_perspective(60.0f * m::deg_to_rad, 0.1, 50);

  auto l_cube_mesh = assets::obj_mesh_loader{}.compile(l_cube_mesh_obj.range());
  ren::mesh_handle l_mesh_handle = l_engine.renderer_api().create_mesh(
      l_cube_mesh, l_engine.rasterizer_api());
  l_cube_mesh.free();

  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          l_engine, ren::shader_meta::get_default());

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  l_scene.mesh_renderer(l_mesh_renderer).set_mesh(l_mesh_handle);
  l_scene.mesh_renderer(l_mesh_renderer).set_program(l_shader_c);

  // look at front face (camera look at -z)
  l_scene.camera(l_camera).set_local_position({0, 0, 10});
  l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi<fix32>(), position_t::up));

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "ren.cube.faces.front.png",
      l_engine, l_width, l_height, container::range<ui8>::make(0, 0));

  // look at back face (camera look at z)
  l_scene.camera(l_camera).set_local_position({0, 0, -10});
  l_scene.camera(l_camera).set_local_rotation(m::quat<fix32>::getIdentity());

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "ren.cube.faces.back.png",
      l_engine, l_width, l_height, container::range<ui8>::make(0, 0));

  // look at left face (camera look at -x)
  l_scene.camera(l_camera).set_local_position({10, 0, 0});
  l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_2<fix32>(), position_t::up));

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "ren.cube.faces.left.png",
      l_engine, l_width, l_height, container::range<ui8>::make(0, 0));

  // look at right face (camera look at +x)
  l_scene.camera(l_camera).set_local_position({-10, 0, 0});
  l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_2<fix32>(), position_t::up));

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "ren.cube.faces.right.png",
      l_engine, l_width, l_height, container::range<ui8>::make(0, 0));

  // look at up face (camera look at -y)
  l_scene.camera(l_camera).set_local_position({0, 10, 0});
  l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(m::pi_2<fix32>(), position_t::left));

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "ren.cube.faces.up.png",
      l_engine, l_width, l_height, container::range<ui8>::make(0, 0));

  // look at bottom face (camera look at +y)
  l_scene.camera(l_camera).set_local_position({0, -10, 0});
  l_scene.camera(l_camera).set_local_rotation(
      m::rotate_around(-m::pi_2<fix32>(), position_t::left));

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "ren.cube.faces.bottom.png",
      l_engine, l_width, l_height, container::range<ui8>::make(0, 0));

  // l_camera_view.set_local_position({0.0f, 5.0f, 35.0f});
}

#include <sys/sys_impl.hpp>