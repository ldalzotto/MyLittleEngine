
#include <assets/loader/mesh_obj.hpp>
#include <cstring>
#include <eng/engine.hpp>
#include <m/const.hpp>
#include <rast/rast.hpp>

#include <ren/impl/ren_impl.hpp>

struct mesh_visualizer {

  bgfx::ProgramHandle m_frame_program;

  ren::camera_handle m_camera;
  ren::mesh_handle m_mesh_0;
  ren::mesh_handle m_mesh_1;

  ren::mesh_handle *m_current_mesh;

  ren::shader_handle m_shader;

private:
  inline static ui16 m_width = 128;
  inline static ui16 m_height = 128;

public:
  template <typename EngineImpl>
  void allocate(eng::engine_api<EngineImpl> p_engine) {
    ren::camera l_camera;
    l_camera.m_width = m_width;
    l_camera.m_height = m_height;
    l_camera.m_rendertexture_width = m_width;
    l_camera.m_rendertexture_height = m_height;

    const m::vec<fix32, 3> at = {0.0f, 0.0f, 0.0f};
    const m::vec<fix32, 3> eye = {-5.0f, 5.0f, -5.0f};

    l_camera.m_view = m::look_at(eye, at, {0, 1, 0});
    l_camera.m_projection =
        m::perspective(fix32(60.0f) * m::deg_to_rad, fix32(m_width) / m_height,
                       fix32(0.1f), fix32(100.0f));

    m_camera = p_engine.renderer().create_camera(l_camera);

    {
      auto l_obj_str = R""""(
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
  )"""";

      assets::mesh l_mesh =
          assets::obj_mesh_loader().compile(container::range<ui8>::make(
              (ui8 *)l_obj_str, std::strlen(l_obj_str)));

      ren::ren_api<
          typename traits::remove_ref<decltype(p_engine.renderer().thiz)>::type>
          zd = p_engine.renderer();

      m_mesh_0 = p_engine.renderer().create_mesh(l_mesh);
      l_mesh.free();
    }
    {
      auto l_obj_str = R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v -0.500000 1.000000 1.000000
v 1.000000 1.000000 1.000000
v -0.500000 -1.000000 1.000000
v 1.000000 -1.000000 1.000000
v -0.500000 1.000000 -1.000
v 1.0000 1.000000 -1.00000
v -0.500000 -1.000000 -1.000000
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
  )"""";

      assets::mesh l_mesh =
          assets::obj_mesh_loader().compile(container::range<ui8>::make(
              (ui8 *)l_obj_str, std::strlen(l_obj_str)));

      m_mesh_1 = p_engine.renderer().create_mesh(l_mesh);
      l_mesh.free();
    }

    m_current_mesh = &m_mesh_0;

    m_shader = p_engine.renderer().create_shader(
        ColorInterpolationShader::s_vertex_output.range(),
        &ColorInterpolationShader::vertex, &ColorInterpolationShader::fragment);
  };

  template <typename EngineImpl>
  void free(eng::engine_api<EngineImpl> p_engine) {
    p_engine.renderer().destroy(m_camera);
    p_engine.renderer().destroy(m_shader);
    p_engine.renderer().destroy(m_mesh_0);
    p_engine.renderer().destroy(m_mesh_1);

    p_engine.rasterizer().destroy(m_frame_program);
  };

  i32 m_counter = 0;
  fix32 m_delta = 0.1f;

  template <typename EngineImpl>
  void frame(eng::engine_api<EngineImpl> p_engine) {

    {
      eng::input::State *l_state;
      p_engine.input().m_heap.m_state_table.at(
          (uimax)eng::input::Key::ARROW_LEFT, &l_state);
      if (*l_state == eng::input::State::JUST_PRESSED) {
        if (m_current_mesh != &m_mesh_0) {
          m_current_mesh = &m_mesh_0;
        }
      }
    }

    {
      eng::input::State *l_state;
      p_engine.input().m_heap.m_state_table.at(
          (uimax)eng::input::Key::ARROW_RIGHT, &l_state);
      if (*l_state == eng::input::State::JUST_PRESSED) {
        if (m_current_mesh != &m_mesh_1) {
          m_current_mesh = &m_mesh_1;
        }
      }
    }

    m::mat<fix32, 4, 4> l_transform = m::mat<fix32, 4, 4>::getIdentity();
    l_transform =
        m::rotate_around(l_transform, fix32(m_counter) * m_delta, {0, 1, 0});

    container::arr<m::mat<fix32, 4, 4>, 1> l_mesh_transform = {l_transform};
    container::arr<ren::mesh_handle, 1> l_meshes = {*m_current_mesh};

    p_engine.renderer().draw(m_camera, m_shader, l_mesh_transform.range(),
                             l_meshes.range());
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

inline static mesh_visualizer s_mesh_visualizer;

// TODO -> move to another header

#if PLATFORM_WEBASSEMBLY_PREPROCESS

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#else

#define EMSCRIPTEN_KEEPALIVE

#endif

inline static eng::details::engine<
    ren::details::ren_impl_v2<rast_impl_software>, rast_impl_software>
    s_engine_impl;
inline static eng::engine_api<decltype(s_engine_impl)> s_engine(s_engine_impl);

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

rast_impl_software s_bgfx_impl = rast_impl_software();

#include <sys/sys_impl.hpp>
#include <sys/win_impl.hpp>
