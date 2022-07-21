
#include "cor/container.hpp"
#include "rast/model.hpp"
#include "shared/types.hpp"
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
  PROGRAM_UNIFORM(0, bgfx::UniformType::Vec4, "test_vertex_uniform_0");
  PROGRAM_UNIFORM(1, bgfx::UniformType::Vec4, "test_vertex_uniform_1");
  PROGRAM_UNIFORM(2, bgfx::UniformType::Vec4, "test_vertex_uniform_2");

  PROGRAM_UNIFORM_VERTEX(0, 0);
  PROGRAM_UNIFORM_VERTEX(1, 1);
  PROGRAM_UNIFORM_VERTEX(2, 2);

  PROGRAM_VERTEX_OUT(0, bgfx::AttribType::Float, 3);

  PROGRAM_META(rast_uniform_vertex_shader, 3, 3, 1, 0);

  PROGRAM_VERTEX {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    rast::uniform_vec4_t *l_delta_pos_x = (rast::uniform_vec4_t *)p_uniforms[0];
    rast::uniform_vec4_t *l_delta_pos_y = (rast::uniform_vec4_t *)p_uniforms[1];
    rast::uniform_vec4_t *l_delta_pos_z = (rast::uniform_vec4_t *)p_uniforms[2];
    out_screen_position =
        p_ctx.m_local_to_unit *
        m::vec<fix32, 4>::make(l_vertex_pos + position_t::make(*l_delta_pos_x +
                                                               *l_delta_pos_y +
                                                               *l_delta_pos_z),
                               1);
    rgbf_t *l_vertex_color = (rgbf_t *)out_vertex[0];
    (*l_vertex_color) = rgbf_t{1.0f, 1.0f, 1.0f};
  };

  PROGRAM_FRAGMENT {
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

  auto l_material = l_test.create_material<rast_uniform_vertex_shader>();
  l_test.material_set_vec4(l_material, 0, {-1, 0, 0, 0});
  l_test.material_set_vec4(l_material, 1, {0, 0, -1, 0});
  l_test.material_set_vec4(l_material, 2, {0, -1, 0, 0});

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

  auto l_material_offset = l_test.create_material<rast_uniform_vertex_shader>();
  l_test.material_set_vec4(l_material_offset, 0, {-1, 0, 0, 0});
  l_test.material_set_vec4(l_material_offset, 1, {0, 0, -1, 0});
  l_test.material_set_vec4(l_material_offset, 2, {0, -1, 0, 0});

  auto l_material_no_offset =
      l_test.create_material<rast_uniform_vertex_shader>();
  l_test.material_set_vec4(l_material_no_offset, 0, {0, 0, 0, 0});
  l_test.material_set_vec4(l_material_no_offset, 1, {0, 0, 0, 0});
  l_test.material_set_vec4(l_material_no_offset, 2, {0, 0, 0, 0});

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

struct rast_uniform_fragment_shader {
  PROGRAM_UNIFORM(0, bgfx::UniformType::Vec4, "test_fragment_uniform_0");
  PROGRAM_UNIFORM(1, bgfx::UniformType::Vec4, "test_fragment_uniform_1");
  PROGRAM_UNIFORM(2, bgfx::UniformType::Vec4, "test_fragment_uniform_2");

  PROGRAM_UNIFORM_FRAGMENT(0, 0);
  PROGRAM_UNIFORM_FRAGMENT(1, 1);
  PROGRAM_UNIFORM_FRAGMENT(2, 2);

  PROGRAM_META(rast_uniform_fragment_shader, 3, 0, 0, 3);

  PROGRAM_VERTEX {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    out_screen_position =
        p_ctx.m_local_to_unit * m::vec<fix32, 4>::make(l_vertex_pos, 1);
  };

  PROGRAM_FRAGMENT {
    auto *l_color_0 = (rast::uniform_vec4_t *)p_uniforms[0];
    auto *l_color_1 = (rast::uniform_vec4_t *)p_uniforms[1];
    auto *l_color_2 = (rast::uniform_vec4_t *)p_uniforms[2];
    out_color = rgbf_t::make(*l_color_0 + *l_color_1 + *l_color_2);
  };
};

TEST_CASE("rast.uniform.fragment") {

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

  auto l_material = l_test.create_material<rast_uniform_fragment_shader>();
  l_test.material_set_vec4(l_material, 0, {0.5f, 0, 0, 0});
  l_test.material_set_vec4(l_material, 1, {0, 0, 0.5f, 0});
  l_test.material_set_vec4(l_material, 2, {0, 0.5f, 0, 0});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_fragment_shader>(), l_material);

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("rast.uniform.fragment.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

TEST_CASE("rast.uniform.fragment.reuse") {

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

  auto l_material_grey = l_test.create_material<rast_uniform_fragment_shader>();
  l_test.material_set_vec4(l_material_grey, 0, {0.5f, 0, 0, 0});
  l_test.material_set_vec4(l_material_grey, 1, {0, 0, 0.5f, 0});
  l_test.material_set_vec4(l_material_grey, 2, {0, 0.5f, 0, 0});

  auto l_material_white =
      l_test.create_material<rast_uniform_fragment_shader>();
  l_test.material_set_vec4(l_material_white, 0, {1.0f, 0, 0, 0});
  l_test.material_set_vec4(l_material_white, 1, {0, 0, 1.0f, 0});
  l_test.material_set_vec4(l_material_white, 2, {0, 1.0f, 0, 0});

  l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_fragment_shader>(), l_material_grey);

  auto l_white_object = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_fragment_shader>(), l_material_white);
  l_test.l_scene.mesh_renderer(l_white_object).set_local_position({-1, -1, 0});

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.uniform.fragment.reuse.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

struct rast_uniform_vertex_fragment_shader {
  PROGRAM_UNIFORM(0, bgfx::UniformType::Vec4, "test_vertex_uniform_0");
  PROGRAM_UNIFORM(1, bgfx::UniformType::Vec4, "test_fragment_uniform_0");
  PROGRAM_UNIFORM(2, bgfx::UniformType::Vec4, "test_vertex_uniform_1");
  PROGRAM_UNIFORM(3, bgfx::UniformType::Vec4, "test_fragment_uniform_1");
  PROGRAM_UNIFORM(4, bgfx::UniformType::Vec4, "test_vertex_uniform_2");
  PROGRAM_UNIFORM(5, bgfx::UniformType::Vec4, "test_fragment_uniform_2");

  PROGRAM_UNIFORM_VERTEX(0, 0);
  PROGRAM_UNIFORM_VERTEX(1, 2);
  PROGRAM_UNIFORM_VERTEX(2, 4);

  PROGRAM_UNIFORM_FRAGMENT(0, 1);
  PROGRAM_UNIFORM_FRAGMENT(1, 3);
  PROGRAM_UNIFORM_FRAGMENT(2, 5);

  PROGRAM_META(rast_uniform_vertex_fragment_shader, 6, 3, 0, 3);

  PROGRAM_VERTEX {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    rast::uniform_vec4_t *l_delta_pos_x = (rast::uniform_vec4_t *)p_uniforms[0];
    rast::uniform_vec4_t *l_delta_pos_y = (rast::uniform_vec4_t *)p_uniforms[1];
    rast::uniform_vec4_t *l_delta_pos_z = (rast::uniform_vec4_t *)p_uniforms[2];
    out_screen_position =
        p_ctx.m_local_to_unit *
        m::vec<fix32, 4>::make(l_vertex_pos + position_t::make(*l_delta_pos_x +
                                                               *l_delta_pos_y +
                                                               *l_delta_pos_z),
                               1);
  };

  PROGRAM_FRAGMENT {
    auto *l_color_0 = (rast::uniform_vec4_t *)p_uniforms[0];
    auto *l_color_1 = (rast::uniform_vec4_t *)p_uniforms[1];
    auto *l_color_2 = (rast::uniform_vec4_t *)p_uniforms[2];
    out_color = rgbf_t::make(*l_color_0 + *l_color_1 + *l_color_2);
  };
};

TEST_CASE("rast.uniform.vertex.fragment") {

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

  auto l_material =
      l_test.create_material<rast_uniform_vertex_fragment_shader>();
  // position
  l_test.material_set_vec4(l_material, 0, {-1, 0, 0, 0});
  l_test.material_set_vec4(l_material, 2, {0, 0, -1, 0});
  l_test.material_set_vec4(l_material, 4, {0, -1, 0, 0});

  // color
  l_test.material_set_vec4(l_material, 1, {0.5f, 0, 0, 0});
  l_test.material_set_vec4(l_material, 3, {0, 0, 0.5f, 0});
  l_test.material_set_vec4(l_material, 5, {0, 0.5f, 0, 0});

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_vertex_fragment_shader>(), l_material);

  l_test.update();

  auto l_tmp_path =
      container::arr_literal<ui8>("rast.uniform.vertex.fragment.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

struct rast_uniform_sampler_shader {
  PROGRAM_UNIFORM(0, bgfx::UniformType::Sampler, "test_sampler_0");

  PROGRAM_UNIFORM_FRAGMENT(0, 0);

  PROGRAM_VERTEX_OUT(0, bgfx::AttribType::Float, 2);

  PROGRAM_META(rast_uniform_sampler_shader, 1, 0, 1, 1);

  PROGRAM_VERTEX {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    const auto &l_vertex_uv =
        l_shader.get_vertex<uv_t>(bgfx::Attrib::Enum::TexCoord0, p_vertex);
    out_screen_position =
        p_ctx.m_local_to_unit * m::vec<fix32, 4>::make(l_vertex_pos, 1);

    uv_t *l_uv = (uv_t *)out_vertex[0];
    *l_uv = l_vertex_uv;
  };

  PROGRAM_FRAGMENT {
    auto *l_texture = (rast::uniform_texture *)p_uniforms[0];
    auto *l_uv = (uv_t *)p_vertex_output_interpolated[0];
    auto l_image = rast::image(
        l_texture->m_texture_info->width, l_texture->m_texture_info->height,
        l_texture->m_texture_info->bitsPerPixel,
        container::range<ui8>::make(l_texture->m_memory->data,
                                    l_texture->m_memory->size));

    uv_t l_mapped_uv_fix32 =
        *l_uv *
        m::vec<ui16, 2>{l_image.m_width, l_image.m_height}.cast<fix32>();
    auto l_mapped_uv = l_mapped_uv_fix32.cast<ui16>();
    auto l_tex =
        l_image.get_pixel(l_mapped_uv.x(), l_mapped_uv.y()).cast<fix32>() / 255;
    out_color = rgbf_t{l_tex.x(), l_tex.y(), l_tex.z()};
  };
};

TEST_CASE("rast.uniform.sampler") {

  constexpr ui16 l_width = 8, l_height = 8;
  auto l_mesh_raw_str = container::arr_literal<ui8>(R""""(
v 0.0 0.0 0.0
v 0.0 1.0 0.0
v 1.0 0.0 0.0
vt 0.0 0.0
vt 0.0 1.0
vt 1.0 0.0
f 1 2 3
  )"""");

  BaseEngineTest l_test = BaseEngineTest(l_width, l_height);
  auto l_camera = l_test.create_orthographic_camera(2, 2);
  l_test.l_scene.camera(l_camera).set_local_position({0, 0, -5});

  auto l_texture = l_test.__engine.m_rasterizer.allocate_texture(
      8, 8, 0, 0, bgfx::TextureFormat::RGB8, 0);

  auto l_material = l_test.create_material<rast_uniform_sampler_shader>();
  l_test.material_set_sampler(l_material, 0, l_texture);

  auto l_mesh_renderer = l_test.create_mesh_renderer(
      l_test.create_mesh_obj(l_mesh_raw_str.range()),
      l_test.create_shader<rast_uniform_sampler_shader>(), l_material);

  l_test.update();

  auto l_tmp_path = container::arr_literal<ui8>("rast.uniform.sampler.png");
  l_test.assert_frame_equals(l_tmp_path.range(), s_resource_config);
}

#include <sys/sys_impl.hpp>