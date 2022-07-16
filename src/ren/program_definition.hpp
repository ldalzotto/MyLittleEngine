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

#define PROGRAM_META(ProgramType, p_uniform_count, p_vertex_uniform_count,     \
                     p_vertex_out_count, p_fragment_uniform_count)             \
  inline static ren::program_definition_meta<                                  \
      p_uniform_count, p_vertex_uniform_count, p_vertex_out_count,             \
      p_fragment_uniform_count>                                                \
      s_meta = s_meta.make<ProgramType>();

#define PROGRAM_VERTEX                                                         \
  static void vertex(const rast::shader_vertex_runtime_ctx &p_ctx,             \
                     const ui8 *p_vertex, ui8 **p_uniforms,                    \
                     m::vec<fix32, 4> &out_screen_position, ui8 **out_vertex)

#define PROGRAM_FRAGMENT                                                       \
  static void fragment(ui8 **p_vertex_output_interpolated, ui8 **p_uniforms,   \
                       rgbf_t &out_color)

namespace ren {

template <ui8 UniformCount, ui8 VertexUniformCount, ui8 VertexOutCount,
          ui8 FragmentUniformCount>
struct program_definition_meta {
  container::arr<const ui8 *, UniformCount> m_uniform_names;
  container::arr<rast::shader_uniform, UniformCount> m_uniforms;
  container::arr<rast::shader_uniform, VertexUniformCount> m_vertex_uniforms;
  container::arr<rast::shader_vertex_output_parameter, VertexOutCount>
      m_vertex_output;
  container::arr<rast::shader_uniform, FragmentUniformCount>
      m_fragment_uniforms;

  template <typename ProgramDefinitionType>
  inline static program_definition_meta make() {
    return {
        .m_uniform_names =
            get_uniform_names<ProgramDefinitionType, UniformCount>{}(),
        .m_uniforms = get_uniforms<ProgramDefinitionType, UniformCount>{}(),
        .m_vertex_uniforms =
            get_vertex_uniforms<ProgramDefinitionType, VertexUniformCount>{}(),
        .m_vertex_output =
            get_vertex_out<ProgramDefinitionType, VertexOutCount>{}(),
        .m_fragment_uniforms = get_fragment_uniforms<ProgramDefinitionType,
                                                     FragmentUniformCount>{}()};
  };

private:
  template <typename ProgramDefinitionType, ui8 Index>
  struct get_vertex_out_from_index {};

  template <typename ProgramDefinitionType>
  struct get_vertex_out_from_index<ProgramDefinitionType, 0> {
    constexpr rast::shader_vertex_output_parameter operator()() {
      return ProgramDefinitionType::s_vertex_out_0;
    };
  };

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_uniform_names {
    auto operator()() {
      return container::arr<const ui8 *, Count>::make(
          get_uniform_names<ProgramDefinitionType, Count - 1>{}(),
          container::arr<const ui8 *, 1>{
              (const ui8 *)get_uniform_name_from_index<ProgramDefinitionType,
                                                       Count - 1>{}()
                  .data()});
    };
  };

  template <typename ProgramDefinitionType>
  struct get_uniform_names<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<const ui8 *, 0>{}; };
  };

  template <typename ProgramDefinitionType, ui8 Count> struct get_uniforms {
    auto operator()() {
      return container::arr<rast::shader_uniform, Count>::make(
          get_uniforms<ProgramDefinitionType, Count - 1>{}(),
          container::arr<rast::shader_uniform, 1>{rast::shader_uniform::make(
              get_uniform_name_from_index<ProgramDefinitionType, Count - 1>{}(),
              get_uniform_type_from_index<ProgramDefinitionType,
                                          Count - 1>{}())});
    };
  };

  template <typename ProgramDefinitionType>
  struct get_uniforms<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<rast::shader_uniform, 0>{}; };
  };

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_vertex_uniforms {
    auto operator()() {
      return container::arr<rast::shader_uniform, Count>::make(
          get_vertex_uniforms<ProgramDefinitionType, Count - 1>{}(),
          container::arr<rast::shader_uniform, 1>{rast::shader_uniform::make(
              get_uniform_name_from_index<
                  ProgramDefinitionType,
                  get_uniform_vertex_index<ProgramDefinitionType,
                                           Count - 1>{}()>{}(),
              get_uniform_type_from_index<
                  ProgramDefinitionType,
                  get_uniform_vertex_index<ProgramDefinitionType,
                                           Count - 1>{}()>{}())});
    };
  };

  template <typename ProgramDefinitionType>
  struct get_vertex_uniforms<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<rast::shader_uniform, 0>{}; };
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
  struct get_fragment_uniforms {
    auto operator()() {
      return container::arr<rast::shader_uniform, Count>::make(
          get_fragment_uniforms<ProgramDefinitionType, Count - 1>{}(),
          container::arr<rast::shader_uniform, 1>{rast::shader_uniform::make(
              get_uniform_name_from_index<
                  ProgramDefinitionType,
                  get_uniform_fragment_index<ProgramDefinitionType,
                                             Count - 1>{}()>{}(),
              get_uniform_type_from_index<
                  ProgramDefinitionType,
                  get_uniform_fragment_index<ProgramDefinitionType,
                                             Count - 1>{}()>{}())});
    };
  };

  template <typename ProgramDefinitionType>
  struct get_fragment_uniforms<ProgramDefinitionType, 0> {
    auto operator()() { return container::arr<rast::shader_uniform, 0>{}; };
  };

  // trivial specialisations

  template <typename ProgramDefinitionType, ui8 Index>
  struct get_uniform_name_from_index {};

#define declare_get_uniform_name_from_index(p_count)                           \
  template <typename ProgramDefinitionType>                                    \
  struct get_uniform_name_from_index<ProgramDefinitionType, p_count> {         \
    constexpr container::range<char> operator()() {                            \
      return ProgramDefinitionType::s_param_##p_count.range();                 \
    };                                                                         \
  };

  declare_get_uniform_name_from_index(0);
  declare_get_uniform_name_from_index(1);
  declare_get_uniform_name_from_index(2);
  declare_get_uniform_name_from_index(3);
  declare_get_uniform_name_from_index(4);
  declare_get_uniform_name_from_index(5);

#undef declare_get_uniform_name_from_index

  template <typename ProgramDefinitionType, ui8 Index>
  struct get_uniform_type_from_index {};

#define declare_get_uniform_type_from_index(p_count)                           \
  template <typename ProgramDefinitionType>                                    \
  struct get_uniform_type_from_index<ProgramDefinitionType, p_count> {         \
    constexpr bgfx::UniformType::Enum operator()() {                           \
      return ProgramDefinitionType::s_param_type_##p_count;                    \
    };                                                                         \
  };

  declare_get_uniform_type_from_index(0);
  declare_get_uniform_type_from_index(1);
  declare_get_uniform_type_from_index(2);
  declare_get_uniform_type_from_index(3);
  declare_get_uniform_type_from_index(4);
  declare_get_uniform_type_from_index(5);

#undef declare_get_uniform_type_from_index

  template <typename ProgramDefinitionType, ui8 Index>
  struct get_uniform_vertex_index {};

#define declare_get_uniform_vertex_index(p_count)                              \
  template <typename ProgramDefinitionType>                                    \
  struct get_uniform_vertex_index<ProgramDefinitionType, p_count> {            \
    constexpr auto operator()() {                                              \
      return ProgramDefinitionType::s_uniform_vertex_##p_count;                \
    };                                                                         \
  };

  declare_get_uniform_vertex_index(0);
  declare_get_uniform_vertex_index(1);
  declare_get_uniform_vertex_index(2);
  declare_get_uniform_vertex_index(3);
  declare_get_uniform_vertex_index(4);
  declare_get_uniform_vertex_index(5);

#undef declare_get_uniform_vertex_index

  template <typename ProgramDefinitionType, ui8 Count>
  struct get_uniform_fragment_index {};

#define declare_get_uniform_fragment_index(p_count)                            \
  template <typename ProgramDefinitionType>                                    \
  struct get_uniform_fragment_index<ProgramDefinitionType, p_count> {          \
    constexpr auto operator()() {                                              \
      return ProgramDefinitionType::s_uniform_fragment_##p_count;              \
    };                                                                         \
  };

  declare_get_uniform_fragment_index(0);
  declare_get_uniform_fragment_index(1);
  declare_get_uniform_fragment_index(2);
  declare_get_uniform_fragment_index(3);
  declare_get_uniform_fragment_index(4);
  declare_get_uniform_fragment_index(5);

#undef declare_get_uniform_fragment_index
};

}; // namespace ren
