#pragma once

#include <assets/loader/mesh_obj.hpp>
#include <eng/scene.hpp>
#include <rast/impl/rast_impl.hpp>
#include <tst/test_common.hpp>

struct BaseEngineTest {

  using engine_t =
      eng::details::engine<ren::details::ren_impl, rast_impl_software>;
  using scene_t = eng::scene<engine_t>;
  engine_t __engine;
  scene_t l_scene;

  ui16 m_width;
  ui16 m_height;

  container::vector<ren::mesh_handle> m_mesh_handles;
  container::vector<ren::shader_handle> m_shader_handles;

  container::vector<eng::object_handle> m_cameras;
  container::vector<eng::object_handle> m_mesh_renderers;

  BaseEngineTest(ui16 p_width, ui16 p_height) {
    __engine.allocate(p_width, p_height);
    api_decltype(eng::engine_api, l_engine, __engine);
    l_scene = {&__engine};
    l_scene.allocate();
    m_width = p_width;
    m_height = p_height;

    m_mesh_handles.allocate(0);
    m_shader_handles.allocate(0);
    m_cameras.allocate(0);
    m_mesh_renderers.allocate(0);
  }

  ~BaseEngineTest() {
    api_decltype(rast_api, l_rast, __engine.m_rasterizer);
    api_decltype(ren::ren_api, l_ren, __engine.m_renderer);

    for (auto i = 0; i < m_shader_handles.count(); ++i) {
      l_ren.destroy(m_shader_handles.at(i), l_rast);
    }

    for (auto i = 0; i < m_mesh_handles.count(); ++i) {
      l_ren.destroy(m_mesh_handles.at(i), l_rast);
    }

    for (auto i = 0; i < m_cameras.count(); ++i) {
      l_scene.camera_destroy(m_cameras.at(i));
    }

    for (auto i = 0; i < m_mesh_renderers.count(); ++i) {
      l_scene.mesh_renderer_destroy(m_mesh_renderers.at(i));
    }
    m_mesh_handles.free();
    m_shader_handles.free();
    m_cameras.free();
    m_mesh_renderers.free();

    l_scene.free();
    __engine.free();
  };

  void update() {
    api_decltype(eng::engine_api, l_engine, __engine);
    l_engine.update([&]() { l_scene.update(); });
  };

  ren::mesh_handle create_mesh_obj(const container::range<ui8> &p_obj_asset) {
    api_decltype(eng::engine_api, l_engine, __engine);
    auto l_cube_mesh = assets::obj_mesh_loader{}.compile(p_obj_asset);
    ren::mesh_handle l_mesh_handle = l_engine.renderer_api().create_mesh(
        l_cube_mesh, l_engine.rasterizer_api());
    l_cube_mesh.free();
    m_mesh_handles.push_back(l_mesh_handle);
    return l_mesh_handle;
  };

  void destroy_mesh(ren::mesh_handle p_mesh) {

    for (auto i = 0; i < m_mesh_handles.count(); ++i) {
      if (m_mesh_handles.at(i).m_idx == p_mesh.m_idx) {
        m_mesh_handles.remove_at(i);
        break;
      }
    }

    api_decltype(eng::engine_api, l_engine, __engine);
    l_engine.renderer_api().destroy(p_mesh, l_engine.rasterizer_api());
  };

  template <typename ShaderType>
  ren::shader_handle create_shader(
      const ren::shader_meta &p_meta = ren::shader_meta::get_default()) {
    api_decltype(eng::engine_api, l_engine, __engine);
    ren::shader_handle l_shader = load_shader<ShaderType>(l_engine, p_meta);
    m_shader_handles.push_back(l_shader);
    return l_shader;
  };

  void destroy_shader(ren::shader_handle p_shader) {
    api_decltype(eng::engine_api, l_engine, __engine);
    l_engine.renderer_api().destroy(p_shader, l_engine.rasterizer_api());

    for (auto i = 0; i < m_shader_handles.count(); ++i) {
      if (m_shader_handles.at(i).m_idx == p_shader.m_idx) {
        m_shader_handles.remove_at(i);
        break;
      }
    }
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

  eng::object_handle create_mesh_renderer(ren::mesh_handle p_mesh,
                                          ren::shader_handle p_shader) {
    eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
    l_scene.mesh_renderer(l_mesh_renderer).set_mesh(p_mesh);
    l_scene.mesh_renderer(l_mesh_renderer).set_program(p_shader);
    m_mesh_renderers.push_back(l_mesh_renderer);
    return l_mesh_renderer;
  };

private:
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
  inline static ren::shader_handle load_shader(eng::engine_api<Engine> p_engine,
                                               const ren::shader_meta &p_meta) {
    ren::shader_meta l_meta = l_meta.get_default();
    return load_shader(p_engine, p_meta, Shader::s_vertex_output.range(),
                       Shader::vertex, Shader::fragment);
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