#include "doctest.h"

#include <assets/loader/mesh_obj.hpp>
#include <eng/engine.hpp>
#include <eng/scene.hpp>
#include <rast/impl/rast_impl.hpp>
#include <tst/test_rasterizer_assets.hpp>
#include <m/const.hpp>

#define WRITE_OUTPUT 1

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

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
    stbi_write_png(p_save_path, p_width, p_height, 3,
                   (ui8 *)l_frame_buffer_rgb.data(), 3 * p_width);
#endif

    i32 l_length;
    l_png_frame.m_begin =
        stbi_write_png_to_mem((ui8 *)l_frame_buffer_rgb.data(), 3 * p_width,
                              p_width, p_height, 3, &l_length);
    l_png_frame.m_count = l_length;

    l_frame_buffer_rgb.free();
  }

  REQUIRE(l_png_frame.count() == p_expected_frame.count());
  REQUIRE(l_png_frame.is_contained_by(p_expected_frame.range()));

  STBIW_FREE(l_png_frame.m_begin);
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
};

static constexpr ren::shader_meta s_cclockwise_shader_meta = {
    .m_cull_mode = ren::shader_meta::cull_mode::cclockwise,
    .m_write_depth = 0,
    .m_depth_test = ren::shader_meta::depth_test::less};
static constexpr ren::shader_meta s_cclockwise_write_less_shader_meta = {
    .m_cull_mode = ren::shader_meta::cull_mode::cclockwise,
    .m_write_depth = 1,
    .m_depth_test = ren::shader_meta::depth_test::less};
static constexpr ren::shader_meta s_cclockwise_nowrite_less_shader_meta = {
    .m_cull_mode = ren::shader_meta::cull_mode::cclockwise,
    .m_write_depth = 0,
    .m_depth_test = ren::shader_meta::depth_test::less};
static constexpr ren::shader_meta s_clockwise_shader_meta = {
    .m_cull_mode = ren::shader_meta::cull_mode::clockwise,
    .m_write_depth = 0,
    .m_depth_test = ren::shader_meta::depth_test::less};
static constexpr ren::shader_meta s_clockwise_write_less_shader_meta = {
    .m_cull_mode = ren::shader_meta::cull_mode::clockwise,
    .m_write_depth = 1,
    .m_depth_test = ren::shader_meta::depth_test::less};

TEST_CASE("rast.single_triangle.visibility") {

  constexpr ui16 l_width = 8, l_height = 8;

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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
f 1 2 3
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();
  ren::shader_handle l_shader_handle =
      RasterizerTestToolbox::load_shader<WhiteShader>(
          eng::engine_api<engine_t>{__engine}, s_cclockwise_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_handle);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.visibility.png",
      eng::engine_api<engine_t>{__engine}, l_width, l_height,
      frame_expected::rast_single_triangle_visibility());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_handle, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

TEST_CASE("rast.single_triangle.vertex_color_interpolation") {
  constexpr ui16 l_width = 8, l_height = 8;
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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
vc 0 0 0
vc 0 255 0
vc 255 255 0
f 1/1 2/2 3/3
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();
  ren::shader_handle l_shader_handle =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          eng::engine_api<engine_t>{__engine}, s_cclockwise_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_handle);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.vertex_color_interpolation.png",
      eng::engine_api<engine_t>{__engine}, l_width, l_height,
      frame_expected::rast_single_triangle_vertex_color_interpolation());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_handle, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

TEST_CASE("rast.cull.clockwise.counterclockwise") {

  constexpr ui16 l_width = 8, l_height = 8;
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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
v 0.0 0.0 0.0
v 0.0 -1.0 0.0
v -1.0 0.0 0.0
f 1 2 3
f 4 5 6
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();

  ren::shader_handle l_shader_cc =
      RasterizerTestToolbox::load_shader<WhiteShader>(l_engine,
                                                      s_cclockwise_shader_meta);
  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<WhiteShader>(l_engine,
                                                      s_clockwise_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_c);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.cull.clockwise.png",
      l_engine, l_width, l_height, frame_expected::rast_cull_clockwise());

  l_scene.mesh_renderer(l_mesh_renderer).set_program(l_shader_cc);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.cull.counterclockwise.png",
      l_engine, l_width, l_height,
      frame_expected::rast_cull_counterclockwise());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_c, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_cc, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

TEST_CASE("rast.depth.comparison") {

  constexpr ui16 l_width = 8, l_height = 8;
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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
v -0.5 0.0 0.1
v 1.0 0.0 0.1
v 0.0 1.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();

  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          l_engine, s_cclockwise_write_less_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_c);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.png",
      l_engine, l_width, l_height, frame_expected::rast_depth_comparison());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_c, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

TEST_CASE("rast.depth.comparison.large_framebuffer") {

  constexpr ui16 l_width = 400, l_height = 400;
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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
v -0.5 0.0 0.1
v 1.0 0.0 0.1
v 0.0 1.0 0.1
vc 255 0 0
vc 0 255 0
vc 0 0 255
f 1/1 2/2 3/3
f 4/2 5/2 6/2
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();

  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          l_engine, s_cclockwise_write_less_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_c);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.large_framebuffer.png",
      l_engine, l_width, l_height,
      frame_expected::rast_depth_comparison_large_framebuffer());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_c, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

TEST_CASE("rast.depth.comparison.readonly") {

  constexpr ui16 l_width = 8, l_height = 8;
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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
v -0.5 0.0 0.1
v 1.0 0.0 0.1
v 0.0 1.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();

  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          l_engine, s_cclockwise_nowrite_less_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_c);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.readonly.png",
      l_engine, l_width, l_height,
      frame_expected::rast_depth_comparison_readonly());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_c, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

// top left-right out of bounds
TEST_CASE("rast.depth.comparison.outofbounds") {

  constexpr ui16 l_width = 8, l_height = 8;
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
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());

  auto l_mesh_raw = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0 0.0 0.0
v 2.0 0.0 0.0
v 0.0 2.0 0.0
v -2.0 -2.0 0.1
v 1.0 0.0 0.1
v 0.0 1.0 0.1
vc 255 0 0
vc 0 255 0
f 1/1 2/1 3/1
f 4/2 5/2 6/2
  )"""");

  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();

  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          l_engine, s_cclockwise_write_less_shader_meta);

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_c);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.depth.comparison.outofbounds.png",
      l_engine, l_width, l_height,
      frame_expected::rast_depth_comparison_outofbounds());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_c, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

TEST_CASE("rast.3Dcube") {

  constexpr ui16 l_width = 128, l_height = 128;
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
  l_camera_view.set_perspective(60.0f * m::deg_to_rad, 0.1, 100);
  l_camera_view.set_local_position({0.0f, 0.0f, -35.0f});

  auto l_cube_mesh_obj = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
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

  auto l_cube_mesh = assets::obj_mesh_loader{}.compile(l_cube_mesh_obj.range());
  ren::mesh_handle l_mesh_handle = l_engine.renderer_api().create_mesh(
      l_cube_mesh, l_engine.rasterizer_api());
  l_cube_mesh.free();

  ren::shader_handle l_shader_c =
      RasterizerTestToolbox::load_shader<ColorInterpolationShader>(
          l_engine, s_clockwise_write_less_shader_meta);

  container::vector<eng::object_handle> l_mesh_renderers;
  l_mesh_renderers.allocate(0);
  // 11x11 cubes.
  for (uint32_t yy = 0; yy < 11; ++yy) {
    for (uint32_t xx = 0; xx < 11; ++xx) {
      eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
      l_scene.mesh_renderer(l_mesh_renderer).set_mesh(l_mesh_handle);
      l_scene.mesh_renderer(l_mesh_renderer).set_program(l_shader_c);
      l_scene.mesh_renderer(l_mesh_renderer)
          .set_local_position({-15.0f + xx * 3.0f, -15.0f + yy * 3.0f, 0});
      l_mesh_renderers.push_back(l_mesh_renderer);
    }
  }

  __engine.update([&]() { l_scene.update(); });

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.3Dcube.png",
      l_engine, l_width, l_height,
      frame_expected::rast_depth_comparison_outofbounds());

  for (auto l_mesh_renderer_it = 0;
       l_mesh_renderer_it < l_mesh_renderers.count(); ++l_mesh_renderer_it) {
    l_scene.mesh_renderer_destroy(l_mesh_renderers.at(l_mesh_renderer_it));
  }
  l_mesh_renderers.free();

  l_scene.camera_destroy(l_camera);

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_c, l_engine.rasterizer_api());

  l_scene.free();
  __engine.free();
}

#include <sys/sys_impl.hpp>