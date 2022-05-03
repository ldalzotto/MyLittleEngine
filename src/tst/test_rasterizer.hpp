#pragma once

#include <m/const.hpp>
#include <rast/rast.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

struct PosColorVertex {
  float m_x;
  float m_y;
  float m_z;
  uint32_t m_abgr;

  inline static bgfx::VertexLayout ms_layout = bgfx::VertexLayout();

  static void init() {
    ms_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
  };
};

struct rastzerizer_sandbox_test {

  inline static const uint16_t s_cubeTriList[] = {
      0,
      1,
      2,
  };

  inline static const uint16_t s_cubeTriStrip[] = {
      0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
  };

  inline static const uint16_t s_cubeLineList[] = {
      0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7,
  };

  inline static const uint16_t s_cubeLineStrip[] = {
      0, 2, 3, 1, 5, 7, 6, 4, 0, 2, 6, 4, 5, 7, 3, 1, 0,
  };

  inline static const uint16_t s_cubePoints[] = {0, 1, 2, 3, 4, 5, 6, 7};

  inline static const char *s_ptNames[]{
      "Triangle List", "Triangle Strip", "Lines", "Line Strip", "Points",
  };

  inline static const uint64_t s_ptState[]{
      UINT64_C(0),          BGFX_STATE_PT_TRISTRIP,
      BGFX_STATE_PT_LINES,  BGFX_STATE_PT_LINESTRIP,
      BGFX_STATE_PT_POINTS,
  };

  static constexpr ui8 s_ptState_count =
      sizeof(s_ptState) / sizeof(s_ptState[0]);

  void operator()() {
    bgfx::init();

    PosColorVertex::init();
    /*
    static PosColorVertex s_cubeVertices[] = {
        {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
        {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
        {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
        {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
    };
    */
    static PosColorVertex s_cubeVertices[] = {

        {0.0f, 0.0f, 0.0f, 0xff0000ff},
        {1.0f, 0.0f, 0.0f, 0xff00ff00},
        {0.0f, 1.0f, 0.0f, 0xff000000},
    };

    bgfx::VertexBufferHandle m_vbh;
    bgfx::IndexBufferHandle m_ibh[s_ptState_count];

    ui16 l_width = 20;
    ui16 l_height = 20;
    auto l_frame_buffer =
        bgfx::createFrameBuffer(l_width, l_height, bgfx::TextureFormat::RGB8);

    m_vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
        PosColorVertex::ms_layout);

    // Create static index buffer for triangle list rendering.
    m_ibh[0] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));

    // Create static index buffer for triangle strip rendering.
    m_ibh[1] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));

    // Create static index buffer for line list rendering.
    m_ibh[2] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeLineList, sizeof(s_cubeLineList)));

    // Create static index buffer for line strip rendering.
    m_ibh[3] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeLineStrip, sizeof(s_cubeLineStrip)));

    // Create static index buffer for point list rendering.
    m_ibh[4] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubePoints, sizeof(s_cubePoints)));

    bgfx::setViewRect(0, 0, 0, l_width, l_height);
    m::mat<f32, 4, 4> viewMatrix, projectionMatrix;
    viewMatrix = viewMatrix.getIdentity();
    projectionMatrix = projectionMatrix.getIdentity();
    bgfx::setViewTransform(0, &viewMatrix, &projectionMatrix);
    bgfx::setViewFrameBuffer(0, l_frame_buffer);
    bgfx::touch(0);

    ui8 m_r = 1, m_g = 1, m_b = 1, m_a = 0;
    uint64_t state =
        0 | (m_r ? BGFX_STATE_WRITE_R : 0) | (m_g ? BGFX_STATE_WRITE_G : 0) |
        (m_b ? BGFX_STATE_WRITE_B : 0) | (m_a ? BGFX_STATE_WRITE_A : 0) |
        BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW |
        BGFX_STATE_MSAA | BGFX_STATE_PT_TRISTRIP;

    rast::shader_vertex_function l_vertex_function =
        [](const rast::shader_vertex_runtime_ctx &p_ctx, const ui8 *p_vertex,
           m::vec<f32, 4> &out_screen_position) {
          const m::vec<f32, 3> &l_vertex_pos =
              rast::shader_utils::get_vertex_vec3f32(p_ctx, 0, p_vertex);
          out_screen_position =
              p_ctx.m_local_to_unit * m::vec<f32, 4>::make(l_vertex_pos, 1);
        };

    bgfx::ShaderHandle l_vertex =
        bgfx::createShader((const bgfx::Memory *)l_vertex_function);
    bgfx::ShaderHandle l_fragment = bgfx::createShader((const bgfx::Memory *)0);
    bgfx::ProgramHandle l_program = bgfx::createProgram(l_vertex, l_fragment);

    m::mat<f32, 4, 4> transformMatrix = m::mat<f32, 4, 4>::getIdentity();
    bgfx::setTransform(&transformMatrix);

    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, m_vbh);
    bgfx::setIndexBuffer(m_ibh[0]);
    bgfx::setState(state);

    bgfx::submit(0, l_program);
    bgfx::frame();

    auto *l_frame_texture =
        bgfx_impl.proxy().FrameBuffer(l_frame_buffer).Texture().value();

    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/test.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);

    // Cleanup.
    for (uint32_t ii = 0; ii < s_ptState_count; ++ii) {
      bgfx::destroy(m_ibh[ii]);
    }

    bgfx::destroy(m_vbh);
    bgfx::destroy(l_program);

    bgfx::shutdown();
  };
};

struct rastzerizer_cube_test {
  ui32 m_width;
  ui32 m_height;

  inline static const uint16_t s_cubeTriList[] = {
      0, 1, 2,          // 0
      1, 3, 2, 4, 6, 5, // 2
      5, 6, 7, 0, 2, 4, // 4
      4, 2, 6, 1, 5, 3, // 6
      5, 7, 3, 0, 4, 1, // 8
      4, 5, 1, 2, 3, 6, // 10
      6, 3, 7,
  };

  inline static const uint16_t s_cubeTriStrip[] = {
      0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
  };

  inline static const uint16_t s_cubeLineList[] = {
      0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7,
  };

  inline static const uint16_t s_cubeLineStrip[] = {
      0, 2, 3, 1, 5, 7, 6, 4, 0, 2, 6, 4, 5, 7, 3, 1, 0,
  };

  inline static const uint16_t s_cubePoints[] = {0, 1, 2, 3, 4, 5, 6, 7};

  inline static const char *s_ptNames[]{
      "Triangle List", "Triangle Strip", "Lines", "Line Strip", "Points",
  };

  inline static const uint64_t s_ptState[]{
      UINT64_C(0),          BGFX_STATE_PT_TRISTRIP,
      BGFX_STATE_PT_LINES,  BGFX_STATE_PT_LINESTRIP,
      BGFX_STATE_PT_POINTS,
  };

  static constexpr ui8 s_ptState_count =
      sizeof(s_ptState) / sizeof(s_ptState[0]);

  inline static PosColorVertex s_cubeVertices[] = {
      {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
      {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
      {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
      {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
  };

  void operator()() {
    m_width = 200;
    m_height = 200;

    bgfx::Init init;
    // init.type     = args.m_type;
    // init.vendorId = args.m_pciId;
    init.resolution.width = m_width;
    init.resolution.height = m_height;
    // init.resolution.reset  = m_reset;
    bgfx::init(init);

    // Enable debug text.
    // bgfx::setDebug(m_debug);

    // Set view 0 clear state.

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f,
                       0);

    // Create vertex stream declaration.
    PosColorVertex::init();

    auto l_frame_buffer =
        bgfx::createFrameBuffer(m_width, m_height, bgfx::TextureFormat::RGB8);

    bgfx::VertexBufferHandle m_vbh;
    bgfx::IndexBufferHandle m_ibh[s_ptState_count];

    // Create static vertex buffer.
    m_vbh = bgfx::createVertexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
        PosColorVertex::ms_layout);

    // Create static index buffer for triangle list rendering.
    m_ibh[0] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));

    // Create static index buffer for triangle strip rendering.
    m_ibh[1] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));

    // Create static index buffer for line list rendering.
    m_ibh[2] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeLineList, sizeof(s_cubeLineList)));

    // Create static index buffer for line strip rendering.
    m_ibh[3] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubeLineStrip, sizeof(s_cubeLineStrip)));

    // Create static index buffer for point list rendering.
    m_ibh[4] = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_cubePoints, sizeof(s_cubePoints)));

    rast::shader_vertex_function l_vertex_function =
        [](const rast::shader_vertex_runtime_ctx &p_ctx, const ui8 *p_vertex,
           m::vec<f32, 4> &out_screen_position) {
          const m::vec<f32, 3> &l_vertex_pos =
              rast::shader_utils::get_vertex_vec3f32(p_ctx, 0, p_vertex);
          out_screen_position =
              p_ctx.m_local_to_unit * m::vec<f32, 4>::make(l_vertex_pos, 1);
        };

    bgfx::ShaderHandle l_vertex =
        bgfx::createShader((const bgfx::Memory *)l_vertex_function);
    bgfx::ShaderHandle l_fragment = bgfx::createShader((const bgfx::Memory *)0);
    bgfx::ProgramHandle l_program = bgfx::createProgram(l_vertex, l_fragment);

    const m::vec<f32, 3> at = {0.0f, 0.0f, 0.0f};
    const m::vec<f32, 3> eye = {0.0f, 0.0f, -35.0f};

    // Set view and projection matrix for view 0.
    {
      m::mat<f32, 4, 4> view = m::look_at(eye, at, {0, 1, 0});
      m::mat<f32, 4, 4> proj =
          m::perspective(60.0f * m::deg_to_rad,
                         float(m_width) / float(m_height), 0.1f, 100.0f);
      bgfx::setViewTransform(0, view.m_data, proj.m_data);

      // Set view 0 default viewport.
      bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    }

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    bgfx::IndexBufferHandle ibh = m_ibh[0];
    uint64_t state =
        0 | (1 ? BGFX_STATE_WRITE_R : 0) | (1 ? BGFX_STATE_WRITE_G : 0) |
        (1 ? BGFX_STATE_WRITE_B : 0) | (1 ? BGFX_STATE_WRITE_A : 0) |
        BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW |
        BGFX_STATE_MSAA;

    // Submit 11x11 cubes.
    for (uint32_t yy = 0; yy < 11; ++yy) {
      for (uint32_t xx = 0; xx < 11; ++xx) {
        m::mat<f32, 4, 4> l_transform = m::mat<f32, 4, 4>::getIdentity();
        l_transform.at(3, 0) = -15.0f + float(xx) * 3.0f;
        l_transform.at(3, 1) = -15.0f + float(yy) * 3.0f;
        l_transform.at(3, 2) = 0.0f;

        // Set model matrix for rendering.
        bgfx::setTransform(l_transform.m_data);
        bgfx::setViewFrameBuffer(0, l_frame_buffer);

        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, m_vbh);
        bgfx::setIndexBuffer(ibh);

        // Set render states.
        bgfx::setState(state);

        // Submit primitive for rendering to view 0.
        bgfx::submit(0, l_program);
      }
    }

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();

    auto *l_frame_texture =
        bgfx_impl.proxy().FrameBuffer(l_frame_buffer).Texture().value();

    stbi_write_png("/media/loic/SSD/SoftwareProjects/glm/test.png",
                   l_frame_texture->info.width, l_frame_texture->info.height, 3,
                   l_frame_texture->range().m_begin,
                   l_frame_texture->info.bitsPerPixel *
                       l_frame_texture->info.width);
    // Cleanup.
    for (uint32_t ii = 0; ii < s_ptState_count; ++ii) {
      bgfx::destroy(m_ibh[ii]);
    }

    bgfx::destroy(m_vbh);
    bgfx::destroy(l_program);

    bgfx::shutdown();
  };
};

inline int test_rasterizer() {

  rastzerizer_sandbox_test{}();
  rastzerizer_cube_test{}();
  return 0;

  // TODO -> memleak in containers (maybe as an option ?) extended container
};