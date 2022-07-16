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

#define PROGRAM_VERTEX_OUT(p_count, p_type, p_element_count)                   \
  inline static const rast::shader_vertex_output_parameter                     \
      s_vertex_out_##p_count =                                                 \
          rast::shader_vertex_output_parameter(p_type, p_element_count)

#define PROGRAM_UNIFORM_FRAGMENT(p_count, p_uniform_index)                     \
  inline static constexpr auto s_uniform_fragment_##p_count = p_uniform_index;

#define PROGRAM_META(ProgramType, p_vertex_uniform_count, p_vertex_out_count,  \
                     p_fragment_uniform_count)                                 \
  inline static ren::program_definition_meta<                                  \
      p_vertex_uniform_count, p_vertex_out_count, p_fragment_uniform_count>    \
      s_meta = s_meta.make<ProgramType>();

#define PROGRAM_VERTEX                                                         \
  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,             \
                     const ui8 *p_vertex, ui8 **p_uniforms,                    \
                     m::vec<fix32, 4> &out_screen_position, ui8 **out_vertex)

#define PROGRAM_FRAGMENT                                                       \
  static void fragment(ui8 **p_vertex_output_interpolated, ui8 **p_uniforms,   \
                       rgbf_t &out_color)

namespace ren {

template <ui8 VertexUniformCount, ui8 VertexOutCount, ui8 FragmentUniformCount>
struct program_definition_meta {
  container::arr<const ui8 *, VertexUniformCount> m_vertex_uniform_names;
  container::arr<rast::shader_uniform, VertexUniformCount> m_vertex_uniforms;
  container::arr<rast::shader_vertex_output_parameter, VertexOutCount>
      m_vertex_output;
  container::arr<rast::shader_uniform, FragmentUniformCount>
      m_fragment_uniforms;

  template <typename ProgramDefinitionType>
  inline static program_definition_meta make() {
    return {
        .m_vertex_uniform_names =
            get_vertex_uniform_names<ProgramDefinitionType,
                                     VertexUniformCount>{}(),
        .m_vertex_uniforms =
            get_vertex_uniforms<ProgramDefinitionType, VertexUniformCount>{}(),
        .m_vertex_output =
            get_vertex_out<ProgramDefinitionType, VertexOutCount>{}(),
        .m_fragment_uniforms = get_fragment_uniforms<ProgramDefinitionType,
                                                     FragmentUniformCount>{}()};
  };

private:
  template <typename ProgramDefinitionType, ui8 Index>
  struct get_uniform_name_from_index {};

  template <typename ProgramDefinitionType>
  struct get_uniform_name_from_index<ProgramDefinitionType, 0> {
    constexpr container::range<char> operator()() {
      return ProgramDefinitionType::s_param_0.range();
    };
  };

  template <typename ProgramDefinitionType>
  struct get_uniform_name_from_index<ProgramDefinitionType, 1> {
    constexpr container::range<char> operator()() {
      return ProgramDefinitionType::s_param_1.range();
    };
  };

  template <typename ProgramDefinitionType>
  struct get_uniform_name_from_index<ProgramDefinitionType, 2> {
    constexpr container::range<char> operator()() {
      return ProgramDefinitionType::s_param_2.range();
    };
  };

  template <typename ProgramDefinitionType, ui8 Index>
  struct get_uniform_type_from_index {};

  template <typename ProgramDefinitionType>
  struct get_uniform_type_from_index<ProgramDefinitionType, 0> {
    constexpr bgfx::UniformType::Enum operator()() {
      return ProgramDefinitionType::s_param_type_0;
    };
  };
  template <typename ProgramDefinitionType>
  struct get_uniform_type_from_index<ProgramDefinitionType, 1> {
    constexpr bgfx::UniformType::Enum operator()() {
      return ProgramDefinitionType::s_param_type_1;
    };
  };
  template <typename ProgramDefinitionType>
  struct get_uniform_type_from_index<ProgramDefinitionType, 2> {
    constexpr bgfx::UniformType::Enum operator()() {
      return ProgramDefinitionType::s_param_type_2;
    };
  };

  template <typename ProgramDefinitionType, ui8 Index>
  struct get_vertex_out_from_index {};

  template <typename ProgramDefinitionType>
  struct get_vertex_out_from_index<ProgramDefinitionType, 0> {
    constexpr rast::shader_vertex_output_parameter operator()() {
      return ProgramDefinitionType::s_vertex_out_0;
    };
  };

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_vertex_uniform_names {};

  template <typename ProgramDefinitionType>
  struct get_vertex_uniform_names<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<const ui8 *, 0>{}; };
  };

  template <typename ProgramDefinitionType>
  struct get_vertex_uniform_names<ProgramDefinitionType, 1> {
    auto operator()() {
      return container::arr<const ui8 *, 1>{
          (const ui8 *)get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_0>{}()
              .data()};
    };
  };

  template <typename ProgramDefinitionType>
  struct get_vertex_uniform_names<ProgramDefinitionType, 3> {
    auto operator()() {
      return container::arr<const ui8 *, 3>{
          (const ui8 *)get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_0>{}()
              .data(),
          (const ui8 *)get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_1>{}()
              .data(),
          (const ui8 *)get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_2>{}()
              .data()};
    };
  };

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_vertex_uniforms {};

  template <typename ProgramDefinitionType>
  struct get_vertex_uniforms<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<rast::shader_uniform, 0>{}; };
  };

  template <typename ProgramDefinitionType>
  struct get_vertex_uniforms<ProgramDefinitionType, 1> {
    auto operator()() {
      return container::arr<rast::shader_uniform, 1>{rast::shader_uniform::make(
          get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_0>{}(),
          get_uniform_type_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_vertex_0>{}())};
    };
  };

  template <typename ProgramDefinitionType>
  struct get_vertex_uniforms<ProgramDefinitionType, 3> {
    auto operator()() {
      return container::arr<rast::shader_uniform, 3>{
          rast::shader_uniform::make(
              get_uniform_name_from_index<
                  ProgramDefinitionType,
                  ProgramDefinitionType::s_uniform_vertex_0>{}(),
              get_uniform_type_from_index<
                  ProgramDefinitionType,
                  ProgramDefinitionType::s_uniform_vertex_0>{}()),
          rast::shader_uniform::make(
              get_uniform_name_from_index<
                  ProgramDefinitionType,
                  ProgramDefinitionType::s_uniform_vertex_1>{}(),
              get_uniform_type_from_index<
                  ProgramDefinitionType,
                  ProgramDefinitionType::s_uniform_vertex_1>{}()),
          rast::shader_uniform::make(
              get_uniform_name_from_index<
                  ProgramDefinitionType,
                  ProgramDefinitionType::s_uniform_vertex_2>{}(),
              get_uniform_type_from_index<
                  ProgramDefinitionType,
                  ProgramDefinitionType::s_uniform_vertex_2>{}())};
    };
  };

  template <typename ProgramDefinitionType, ui8 Count> struct get_vertex_out {};

  template <typename ProgramDefinitionType>
  struct get_vertex_out<ProgramDefinitionType, 0> {
    auto operator()() {
      return container::arr<rast::shader_vertex_output_parameter, 0>{};
    };
  };

  template <typename ProgramDefinitionType>
  struct get_vertex_out<ProgramDefinitionType, 1> {
    auto operator()() {
      return container::arr<rast::shader_vertex_output_parameter, 1>{
          get_vertex_out_from_index<ProgramDefinitionType, 0>{}()};
    };
  };

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_fragment_uniforms {};

  template <typename ProgramDefinitionType>
  struct get_fragment_uniforms<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<rast::shader_uniform, 0>{}; };
  };

  template <typename ProgramDefinitionType>
  struct get_fragment_uniforms<ProgramDefinitionType, 1> {
    auto operator()() {
      return container::arr<rast::shader_uniform, 1>{rast::shader_uniform::make(
          get_uniform_name_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_fragment_0>{}(),
          get_uniform_type_from_index<
              ProgramDefinitionType,
              ProgramDefinitionType::s_uniform_fragment_0>{}())};
    };
  };
};

}; // namespace ren
