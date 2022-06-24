#include "doctest.h"

#include <assets/loader/mesh_obj.hpp>
#include <eng/engine.hpp>
#include <eng/scene.hpp>
#include <rast/impl/rast_impl.hpp>

#define WRITE_OUTPUT 0

namespace RasterizerTestToolbox {

template <typename Rasterizer>
inline static container::range<rgb_t>
getFrameBuffer(bgfx::FrameBufferHandle p_frame_buffer,
               rast_api<Rasterizer> p_rast) {
  return p_rast.fetchTextureSync(p_rast.getTexture(p_frame_buffer))
      .template cast_to<rgb_t>();
};

template <typename Engine>
inline static ren::shader_handle
load_shader(eng::engine_api<Engine> p_engine,
            const container::range<rast::shader_vertex_output_parameter>
                &p_vertex_output,
            rast::shader_vertex_function p_vertex,
            rast::shader_fragment_function p_fragment) {

  api_decltype(rast_api, l_rast, p_engine.rasterizer());
  api_decltype(ren::ren_api, l_ren, p_engine.renderer());

  return l_ren.create_shader(p_vertex_output, p_vertex, p_fragment, l_rast);
};

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

  template <typename Engine>
  inline static ren::shader_handle
  load_shader(eng::engine_api<Engine> p_engine) {
    return RasterizerTestToolbox::load_shader(p_engine, s_vertex_output.range(),
                                              vertex, fragment);
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

  template <typename Engine>
  inline static ren::shader_handle
  load_shader(eng::engine_api<Engine> p_engine) {
    return RasterizerTestToolbox::load_shader(p_engine, s_vertex_output.range(),
                                              vertex, fragment);
  };
};

template <typename Rasterizer>
inline static bgfx::ProgramHandle
load_program(rast_api<Rasterizer> p_rast,
             const container::range<rast::shader_vertex_output_parameter>
                 &p_vertex_output,
             rast::shader_vertex_function p_vertex,
             rast::shader_fragment_function p_fragment,
             bgfx::ShaderHandle *out_vertex, bgfx::ShaderHandle *out_fragment) {

  uimax l_vertex_shader_size = rast::shader_vertex_bytes::byte_size(1);
  const bgfx::Memory *l_vertex_shader_memory =
      p_rast.alloc(l_vertex_shader_size);
  rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
      p_vertex_output, p_vertex);

  const bgfx::Memory *l_fragment_shader_memory =
      p_rast.alloc(rast::shader_fragment_bytes::byte_size());
  rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
      p_fragment);

  *out_vertex = p_rast.createShader(l_vertex_shader_memory);
  *out_fragment = p_rast.createShader(l_fragment_shader_memory);
  return p_rast.createProgram(*out_vertex, *out_fragment);
};

}; // namespace RasterizerTestToolbox

TEST_CASE("rastV2.single_triangle.visibility") {

  constexpr ui16 l_width = 8, l_height = 8;

  using engine_t =
      eng::details::engine<ren::details::ren_impl, rast_impl_software>;
  engine_t l_engine;
  l_engine.allocate(l_width, l_height);

  using scene_t = eng::scene<engine_t>;
  scene_t l_scene{&l_engine};
  l_scene.allocate();

  eng::object_handle l_camera = l_scene.camera_create();
  eng::camera_view<scene_t> l_camera_view = l_scene.camera(l_camera);
  l_camera_view.set_width_height(l_width, l_height);
  l_camera_view.set_render_width_height(l_width, l_height);
  l_camera_view.set_projection(m::mat<fix32, 4, 4>::getIdentity());
  l_camera_view.set_local_rotation(m::rotate_around(m::pi<fix32>(), {0, 1, 0}));

  container::arr<ui8, 138> l_mesh_raw = {R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 0.0, 0.0, 0.0
v 1.0, 0.0, 0.0
v 0.0, 1.0, 0.0
f 1 2 3
  )""""};
  auto l_mesh = assets::obj_mesh_loader{}.compile(l_mesh_raw.range());
  ren::mesh_handle l_mesh_handle =
      l_engine.renderer_api().create_mesh(l_mesh, l_engine.rasterizer_api());
  l_mesh.free();
  ren::shader_handle l_shader_handle =
      RasterizerTestToolbox::WhiteShader::load_shader(
          eng::engine_api<engine_t>{l_engine});

  eng::object_handle l_mesh_renderer = l_scene.mesh_renderer_create();
  eng::mesh_renderer_view<scene_t> l_mesh_renderer_view =
      l_scene.mesh_renderer(l_mesh_renderer);
  l_mesh_renderer_view.set_program(l_shader_handle);
  l_mesh_renderer_view.set_mesh(l_mesh_handle);

  l_engine.update([&]() { l_scene.update(); });

  auto l_window_buffer =
      l_engine.m_window_system.get_window_buffer(l_engine.m_window);

  /*
    RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.visibility.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_single_triangle_visibility(), l_rast);
  */

  l_engine.renderer_api().destroy(l_mesh_handle, l_engine.rasterizer_api());
  l_engine.renderer_api().destroy(l_shader_handle, l_engine.rasterizer_api());

  l_scene.mesh_renderer_destroy(l_mesh_renderer);
  l_scene.camera_destroy(l_camera);
  l_scene.free();
  l_engine.free();
}

#include <sys/sys_impl.hpp>