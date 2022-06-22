#include "doctest.h"

#include <eng/engine.hpp>
#include <eng/scene.hpp>
#include <rast/impl/rast_impl.hpp>

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
  l_camera_view.set_perspective(60, 0, 10);

  // TODO -> have orthographic camera
  // TODO -> load mesh and shader
  // TODO -> fetch the window output ?

#if 0
  rast_impl_software __rast;
  api_decltype(rast_api, l_rast, __rast);

  l_rast.init();

  bgfx::VertexLayout l_vertex_layout;
  l_vertex_layout.begin();
  l_vertex_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

  container::arr<position_t, 3> l_triangle_vertices = {
      .m_data = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}}};
  container::arr<vindex_t, 3> l_triangle_indices = {0, 1, 2};

  bgfx::VertexBufferHandle l_vertex_buffer;
  bgfx::IndexBufferHandle l_index_buffer;
  RasterizerTestToolbox::loadVertexIndex(
      l_rast, l_vertex_layout, l_triangle_vertices.range().cast_to<ui8>(),
      l_triangle_indices.range().cast_to<ui8>(), &l_vertex_buffer,
      &l_index_buffer);

  auto l_frame_buffer_format = bgfx::TextureFormat::RGB8;

  bgfx::FrameBufferHandle l_frame_buffer =
      l_rast.createFrameBuffer(l_width, l_height, l_frame_buffer_format);
  rast::image_view l_frame_buffer_view = rast::image_view(
      l_width, l_height, textureformat_to_pixel_size(l_frame_buffer_format),
      l_rast.fetchTextureSync(l_rast.getTexture(l_frame_buffer)));

  bgfx::ShaderHandle l_vertex, l_fragment;
  bgfx::ProgramHandle l_program =
      WhiteShader::load_program(l_rast, &l_vertex, &l_fragment);

  m::mat<fix32, 4, 4> l_indentity = l_indentity.getIdentity();

  l_rast.setViewClear(0, BGFX_CLEAR_COLOR);
  l_rast.setViewRect(0, 0, 0, l_width, l_height);
  l_rast.setViewTransform(0, l_indentity.m_data, l_indentity.m_data);
  l_rast.setViewFrameBuffer(0, l_frame_buffer);

  l_rast.touch(0);

  l_rast.setTransform(l_indentity.m_data);
  l_rast.setIndexBuffer(l_index_buffer);
  l_rast.setVertexBuffer(0, l_vertex_buffer);
  l_rast.setState(0);

  l_rast.submit(0, l_program);

  l_rast.frame();

  RasterizerTestToolbox::assert_frame_equals(
      "/media/loic/SSD/SoftwareProjects/glm/"
      "rast.single_triangle.visibility.png",
      l_frame_buffer, l_frame_buffer_view,
      frame_expected::rast_single_triangle_visibility(), l_rast);

  l_rast.destroy(l_index_buffer);
  l_rast.destroy(l_vertex_buffer);
  l_rast.destroy(l_program);
  l_rast.destroy(l_vertex);
  l_rast.destroy(l_fragment);
  l_rast.destroy(l_frame_buffer);

  l_rast.shutdown();
#endif

  l_scene.camera_destroy(l_camera);
  l_scene.free();
  l_engine.free();
}
