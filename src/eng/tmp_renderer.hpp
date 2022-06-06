#pragma once

#include <assets/loader/mesh_obj.hpp>
#include <assets/mesh.hpp>
#include <cstring>
#include <m/const.hpp>
#include <rast/rast.hpp>
#include <ren/ren.hpp>

struct tmp_renderer {

  bgfx::ProgramHandle m_frame_program;

  ren::camera_handle m_camera;
  ren::mesh_handle m_mesh;
  ren::shader_handle m_shader;

private:
  inline static ui16 m_width = 128;
  inline static ui16 m_height = 128;

public:
  void allocate(ren::ren_handle p_ren) {
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

    m_camera = p_ren.create_camera(l_camera);

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

    assets::mesh l_mesh = assets::obj_mesh_loader().compile(
        container::range<ui8>::make((ui8 *)l_obj_str, std::strlen(l_obj_str)));

    m_mesh = p_ren.create_mesh(l_mesh);
    l_mesh.free();

    m_shader = p_ren.create_shader(
        ColorInterpolationShader::s_vertex_output.range(),
        &ColorInterpolationShader::vertex, &ColorInterpolationShader::fragment);
  };

  void free(ren::ren_handle p_ren) {
    p_ren.destroy(m_camera);
    p_ren.destroy(m_shader);
    p_ren.destroy(m_mesh);

    bgfx::destroy(m_frame_program);
  };

  ui32 m_counter = 0;
  fix32 m_delta = 0.1f;

  void frame(ren::ren_handle p_ren) {

    m::mat<fix32, 4, 4> l_transform = m::mat<fix32, 4, 4>::getIdentity();
    l_transform =
        m::rotate_around(l_transform, fix32(m_counter) * m_delta, {0, 1, 0});

    container::arr<m::mat<fix32, 4, 4>, 1> l_mesh_transform = {l_transform};
    container::arr<ren::mesh_handle, 1> l_meshes = {m_mesh};

    p_ren.draw(m_camera, m_shader, l_mesh_transform.range(), l_meshes.range());

    m_counter += 1;
  };

  rast::image_view frame_view(ren::ren_handle p_ren) {
    return p_ren.frame_view(m_camera);
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