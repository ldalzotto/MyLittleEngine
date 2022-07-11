
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
      l_test.create_shader<WhiteShader>(), l_test.material_default());

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
      l_test.create_shader<ColorInterpolationShader>(),
      l_test.material_default());

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
      l_test.create_shader<WhiteShader>(l_c_meta), l_test.material_default());

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
      l_test.create_shader<ColorInterpolationShader>(),
      l_test.material_default());

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
      l_test.create_shader<ColorInterpolationShader>(),
      l_test.material_default());

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
      l_test.create_shader<ColorInterpolationShader>(l_meta),
      l_test.material_default());

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
      l_test.create_shader<ColorInterpolationShader>(),
      l_test.material_default());

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.depth.comparison.outofbounds.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

struct rast_uniform_vertex_shader {
  inline static const auto s_param_0 =
      container::arr_literal<i8>("test_vertex_uniform_0\0");
  inline static const auto s_param_1 =
      container::arr_literal<i8>("test_vertex_uniform_1\0");
  inline static const auto s_param_2 =
      container::arr_literal<i8>("test_vertex_uniform_2\0");

  inline static container::arr<rast::shader_vertex_output_parameter, 1>
      s_vertex_output = {
          rast::shader_vertex_output_parameter(bgfx::AttribType::Float, 3)};

  inline static container::arr<rast::shader_uniform, 3> s_vertex_uniforms = {
      .m_data = {rast::shader_uniform{
                     .type = bgfx::UniformType::Vec4,
                     .hash = algorithm::hash(
                         s_param_0.range().shrink_to(s_param_0.count() - 1))},
                 rast::shader_uniform{
                     .type = bgfx::UniformType::Vec4,
                     .hash = algorithm::hash(
                         s_param_1.range().shrink_to(s_param_1.count() - 1))},
                 rast::shader_uniform{
                     .type = bgfx::UniformType::Vec4,
                     .hash = algorithm::hash(
                         s_param_2.range().shrink_to(s_param_2.count() - 1))}}};

  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                     const ui8 *p_vertex, ui8 **p_uniforms,
                     m::vec<fix32, 4> &out_screen_position, ui8 **out_vertex) {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    position_t *l_delta_pos_x = (position_t *)p_uniforms[0];
    position_t *l_delta_pos_y = (position_t *)p_uniforms[1];
    position_t *l_delta_pos_z = (position_t *)p_uniforms[2];
    out_screen_position =
        p_ctx.m_local_to_unit *
        m::vec<fix32, 4>::make(
            l_vertex_pos + (*l_delta_pos_x + *l_delta_pos_y + *l_delta_pos_z),
            1);
    rgbf_t *l_vertex_color = (rgbf_t *)out_vertex[0];
    (*l_vertex_color) = rgbf_t{1.0f, 1.0f, 1.0f};
  };

  static void fragment(ui8 **p_vertex_output_interpolated, rgbf_t &out_color) {
    rgbf_t *l_vertex_color = (position_t *)p_vertex_output_interpolated[0];
    out_color = *l_vertex_color;
  };
};

TEST_CASE("rast.uniform.vertex") {

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

  auto l_material = l_test.__engine.m_renderer.material_create();
  l_test.__engine.m_renderer.material_push(
      l_material, rast_uniform_vertex_shader::s_param_0.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_push(
      l_material, rast_uniform_vertex_shader::s_param_1.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_push(
      l_material, rast_uniform_vertex_shader::s_param_2.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material, 0, m::vec<fix32, 4>{-1, 0, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material, 1, m::vec<fix32, 4>{0, 0, -1, 0},
      rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material, 2, m::vec<fix32, 4>{0, -1, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));

  l_test.m_material_handles.push_back(l_material);

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_vertex_shader>(), l_material);

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("rast.uniform.vertex.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

TEST_CASE("rast.uniform.vertex.reuse") {

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

  auto l_material_offset = l_test.__engine.m_renderer.material_create();
  l_test.__engine.m_renderer.material_push(
      l_material_offset, rast_uniform_vertex_shader::s_param_0.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_push(
      l_material_offset, rast_uniform_vertex_shader::s_param_1.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_push(
      l_material_offset, rast_uniform_vertex_shader::s_param_2.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material_offset, 0, m::vec<fix32, 4>{-1, 0, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material_offset, 1, m::vec<fix32, 4>{0, 0, -1, 0},
      rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material_offset, 2, m::vec<fix32, 4>{0, -1, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));

  auto l_material_no_offset = l_test.__engine.m_renderer.material_create();
  l_test.__engine.m_renderer.material_push(
      l_material_no_offset, rast_uniform_vertex_shader::s_param_0.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_push(
      l_material_no_offset, rast_uniform_vertex_shader::s_param_1.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_push(
      l_material_no_offset, rast_uniform_vertex_shader::s_param_2.data(),
      bgfx::UniformType::Vec4, rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material_no_offset, 0, m::vec<fix32, 4>{0, 0, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material_no_offset, 1, m::vec<fix32, 4>{0, 0, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));
  l_test.__engine.m_renderer.material_set_vec4(
      l_material_no_offset, 2, m::vec<fix32, 4>{0, 0, 0, 0},
      rast_api(l_test.__engine.m_rasterizer));

  l_test.m_material_handles.push_back(l_material_offset);
  l_test.m_material_handles.push_back(l_material_no_offset);

  l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_vertex_shader>(), l_material_offset);

  l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_vertex_shader>(), l_material_no_offset);

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.uniform.vertex.reuse.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

#include <sys/sys_impl.hpp>