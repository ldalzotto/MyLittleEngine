#pragma once

#include <cor/traits.hpp>
#include <eng/engine.hpp>

namespace eng {

struct transform_handle {
  uimax m_idx;
};

struct transform_meta {
  ui8 m_updated_this_frame;
  ui8 m_changed;
};

struct transform {

  position_t m_local_position;
  rotation_t m_local_rotation;
  m::vec<fix32, 3> m_local_scale;

  m::mat<fix32, 4, 4> m_local_to_world;

  static transform make_default() {
    return transform{.m_local_position = position_t::getZero(),
                     .m_local_rotation = rotation_t::getIdentity(),
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

struct mesh_renderer {
  transform_handle m_transform;
  ren::mesh_handle m_mesh;
  ren::shader_handle m_shader;
};

template <typename Scene> struct object_view {
  Scene *m_scene;
  transform_handle m_transform;

  void set_local_position(const position_t p_local_position) {
    struct transform *l_transform;
    transform_meta *l_transform_meta;
    transform_with_meta(&l_transform, &l_transform_meta);
    if (l_transform->m_local_position != p_local_position) {
      l_transform_meta->m_changed = 1;
    }
    l_transform->m_local_position = p_local_position;
  };

  void set_local_rotation(const rotation_t &p_local_rotation) {
    struct transform *l_transform;
    transform_meta *l_transform_meta;
    transform_with_meta(&l_transform, &l_transform_meta);
    if (l_transform->m_local_rotation != p_local_rotation) {
      l_transform_meta->m_changed = 1;
    }
    l_transform->m_local_rotation = p_local_rotation;
  };

  position_t &get_local_position() { return transform().m_local_position; };

  rotation_t &get_local_rotation() { return transform().m_local_rotation; };

protected:
  struct transform &transform() {
    struct transform *l_transform;
    m_scene->m_transforms.at(m_transform.m_idx, &l_transform);
    return *l_transform;
  };

  void transform_with_meta(struct transform **p_transform,
                           transform_meta **p_meta) {
    m_scene->m_transforms.at(m_transform.m_idx, p_transform, p_meta);
  };
};

template <typename Scene> struct camera_view : object_view<Scene> {
  using base = object_view<Scene>;
  object_handle m_handle;

  camera_view(Scene &p_scene, object_handle p_handle) : m_handle(p_handle) {
    base::m_scene = &p_scene;
    base::m_transform = get_camera().m_transform;
  };

  // TODO -> temporary public
  camera &get_camera() { return base::m_scene->m_cameras.at(m_handle.m_idx); };

  void set_width_height(ui32 p_width, ui32 p_height) {
    api_decltype(eng::engine_api, l_engine, *base::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    camera &l_camera = get_camera();
    l_ren.camera_set_width_height(l_camera.m_camera, p_width, p_height);
  };

  void set_render_width_height(ui32 p_rendertexture_width,
                               ui32 p_rendertexture_height) {
    api_decltype(eng::engine_api, l_engine, *base::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    api_decltype(rast_api, l_rast, l_engine.rasterizer());
    camera &l_camera = get_camera();
    l_ren.camera_set_render_width_height(l_camera.m_camera,
                                         p_rendertexture_width,
                                         p_rendertexture_height, l_rast);
  };

  void set_perspective(fix32 p_fov, fix32 p_near, fix32 p_far) {
    camera &l_camera = get_camera();
    api_decltype(eng::engine_api, l_engine, *base::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    l_ren.camera_set_perspective(l_camera.m_camera, p_fov, p_near, p_far);
  };

  void set_projection(const m::mat<fix32, 4, 4> &p_projection) {
    camera &l_camera = get_camera();
    api_decltype(eng::engine_api, l_engine, *base::m_scene->m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    l_ren.camera_set_projection(l_camera.m_camera, p_projection);
  };
};

template <typename Scene> struct mesh_renderer_view : object_view<Scene> {
  using base = object_view<Scene>;
  object_handle m_handle;

  mesh_renderer_view(Scene &p_scene, object_handle p_handle)
      : m_handle(p_handle) {
    base::m_scene = &p_scene;
    base::m_transform = get_mesh_renderer().m_transform;
  };

  void set_program(ren::shader_handle p_program) {
    struct mesh_renderer &l_mesh_renderer = get_mesh_renderer();
    l_mesh_renderer.m_shader = p_program;
  };

  void set_mesh(ren::mesh_handle p_mesh) {
    struct mesh_renderer &l_mesh_renderer = get_mesh_renderer();
    l_mesh_renderer.m_mesh = p_mesh;
  };

private:
  mesh_renderer &get_mesh_renderer() {
    return base::m_scene->m_mesh_renderers.at(m_handle.m_idx);
  };
};

template <typename Engine> struct scene {

  Engine *m_engine;

  orm::table_pool_v2<transform, transform_meta> m_transforms;
  container::vector<transform_handle> m_allocated_transform;

  container::pool<camera> m_cameras;
  container::vector<uimax> m_allocated_cameras;

  container::pool<mesh_renderer> m_mesh_renderers;
  // TODO -> mesh rendering should be handled by the renderer instead
  container::vector<uimax> m_allocated_mesh_renderers;

  void allocate() {
    m_transforms.allocate(0);
    m_allocated_transform.allocate(0);
    m_cameras.allocate(0);
    m_allocated_cameras.allocate(0);
    m_mesh_renderers.allocate(0);
    m_allocated_mesh_renderers.allocate(0);
  };

  void free() {
    m_transforms.free();
    m_allocated_transform.free();
    m_cameras.free();
    m_allocated_cameras.free();
    m_mesh_renderers.free();
    m_allocated_mesh_renderers.free();
  };

  object_handle camera_create() {
    struct camera l_scene_camera;
    api_decltype(engine_api, l_engine, *m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());
    l_scene_camera.m_camera = l_ren.camera_create();
    l_scene_camera.m_transform = __push_transform(transform::make_default());
    uimax l_camera_index = m_cameras.push_back(l_scene_camera);
    m_allocated_cameras.push_back(l_camera_index);
    return {l_camera_index};
  };

  void camera_destroy(object_handle p_camera) {
    for (auto i = 0; i < m_allocated_cameras.count(); ++i) {
      if (m_allocated_cameras.at(i) == p_camera.m_idx) {
        m_allocated_cameras.remove_at(i);
        struct camera &l_camera = m_cameras.at(p_camera.m_idx);
        api_decltype(eng::engine_api, l_engine, *m_engine);
        api_decltype(ren::ren_api, l_ren, l_engine.renderer());
        api_decltype(rast_api, l_rast, l_engine.rasterizer());
        l_ren.camera_destroy(l_camera.m_camera, l_rast);
        m_cameras.remove_at(p_camera.m_idx);
        __remove_transform(l_camera.m_transform);
        return;
      }
    }

    sys::abort();
  };

  camera_view<scene<Engine>> camera(object_handle p_camera) {
    return camera_view<scene<Engine>>(*this, p_camera);
  };

  object_handle mesh_renderer_create() {
    struct mesh_renderer l_mesh_renderer;
    l_mesh_renderer.m_transform = __push_transform(transform::make_default());
    object_handle l_mesh_renderer_handle = {
        m_mesh_renderers.push_back(l_mesh_renderer)};
    m_allocated_mesh_renderers.push_back(l_mesh_renderer_handle.m_idx);
    return l_mesh_renderer_handle;
  };

  void mesh_renderer_destroy(object_handle p_mesh_renderer) {
    for (auto i = 0; i < m_allocated_mesh_renderers.count(); ++i) {
      if (m_allocated_mesh_renderers.at(i) == p_mesh_renderer.m_idx) {
        m_allocated_mesh_renderers.remove_at(i);
        struct mesh_renderer &l_mesh_renderer =
            m_mesh_renderers.at(p_mesh_renderer.m_idx);
        m_mesh_renderers.remove_at(p_mesh_renderer.m_idx);
        __remove_transform(l_mesh_renderer.m_transform);
        return;
      }
    }
  };

  mesh_renderer_view<scene<Engine>>
  mesh_renderer(object_handle p_mesh_renderer) {
    return mesh_renderer_view<scene<Engine>>(*this, p_mesh_renderer);
  };

  void update() {

    api_decltype(engine_api, l_engine, *m_engine);
    api_decltype(ren::ren_api, l_ren, l_engine.renderer());

    for (auto i = 0; i < m_allocated_transform.count(); ++i) {
      transform *l_transform;
      transform_meta *l_transform_meta;
      m_transforms.at(m_allocated_transform.at(i).m_idx, &l_transform,
                      &l_transform_meta);
      if (l_transform_meta->m_changed) {
        __update_transform(*l_transform, *l_transform_meta);
        l_transform_meta->m_updated_this_frame = 1;
      } else {
        l_transform_meta->m_updated_this_frame = 0;
      }
    }

    for (auto i = 0; i < m_allocated_cameras.count(); ++i) {
      struct camera &l_camera = m_cameras.at(m_allocated_cameras.at(i));

      transform *l_transform;
      transform_meta *l_transform_meta;
      m_transforms.at(l_camera.m_transform.m_idx, &l_transform,
                      &l_transform_meta);
      if (l_transform_meta->m_updated_this_frame) {
        l_ren.camera_set_view(
            l_camera.m_camera,
            m::look_at(l_transform->m_local_position,
                       l_transform->m_local_position +
                           l_transform->m_local_to_world.forward().xyz(), position_t::up));
      }
    }

    if (m_allocated_cameras.count() > 0) {
      struct camera &l_main_camera = m_cameras.at(m_allocated_cameras.at(0));
      for (auto i = 0; i < m_allocated_mesh_renderers.count(); ++i) {
        struct mesh_renderer &l_mesh_renderer =
            m_mesh_renderers.at(m_allocated_mesh_renderers.at(i));
        transform *l_transform;
        m_transforms.at(l_mesh_renderer.m_transform.m_idx, &l_transform,
                        none());
        container::arr<m::mat<fix32, 4, 4>, 1> l_rendered_transforms = {
            l_transform->m_local_to_world};
        container::arr<ren::mesh_handle, 1> l_rendered_meshes = {
            l_mesh_renderer.m_mesh};
        l_ren.draw(l_main_camera.m_camera, l_mesh_renderer.m_shader,
                   l_rendered_transforms.range(), l_rendered_meshes.range());
      }
    }
  };

private:
  void __update_transform(transform &p_transform,
                          transform_meta &p_transform_meta) {
    assert_debug(p_transform_meta.m_changed);
    p_transform.m_local_to_world = m::translate(p_transform.m_local_position) *
                                   m::rotation(p_transform.m_local_rotation);
    p_transform_meta.m_changed = 0;
  };

  transform_handle __push_transform(const transform &p_transform) {
    transform_meta l_transform_meta = {.m_updated_this_frame = 0,
                                       .m_changed = 1};
    transform_handle l_handle = {
        m_transforms.push_back(p_transform, l_transform_meta)};
    m_allocated_transform.push_back(l_handle);
    return l_handle;
  };

  void __remove_transform(transform_handle p_transform) {
    for (auto i = 0; i < m_allocated_transform.count(); ++i) {
      if (m_allocated_transform.at(i).m_idx == p_transform.m_idx) {

        m_allocated_transform.remove_at(i);
        m_transforms.remove_at(p_transform.m_idx);
        return;
      }
    }
    assert_debug(0);
  };
};

}; // namespace eng
