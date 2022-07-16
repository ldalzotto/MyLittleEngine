#pragma once

#include <cor/container.hpp>
#include <rast/model.hpp>
#include <ren/model.hpp>

#define PROGRAM_UNIFORM(p_count, p_type, p_name)                               \
  inline static const auto s_param_##p_count =                                 \
      container::arr_literal<i8>(p_name "\0");                                 \
  inline static const auto s_param_type_##p_count = p_type;

#define PROGRAM_UNIFORM_VERTEX(p_count, p_uniform_index)                       \
  inline static constexpr auto s_uniform_vertex_##p_count = p_uniform_index;

#define PROGRAM_VERTEX_META(p_uniform_count)                                   \
  inline static auto s_vertex_uniform_names = get_vertex_uniform_names<>;

#define PROGRAM_VERTEX                                                         \
  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,             \
                     const ui8 *p_vertex, ui8 **p_uniforms,                    \
                     m::vec<fix32, 4> &out_screen_position, ui8 **out_vertex)

#define PROGRAM_FRAGMENT                                                       \
  static void fragment(ui8 **p_vertex_output_interpolated, rgbf_t &out_color)

namespace ren {

template <ui8 VertexUniformCount> struct program_definition_meta {
  container::arr<const ui8 *, VertexUniformCount> m_vertex_uniform_names;
  //  container::arr<rast::shader_uniform, 3> m_vertex_uniforms;

  template <typename ProgramDefinitionType>
  inline static program_definition_meta make() {
    return {.m_vertex_uniform_names =
                get_vertex_uniform_names<ProgramDefinitionType,
                                         VertexUniformCount>{}()};
  };

private:
  template <typename ProgramDefinitionType, ui8 Index>
  struct get_uniform_name_from_index {};

  template <typename ProgramDefinitionType>
  struct get_uniform_name_from_index<ProgramDefinitionType, 0> {
    constexpr const ui8 *operator()() {
      return (const ui8 *)ProgramDefinitionType::s_param_0.data();
    };
  };

  template <typename ProgramDefinitionType>
  struct get_uniform_name_from_index<ProgramDefinitionType, 1> {
    constexpr const ui8 *operator()() {
      return (const ui8 *)ProgramDefinitionType::s_param_1.data();
    };
  };

  template <typename ProgramDefinitionType>
  struct get_uniform_name_from_index<ProgramDefinitionType, 2> {
    constexpr const ui8 *operator()() {
      return (const ui8 *)ProgramDefinitionType::s_param_2.data();
    };
  };

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_vertex_uniform_names {};

  template <typename ProgramDefinitionType>
  struct get_vertex_uniform_names<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<const ui8 *, 0>{}; };
  };
  template <typename ProgramDefinitionType>
  struct get_vertex_uniform_names<ProgramDefinitionType, 3> {
    auto operator()() {
      return container::arr<const ui8 *, 3>{
          get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_0>{}(),
          get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_1>{}(),
          get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_2>{}()};
    };
  };
};

}; // namespace ren

// struct

#if 0

struct rast_uniform_vertex_shader {
  inline static const auto s_param_0 =
      container::arr_literal<i8>("test_vertex_uniform_0\0");
  inline static const auto s_param_1 =
      container::arr_literal<i8>("test_vertex_uniform_1\0");
  inline static const auto s_param_2 =
      container::arr_literal<i8>("test_vertex_uniform_2\0");

  inline static container::arr<rast::shader_vertex_output_parameter, 1>
      s_vertex_output = {
          rast::shader_vertex_output_parameter(bgfx::AttribType::Float, 3)};

  inline static container::arr<const ui8 *, 3> s_vertex_uniform_names = {
      (const ui8 *)s_param_0.data(), (const ui8 *)s_param_1.data(),
      (const ui8 *)s_param_2.data()};

  inline static container::arr<rast::shader_uniform, 3> s_vertex_uniforms = {
      .m_data = {rast::shader_uniform::make(s_param_0.range(),
                                            bgfx::UniformType::Vec4),
                 rast::shader_uniform::make(s_param_1.range(),
                                            bgfx::UniformType::Vec4),
                 rast::shader_uniform::make(s_param_2.range(),
                                            bgfx::UniformType::Vec4)}};

  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,
                     const ui8 *p_vertex, ui8 **p_uniforms,
                     m::vec<fix32, 4> &out_screen_position, ui8 **out_vertex) {
    rast::shader_vertex l_shader = {p_ctx};
    const auto &l_vertex_pos =
        l_shader.get_vertex<position_t>(bgfx::Attrib::Enum::Position, p_vertex);
    rast::uniform_vec4_t *l_delta_pos_x = (rast::uniform_vec4_t *)p_uniforms[0];
    rast::uniform_vec4_t *l_delta_pos_y = (rast::uniform_vec4_t *)p_uniforms[1];
    rast::uniform_vec4_t *l_delta_pos_z = (rast::uniform_vec4_t *)p_uniforms[2];
    out_screen_position =
        p_ctx.m_local_to_unit *
        m::vec<fix32, 4>::make(l_vertex_pos + position_t::make(*l_delta_pos_x +
                                                               *l_delta_pos_y +
                                                               *l_delta_pos_z),
                               1);
    rgbf_t *l_vertex_color = (rgbf_t *)out_vertex[0];
    (*l_vertex_color) = rgbf_t{1.0f, 1.0f, 1.0f};
  };

  static void fragment(ui8 **p_vertex_output_interpolated, rgbf_t &out_color) {
    rgbf_t *l_vertex_color = (position_t *)p_vertex_output_interpolated[0];
    out_color = *l_vertex_color;
  };
};

#endif