
#include "doctest.h"
#include <eng/engine.hpp>
#include <eng/scene.hpp>
#include <m/const.hpp>
#include <rast/impl/rast_impl.hpp>
#include <ren/impl/ren_impl.hpp>

TEST_CASE("engine.drawCube") {
  using engine_t =
      eng::details::engine<ren::details::ren_impl, rast_impl_software>;
  using scene_t = eng::scene<engine_t>;
  engine_t __engine;
  api_decltype(eng::engine_api, l_engine, __engine);
  scene_t l_scene = {__engine};

  ui32 l_width = 32;
  ui32 l_height = 32;
  l_engine.allocate(l_width, l_height);
  l_scene.allocate();

  eng::object_handle __camera = l_scene.camera_create();
  {
    eng::camera_view<scene_t> l_camera = l_scene.camera(__camera);
    l_camera.set_width_height(l_width, l_height);
    l_camera.set_render_width_height(l_width, l_height);
    l_camera.set_local_position({1.0f, 2.0f, 3.0f});
    l_camera.set_perspective(fix32(60) * m::deg_to_rad, 0.01, 100);
  }

  auto l_frame_func = [&]() { l_scene.update(); };
  l_engine.update(l_frame_func);

  l_scene.camera_destroy(__camera);

  l_scene.free();
  l_engine.free();
}