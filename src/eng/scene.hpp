#pragma once

#include <cor/traits.hpp>
#include <eng/engine.hpp>

#define scene_get_object(scene_var, name, p_handle)                            \
  eng::name##_view<traits::remove_ptr_ref_t<decltype(scene_var)>> name =       \
      (scene_var).##name##(p_handle)

namespace eng {

struct transform_handle {
  uimax m_idx;
};

struct transform {

  ui8 m_changed;

  position_t m_local_position;
  m::vec<fix32, 3> m_local_scale;

  m::mat<fix32, 4, 4> m_local_to_world;

  static transform make_default() {
    return transform{.m_changed = 1,
                     .m_local_position = position_t::getZero(),
                     .m_local_scale = position_t::getZero(),
                     .m_local_to_world = m::mat<fix32, 4, 4>::getZero()};
  };
};

struct object_handle {
  uimax m_idx;
};

struct camera {
  transform_handle m_transform;
  ren::camera_handle m_camera;
};

template <typename Scene> struct object_view {
  Scene *m_scene;
  transform_handle m_transform;

  void set_local_position(const position_t p_local_position) {
    struct transform &l_transform = transform();
    if (l_transform.m_local_position != p_local_position) {
      l_transform.m_changed = 1;
    }
    l_transform.m_local_position = p_local_position;
  };

  struct transform &transform() {
    return m_scene->m_transforms.at(m_transform.m_idx);
  };
};

template <typename Scene> struct camera_view : object_view<Scene> {
  object_handle m_handle;

  camera_view(Scene &p_scene, object_handle p_handle) : m_handle(p_handle) {
    object_view<Scene>::m_scene = &p_scene;
    object_view<Scene>::m_transform = __get_camera().m_transform;
  };

  void set_width_height(ui32 p_width, ui32 p_height) {
    api_decltype(eng::engine_api, l_engine,
                 object_view<Scene>::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    camera &l_camera = __get_camera();
    l_ren.camera_set_width_height(l_camera.m_camera, p_width, p_height);
  };

  void set_render_width_height(ui32 p_rendertexture_width,
                               ui32 p_rendertexture_height) {
    api_decltype(eng::engine_api, l_engine,
                 object_view<Scene>::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    api_decltype(rast_api, l_rast, l_engine.rasterizer());
    camera &l_camera = __get_camera();
    l_ren.camera_set_render_width_height(l_camera.m_camera,
                                         p_rendertexture_width,
                                         p_rendertexture_height, l_rast);
  };

  void set_perspective(fix32 p_fov, fix32 p_near, fix32 p_far) {
    camera &l_camera = __get_camera();
    api_decltype(eng::engine_api, l_engine,
                 object_view<Scene>::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    l_ren.camera_set_perspective(l_camera.m_camera, p_fov, p_near, p_far);
  };

private:
  camera &__get_camera() {
    return object_view<Scene>::m_scene->m_cameras.at(m_handle.m_idx);
  };
};

template <typename Engine> struct scene {

  Engine &m_engine;

  container::pool<transform> m_transforms;

  container::pool<camera> m_cameras;
  container::vector<uimax> m_allocated_cameras;

  void allocate() {
    m_transforms.allocate(0);
    m_cameras.allocate(0);
    m_allocated_cameras.allocate(0);
  };
  void free() {
    m_transforms.free();
    m_cameras.free();
    m_allocated_cameras.free();
  };

  object_handle camera_create() {
    struct camera l_scene_camera;
    api_decltype(engine_api, l_engine, m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    l_scene_camera.m_camera = l_ren.camera_create();
    l_scene_camera.m_transform = {
        m_transforms.push_back(transform::make_default())};
    uimax l_camera_index = m_cameras.push_back(l_scene_camera);
    m_allocated_cameras.push_back(l_camera_index);
    return {l_camera_index};
  };

  void camera_destroy(object_handle p_camera) {
    for (auto i = 0; i < m_allocated_cameras.count(); ++i) {
      if (m_allocated_cameras.at(i) == p_camera.m_idx) {
        m_allocated_cameras.remove_at(i);
        struct camera &l_camera = m_cameras.at(p_camera.m_idx);
        api_decltype(eng::engine_api, l_engine, m_engine);
        api_decltype(ren::ren_api, l_ren, l_engine.renderer());
        api_decltype(rast_api, l_rast, l_engine.rasterizer());
        l_ren.camera_destroy(l_camera.m_camera, l_rast);
        m_cameras.remove_at(p_camera.m_idx);
        return;
      }
    }

    sys::abort();
  };

  camera_view<scene<Engine>> camera(object_handle p_camera) {
    return camera_view<scene<Engine>>(*this, p_camera);
  };

  void update() {
    for (auto i = 0; i < m_allocated_cameras.count(); ++i) {
      struct camera &l_camera = m_cameras.at(m_allocated_cameras.at(i));
      transform &l_transform = m_transforms.at(l_camera.m_transform.m_idx);
      if (l_transform.m_changed) {
        __update_transform(l_transform);
        api_decltype(engine_api, l_engine, m_engine);
        api_decltype(ren::ren_api, l_ren, l_engine.renderer());

        // TODO
        const m::vec<fix32, 3> eye = {-5.0f, 5.0f, -5.0f};
        l_ren.camera_set_view(
            l_camera.m_camera,
            m::look_at(eye, l_transform.m_local_position, {0, 1, 0}));
      }
    }
  };

private:
  void __update_transform(transform &p_transform) {
    assert_debug(p_transform.m_changed);
    p_transform.m_local_to_world = m::translate(p_transform.m_local_position);
    p_transform.m_changed = 0;
  };
};

}; // namespace eng