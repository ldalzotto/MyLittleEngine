#pragma once

#include <m/const.hpp>
#include <rast/rast.hpp>

struct tmp_renderer {

  bgfx::FrameBufferHandle m_frame_buffer;
  bgfx::ProgramHandle m_frame_program;
  bgfx::VertexLayout m_vertex_layout;
  container::span<ui8> m_triangle_vertices;
  container::arr<ui16, 36> m_triangle_indices;
  bgfx::VertexBufferHandle m_vertex_buffer;
  bgfx::IndexBufferHandle m_index_buffer;

private:
  inline static ui16 m_width = 128;
  inline static ui16 m_height = 128;

public:
  void allocate() {
    m_frame_buffer =
        bgfx::createFrameBuffer(0, m_width, m_height, bgfx::TextureFormat::RGB8,
                                bgfx::TextureFormat::D32F);

    m_vertex_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
        .end();

    m_triangle_vertices.allocate(m_vertex_layout.getSize(8));

    {
      const uimax l_slide_offset = m_vertex_layout.getStride() -
                                   sizeof(m::vec<ff32, 3>) -
                                   sizeof(m::vec<ui8, 3>);
      m_triangle_vertices.range()
          .stream(m::vec<ff32, 3>{-1.0f, 1.0f, 1.0f})
          .stream(m::vec<ui8, 3>{0, 0, 0})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{1.0f, 1.0f, 1.0f})
          .stream(m::vec<ui8, 3>{0, 0, 255})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{-1.0f, -1.0f, 1.0f})
          .stream(m::vec<ui8, 3>{0, 255, 0})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{1.0f, -1.0f, 1.0f})
          .stream(m::vec<ui8, 3>{0, 255, 255})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{-1.0f, 1.0f, -1.0f})
          .stream(m::vec<ui8, 3>{255, 0, 0})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{1.0f, 1.0f, -1.0f})
          .stream(m::vec<ui8, 3>{255, 0, 255})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{-1.0f, -1.0f, -1.0f})
          .stream(m::vec<ui8, 3>{255, 255, 0})
          .slide(l_slide_offset)
          .stream(m::vec<ff32, 3>{1.0f, -1.0f, -1.0f})
          .stream(m::vec<ui8, 3>{255, 255, 255});
    }

    m_triangle_indices = {0, 1, 2,          // 0
                          1, 3, 2, 4, 6, 5, // 2
                          5, 6, 7, 0, 2, 4, // 4
                          4, 2, 6, 1, 5, 3, // 6
                          5, 7, 3, 0, 4, 1, // 8
                          4, 5, 1, 2, 3, 6, // 10
                          6, 3, 7};

    loadVertexIndex(m_vertex_layout, m_triangle_vertices.range().cast_to<ui8>(),
                    m_triangle_indices.range().cast_to<ui8>(), &m_vertex_buffer,
                    &m_index_buffer);

    m_frame_program = ColorInterpolationShader::load_program();
  };

  void free() {
    bgfx::destroy(m_index_buffer);
    bgfx::destroy(m_vertex_buffer);
    bgfx::destroy(m_frame_program);

    m_triangle_vertices.free();
  };

  // f32 m_offset = 0;

  ui32 m_counter = 0;
  ff32 m_delta = 0.1f;

  void draw() {

    m::mat<ff32, 4, 4> l_view, l_proj;
    {
      const m::vec<ff32, 3> at = {0.0f, 0.0f, 0.0f};
      const m::vec<ff32, 3> eye = {-5.0f, 5.0f, -5.0f};

      // Set view and projection matrix for view 0.
      {
        l_view = m::look_at(eye, at, {0, 1, 0});
        l_proj = m::perspective(ff32(60.0f) * m::deg_to_rad,
                                ff32(m_width) / ff32(m_height), ff32(0.1f),
                                ff32(100.0f));
      }
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
    bgfx::setViewRect(0, 0, 0, m_width, m_height);
    bgfx::setViewTransform(0, l_view.m_data, l_proj.m_data);
    bgfx::setViewFrameBuffer(0, m_frame_buffer);

    bgfx::touch(0);

    // Submit 11x11 cubes.
    m::mat<ff32, 4, 4> l_transform = m::mat<ff32, 4, 4>::getIdentity();
    l_transform = m::rotate_around(l_transform, m_delta * m_counter, {0, 1, 0});
    // l_transform.at(3, 0) = m_delta * m_counter;
    // Set model matrix for rendering.
    bgfx::setTransform(l_transform.m_data);

    bgfx::setIndexBuffer(m_index_buffer);
    bgfx::setVertexBuffer(0, m_vertex_buffer);
    bgfx::setState(BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_WRITE_Z |
                   BGFX_STATE_CULL_CW);

    // Submit primitive for rendering to view 0.
    bgfx::submit(0, m_frame_program);

    // m_offset += 0.5;
    m_counter += 1;
    bgfx::frame();
  };

  rast::image_view frame_view() {
    // TODO -> use bgfx api ?
    auto *l_texture =
        s_bgfx_impl.proxy().FrameBuffer(m_frame_buffer).RGBTexture().value();
    return rast::image_view(l_texture->info.width, l_texture->info.height,
                            l_texture->info.bitsPerPixel, l_texture->range());
  };

private:
  struct ColorInterpolationShader {
    inline static container::arr<rast::shader_vertex_output_parameter, 1>
        s_vertex_output = {
            rast::shader_vertex_output_parameter(bgfx::AttribType::Float, 3)};

    static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                       const ui8 *p_vertex,
                       m::vec<ff32, 4> &out_screen_position, ui8 **out_vertex) {
      rast::shader_vertex l_shader = {p_ctx};
      const auto &l_vertex_pos = l_shader.get_vertex<m::vec<ff32, 3>>(
          bgfx::Attrib::Enum::Position, p_vertex);
      const m::vec<ui8, 3> &l_color = l_shader.get_vertex<m::vec<ui8, 3>>(
          bgfx::Attrib::Enum::Color0, p_vertex);
      out_screen_position =
          p_ctx.m_local_to_unit * m::vec<ff32, 4>::make(l_vertex_pos, 1);

      m::vec<ff32, 3> *l_vertex_color = (m::vec<ff32, 3> *)out_vertex[0];
      (*l_vertex_color) = l_color.cast<ff32>() / 255;
    };

    static void fragment(ui8 **p_vertex_output_interpolated,
                         m::vec<ff32, 3> &out_color) {
      m::vec<ff32, 3> *l_vertex_color =
          (m::vec<ff32, 3> *)p_vertex_output_interpolated[0];
      out_color = *l_vertex_color;
    };

    inline static bgfx::ProgramHandle load_program() {
      return tmp_renderer::load_program(s_vertex_output.range(), vertex,
                                        fragment);
    };
  };

  inline static bgfx::ProgramHandle
  load_program(const container::range<rast::shader_vertex_output_parameter>
                   &p_vertex_output,
               rast::shader_vertex_function p_vertex,
               rast::shader_fragment_function p_fragment) {

    uimax l_vertex_shader_size = rast::shader_vertex_bytes::byte_size(1);
    const bgfx::Memory *l_vertex_shader_memory =
        bgfx::alloc(l_vertex_shader_size);
    rast::shader_vertex_bytes::view{l_vertex_shader_memory->data}.fill(
        p_vertex_output, p_vertex);

    const bgfx::Memory *l_fragment_shader_memory =
        bgfx::alloc(rast::shader_fragment_bytes::byte_size());
    rast::shader_fragment_bytes::view{l_fragment_shader_memory->data}.fill(
        p_fragment);

    bgfx::ShaderHandle l_vertex = bgfx::createShader(l_vertex_shader_memory);
    bgfx::ShaderHandle l_fragment =
        bgfx::createShader(l_fragment_shader_memory);
    return bgfx::createProgram(l_vertex, l_fragment);
  };

  inline static void loadVertexIndex(const bgfx::VertexLayout &p_vertex_layout,
                                     const container::range<ui8> &p_vertices,
                                     const container::range<ui8> &p_indicex,
                                     bgfx::VertexBufferHandle *out_vertex,
                                     bgfx::IndexBufferHandle *out_index) {
    const bgfx::Memory *l_vertex_memory =
        bgfx::makeRef(p_vertices.data(), p_vertices.count());
    const bgfx::Memory *l_index_memory =
        bgfx::makeRef(p_indicex.data(), p_indicex.count());

    *out_vertex = bgfx::createVertexBuffer(l_vertex_memory, p_vertex_layout);
    *out_index = bgfx::createIndexBuffer(l_index_memory);
  };
};