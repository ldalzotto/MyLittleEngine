
#include <assets/loader/mesh_obj.hpp>
#include <eng/engine.hpp>
#include <m/const.hpp>

#include <eng/scene.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/impl/ren_impl.hpp>

template <typename EngineImpl> struct mesh_visualizer {

  using scene_t = eng::scene<EngineImpl>;
  scene_t m_scene;

  bgfx::ProgramHandle m_frame_program;

  eng::object_handle m_camera;

  ren::mesh_handle m_mesh_0;
  ren::mesh_handle m_mesh_1;

  eng::object_handle m_mesh_renderer;

  ren::shader_handle m_shader;

private:
  inline static ui16 m_width = 128;
  inline static ui16 m_height = 128;

public:
  void allocate(eng::engine_api<EngineImpl> p_engine) {

    api_decltype(ren::ren_api, l_ren, p_engine.renderer());
    api_decltype(rast_api, l_rast, p_engine.rasterizer());

    m_scene.m_engine = &p_engine.thiz;
    m_scene.allocate();

    m_camera = m_scene.camera_create();
    {
      eng::camera_view<scene_t> l_camera_view = m_scene.camera(m_camera);
      l_camera_view.set_width_height(m_width, m_height);
      l_camera_view.set_render_width_height(m_width, m_height);
      l_camera_view.set_perspective(fix32(60.0f) * m::deg_to_rad, fix32(0.1f),
                                    fix32(100.0f));

      l_camera_view.set_local_position({5, 5, 5});
      auto l_rot =
          m::quat_lookat(m::normalize(position_t{-1, -1, -1}), position_t::up);
#if 1
      l_camera_view.set_local_rotation(l_rot);
#endif
// OK case
#if 0
      l_camera_view.set_local_position({0,5, 5});
      l_camera_view.set_local_rotation(m::rotate_around(m::pi<fix32>(), position_t::up) * m::rotate_around(m::pi_4<fix32>(), position_t::left));
#endif
    }

    {
      auto l_obj_str = container::arr_literal<ui8>(R""""(
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

  #if 0
        auto l_obj_str = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v -1.000000 1.000000 1.000000
v 1.000000 1.000000 1.000000
v -1.000000 -1.000000 1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 1.000000 -1.000
v 1.0000 1.000000 -1.00000
v -1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 -1.000000
vc 0 0 0
vc 0 0 255
vc 0 255 0
vc 0 255 255
vc 255 0 0
vc 255 0 255
vc 255 255 0
vc 255 255 255
o mat
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

  #endif

      assets::mesh l_mesh =
          assets::obj_mesh_loader().compile(l_obj_str.range());

      m_mesh_0 = l_ren.create_mesh(l_mesh, l_rast);
      l_mesh.free();
    }
    {
      auto l_obj_str = container::arr_literal<ui8>(R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v -0.000000 1.000000 1.000000
v 1.000000 1.000000 1.000000
v -0.000000 -1.000000 1.000000
v 1.000000 -1.000000 1.000000
v -0.000000 1.000000 -1.000
v 1.0000 1.000000 -1.00000
v -0.000000 -1.000000 -1.000000
v 1.000000 -1.000000 -1.000000
vc 0 0 0
vc 0 0 255
vc 0 255 0
vc 0 255 255
vc 255 0 0
vc 255 0 255
vc 255 255 0
vc 255 255 255
o mat
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

      assets::mesh l_mesh =
          assets::obj_mesh_loader().compile(l_obj_str.range());

      m_mesh_1 = l_ren.create_mesh(l_mesh, l_rast);
      l_mesh.free();
    }

    m_mesh_renderer = m_scene.mesh_renderer_create();

    m_shader =
        l_ren.create_shader(ren::shader_meta::get_default(),
                            ColorInterpolationShader::s_vertex_output.range(),
                            &ColorInterpolationShader::vertex,
                            &ColorInterpolationShader::fragment, l_rast);

    eng::mesh_renderer_view<scene_t> l_mesh_renderer =
        m_scene.mesh_renderer(m_mesh_renderer);
    l_mesh_renderer.set_mesh(m_mesh_0);
    l_mesh_renderer.set_program(m_shader);
    l_mesh_renderer.set_local_position({0, 0, 0});

    {
      auto l_second_mesh_renderer = m_scene.mesh_renderer_create();
      m_scene.mesh_renderer(l_second_mesh_renderer).set_mesh(m_mesh_0);
      m_scene.mesh_renderer(l_second_mesh_renderer).set_program(m_shader);
      m_scene.mesh_renderer(l_second_mesh_renderer)
          .set_local_position({0, 0, 1});
    }
    {
      auto l_second_mesh_renderer = m_scene.mesh_renderer_create();
      m_scene.mesh_renderer(l_second_mesh_renderer).set_mesh(m_mesh_0);
      m_scene.mesh_renderer(l_second_mesh_renderer).set_program(m_shader);
      m_scene.mesh_renderer(l_second_mesh_renderer)
          .set_local_position({1.5, 1, 0});
    }
  };

  void free(eng::engine_api<EngineImpl> p_engine) {

    api_decltype(ren::ren_api, l_ren, p_engine.renderer());
    api_decltype(rast_api, l_rast, p_engine.rasterizer());

    m_scene.camera_destroy(m_camera);
    m_scene.mesh_renderer_destroy(m_mesh_renderer);
    l_ren.destroy(m_shader, l_rast);
    l_ren.destroy(m_mesh_0, l_rast);
    l_ren.destroy(m_mesh_1, l_rast);
  };

  i32 m_counter = 0;
  fix32 m_delta = 0.1f;

  void frame(eng::engine_api<EngineImpl> p_engine) {

    {
      eng::input::State *l_state;
      p_engine.input().m_heap.m_state_table.at(
          (uimax)eng::input::Key::ARROW_LEFT, &l_state);
      if (*l_state == eng::input::State::JUST_PRESSED) {
        m_scene.mesh_renderer(m_mesh_renderer).set_mesh(m_mesh_0);
        m_scene.camera(m_camera).set_local_position({5, 5, 5});
      }
    }

    {
      eng::input::State *l_state;
      p_engine.input().m_heap.m_state_table.at(
          (uimax)eng::input::Key::ARROW_RIGHT, &l_state);
      if (*l_state == eng::input::State::JUST_PRESSED) {
        m_scene.mesh_renderer(m_mesh_renderer).set_mesh(m_mesh_1);
        m_scene.camera(m_camera).set_local_position({10, 10, 10});
      }
    }

    rotation_t l_rotation =
        m::rotate_around(m_delta * m_counter, {1,0,0});
    eng::mesh_renderer_view<scene_t> l_mesh_renderer =
        m_scene.mesh_renderer(m_mesh_renderer);
    l_mesh_renderer.set_local_rotation(l_rotation);

    m_scene.update();

    m_counter += 1;
  };

private:
  struct ColorInterpolationShader {
    inline static container::arr<rast::shader_vertex_output_parameter, 1>
        s_vertex_output = {
            rast::shader_vertex_output_parameter(bgfx::AttribType::Float, 3)};

    static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                       const ui8 *p_vertex, rgbaf_t &out_screen_position,
                       ui8 **out_vertex) {
      rast::shader_vertex l_shader = {p_ctx};
      const auto &l_vertex_pos = l_shader.get_vertex<position_t>(
          bgfx::Attrib::Enum::Position, p_vertex);
      const rgb_t &l_color =
          l_shader.get_vertex<rgb_t>(bgfx::Attrib::Enum::Color0, p_vertex);
      out_screen_position =
          p_ctx.m_local_to_unit * m::vec<fix32, 4>::make(l_vertex_pos, 1);

      position_t *l_vertex_color = (position_t *)out_vertex[0];
      (*l_vertex_color) = l_color.cast<fix32>() / 255;
    };

    static void fragment(ui8 **p_vertex_output_interpolated,
                         rgbf_t &out_color) {
      rgbf_t *l_vertex_color = (rgbf_t *)p_vertex_output_interpolated[0];
      out_color = *l_vertex_color;
    };
  };
};

// TODO -> move to another header

#if PLATFORM_WEBASSEMBLY_PREPROCESS

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#else

#define EMSCRIPTEN_KEEPALIVE

#endif

inline static eng::details::engine<ren::details::ren_impl, rast_impl_software>
    s_engine_impl;
inline static eng::engine_api<decltype(s_engine_impl)> s_engine(s_engine_impl);

inline static mesh_visualizer<decltype(s_engine_impl)> s_mesh_visualizer;

extern "C" {
EMSCRIPTEN_KEEPALIVE
void initialize() {
  s_engine.allocate(800, 800);
  s_mesh_visualizer.allocate(s_engine);
};
EMSCRIPTEN_KEEPALIVE
unsigned char main_loop() {
  return s_engine.update([&]() { s_mesh_visualizer.frame(s_engine); });
};
EMSCRIPTEN_KEEPALIVE
void terminate() {
  s_mesh_visualizer.free(s_engine);
  s_engine.free();
};
}

#if !PLATFORM_WEBASSEMBLY_PREPROCESS
int main() {
  initialize();
  while (main_loop()) {
  };
  terminate();
};
#endif

#include <sys/sys_impl.hpp>
#include <sys/win_impl.hpp>
