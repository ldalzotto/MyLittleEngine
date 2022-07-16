
#include <assets/loader/mesh_obj.hpp>
#include <eng/engine.hpp>

#include <eng/scene.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/algorithm.hpp>
#include <ren/impl/ren_impl.hpp>
#include <ren/shader_definition.hpp>

template <typename EngineImpl> struct mesh_visualizer {

  using scene_t = eng::scene<EngineImpl>;
  scene_t m_scene;

  bgfx::ProgramHandle m_frame_program;

  eng::object_handle m_camera;

  ren::mesh_handle m_mesh_0;
  ren::mesh_handle m_mesh_1;

  eng::object_handle m_mesh_renderer;

  ren::program_handle m_program;
  ren::material_handle m_material;

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
      l_camera_view.set_orthographic(5, 5, 0.1, 50);
      l_camera_view.set_local_position({5, 7.5, 5});
      l_camera_view.set_local_rotation(
          m::rotate_around(-m::pi_2<fix32>() - m::pi_4<fix32>(),
                           position_t::up) *
          m::rotate_around(m::pi_4<fix32>(), position_t::left));
    }

    {
      auto l_obj_str = container::arr_literal<ui8>(R""""(
v -1.0 1.0 1.0
v -1.0 -1.0 1.0
v -1.0 1.0 -1.0
v -1.0 -1.0 -1.0
v 1.0 1.0 1.0
v 1.0 -1.0 1.0
v 1.0 1.0 -1.0
v 1.0 -1.0 -1.0
vc 0 0 0
vc 0 255 0
vc 255 0 0
vc 255 255 0
vc 0 0 255
vc 0 255 255
vc 255 0 255
vc 255 255 255
f 5/5 3/3 1/1
f 3/3 8/8 4/4
f 7/7 6/6 8/8
f 2/2 8/8 6/6
f 1/1 4/4 2/2
f 5/5 2/2 6/6
f 5/5 7/7 3/3
f 3/3 7/7 8/8
f 7/7 5/5 6/6
f 2/2 4/4 8/8
f 1/1 3/3 4/4
f 5/5 1/1 2/2
  )"""");

      assets::mesh l_mesh =
          assets::obj_mesh_loader().compile(l_obj_str.range());

      m_mesh_0 = l_ren.mesh_create(l_mesh, l_rast);
      l_mesh.free();
    }
    {
      auto l_obj_str = container::arr_literal<ui8>(R""""(
v -0.0 1.0 1.0
v -0.0 -1.0 1.0
v -0.0 1.0 -1.0
v -0.0 -1.0 -1.0
v 1.0 1.0 1.0
v 1.0 -1.0 1.0
v 1.0 1.0 -1.0
v 1.0 -1.0 -1.0
vc 0 0 0
vc 0 255 0
vc 255 0 0
vc 255 255 0
vc 0 0 255
vc 0 255 255
vc 255 0 255
vc 255 255 255
f 5/5 3/3 1/1
f 3/3 8/8 4/4
f 7/7 6/6 8/8
f 2/2 8/8 6/6
f 1/1 4/4 2/2
f 5/5 2/2 6/6
f 5/5 7/7 3/3
f 3/3 7/7 8/8
f 7/7 5/5 6/6
f 2/2 4/4 8/8
f 1/1 3/3 4/4
f 5/5 1/1 2/2
  )"""");

      assets::mesh l_mesh =
          assets::obj_mesh_loader().compile(l_obj_str.range());

      m_mesh_1 = l_ren.mesh_create(l_mesh, l_rast);
      l_mesh.free();
    }

    m_mesh_renderer = m_scene.mesh_renderer_create();

    m_material = ren::algorithm::material_create_from_shaderdefinition<
        ColorInterpolationShader>(l_ren, l_rast);

    m_program = ren::algorithm::program_create_from_shaderdefinition<
        ColorInterpolationShader>(l_ren, l_rast,
                                  ren::program_meta::get_default());

    eng::mesh_renderer_view<scene_t> l_mesh_renderer =
        m_scene.mesh_renderer(m_mesh_renderer);
    l_mesh_renderer.set_mesh(m_mesh_0);
    l_mesh_renderer.set_program(m_program);
    l_mesh_renderer.set_material(m_material);
    l_mesh_renderer.set_local_position({0, 0, 0});
  };

  void free(eng::engine_api<EngineImpl> p_engine) {

    api_decltype(ren::ren_api, l_ren, p_engine.renderer());
    api_decltype(rast_api, l_rast, p_engine.rasterizer());

    m_scene.camera_destroy(m_camera);
    m_scene.mesh_renderer_destroy(m_mesh_renderer);
    l_ren.program_destroy(m_program, l_rast);
    l_ren.mesh_destroy(m_mesh_0, l_rast);
    l_ren.mesh_destroy(m_mesh_1, l_rast);
  };

  fix32 m_speed = m::pi<fix32>() / 4;

  void frame(eng::engine_api<EngineImpl> p_engine) {

    api_decltype(ren::ren_api, l_ren, p_engine.renderer());
    api_decltype(rast_api, l_rast, p_engine.rasterizer());
    l_ren.material_set_vec4(m_material, 0, {p_engine.time().m_elapsed, 0, 0, 0},
                            l_rast);

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
        m::rotate_around(m_speed * p_engine.time().m_elapsed, position_t::up);
    eng::mesh_renderer_view<scene_t> l_mesh_renderer =
        m_scene.mesh_renderer(m_mesh_renderer);
    l_mesh_renderer.set_local_rotation(l_rotation);

    m_scene.update();
  };

private:
  struct ColorInterpolationShader {

    PROGRAM_UNIFORM(0, bgfx::UniformType::Vec4, "u_time");

    PROGRAM_UNIFORM_VERTEX(0, 0);

    PROGRAM_VERTEX_OUT(0, bgfx::AttribType::Float, 3);

    PROGRAM_META(ColorInterpolationShader, 1, 1);

    static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                       const ui8 *p_vertex, ui8 **p_uniforms,
                       rgbaf_t &out_screen_position, ui8 **out_vertex) {
      rast::shader_vertex l_shader = {p_ctx};
      const position_t &l_vertex_pos = l_shader.get_vertex<position_t>(
          bgfx::Attrib::Enum::Position, p_vertex);

      rast::uniform_vec4_t *l_time = (rast::uniform_vec4_t *)p_uniforms[0];
      position_t l_position_local =
          l_vertex_pos +
          (position_t::up * l_vertex_pos.x() * m::sin(l_time->x()));

      const rgb_t &l_color =
          l_shader.get_vertex<rgb_t>(bgfx::Attrib::Enum::Color0, p_vertex);
      out_screen_position =
          p_ctx.m_local_to_unit * m::vec<fix32, 4>::make(l_position_local, 1);

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

#include <sys/clock.hpp>

static struct clock s_clock;

inline static eng::details::engine<ren::details::ren_impl, rast_impl_software>
    s_engine_impl;
inline static eng::engine_api<decltype(s_engine_impl)> s_engine(s_engine_impl);

inline static mesh_visualizer<decltype(s_engine_impl)> s_mesh_visualizer;

extern "C" {
EMSCRIPTEN_KEEPALIVE
void initialize() {
  s_clock.init(clock_time::make_s_ms(0, 30));
  s_engine.allocate(800, 800);
  s_mesh_visualizer.allocate(s_engine);
};
EMSCRIPTEN_KEEPALIVE
void main_loop() {
  if (s_clock.update()) {
    s_engine.update(s_clock.delta(),
                    [&]() { s_mesh_visualizer.frame(s_engine); });
  }
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
  while (true) {
    main_loop();
  };
  terminate();
};
#endif

#include <sys/clock_impl.hpp>
#include <sys/sys_impl.hpp>
#include <sys/win_impl.hpp>