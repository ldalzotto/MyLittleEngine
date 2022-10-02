#pragma once

#include <cor/assertions.hpp>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace v2 {

template <typename T> T *array_allocate(uimax p_count) {
  return (T *)sys::malloc(p_count * sizeof(T));
};

template <typename T> void array_copy_to(T *p_from, T *p_to, uimax p_count) {
  sys::memcpy(p_to, p_from, sizeof(T) * p_count);
};

template <typename T> void array_zero(T *p_ptr, uimax p_count) {
  sys::memset(p_ptr, 0, sizeof(T) * p_count);
};

template <typename T> T *array_realloc(T *p_ptr, uimax p_new_count) {
  return (T *)sys::realloc(p_ptr, sizeof(T) * p_new_count);
};

template <typename T> struct meta_remove_ptr { using Type = T; };
template <typename T> struct meta_remove_ptr<T *> { using Type = T; };

template <i32 N, typename... T> struct tuple;
template <typename T0> struct tuple<1, T0> { T0 m_0; };
template <typename T0, typename T1> struct tuple<2, T0, T1> {
  T0 m_0;
  T1 m_1;
};

template <typename TupleType> struct tuple_get_count {};
template <typename T0> struct tuple_get_count<tuple<1, T0>> {
  constexpr i32 operator()() { return 1; }
};
template <typename T0, typename T1> struct tuple_get_count<tuple<2, T0, T1>> {
  constexpr i32 operator()() { return 2; }
};

template <typename TupleType, i32 N> struct tuple_get_element {};
template <typename T0> struct tuple_get_element<tuple<1, T0>, 0> {
  T0 *operator()(tuple<1, T0> *thiz) { return &thiz->m_0; }
};
template <typename T0, typename T1>
struct tuple_get_element<tuple<2, T0, T1>, 0> {
  T0 *operator()(tuple<2, T0, T1> *thiz) { return &thiz->m_0; }
};
template <typename T0, typename T1>
struct tuple_get_element<tuple<2, T0, T1>, 1> {
  T1 *operator()(tuple<2, T0, T1> *thiz) { return &thiz->m_1; }
};

template <typename TupleType, i32 N> struct tuple_get_element_type {};
template <typename T0> struct tuple_get_element_type<tuple<1, T0>, 0> {
  using Type = T0;
};
template <typename T0, typename T1>
struct tuple_get_element_type<tuple<2, T0, T1>, 0> {
  using Type = T0;
};
template <typename T0, typename T1>
struct tuple_get_element_type<tuple<2, T0, T1>, 1> {
  using Type = T1;
};

template <i32 N, typename TupleType>
void tuple_data_allocate_loop(TupleType *thiz, uimax p_count) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);
    *l_element = array_allocate<T>(p_count);

    tuple_data_allocate_loop<N + 1>(thiz, p_count);
  }
};

template <i32 N, typename TupleType>
void tuple_data_free_loop(TupleType *thiz) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);
    sys::free(*l_element);

    tuple_data_free_loop<N + 1>(thiz);
  }
};

template <i32 N, typename TupleType>
void tuple_data_copy_to_loop(TupleType *from, TupleType *to, uimax p_count) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element_from = tuple_get_element<TupleType, N>{}(from);
    T **l_element_to = tuple_get_element<TupleType, N>{}(to);

    array_copy_to(*l_element_from, *l_element_to, p_count);

    tuple_data_copy_to_loop<N + 1>(from, to, p_count);
  }
};

template <i32 N, typename TupleType>
void tuple_data_zero_loop(TupleType *thiz, uimax p_count) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);

    array_zero(*l_element, p_count);

    tuple_data_zero_loop<N + 1>(thiz, p_count);
  }
};

template <i32 N, typename TupleType>
void tuple_data_realloc_loop(TupleType *thiz, uimax p_count) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);

    *l_element = array_realloc(*l_element, p_count);

    tuple_data_realloc_loop<N + 1>(thiz, p_count);
  }
};

template <i32 N, typename TupleType>
void tuple_data_memmovedown_loop(TupleType *thiz, uimax p_break_index,
                                 uimax p_move_delta, uimax p_chunk_count) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);

    sys::memmove_down_t(*l_element, p_break_index, p_move_delta, p_chunk_count);

    tuple_data_memmovedown_loop<N + 1>(thiz, p_break_index, p_move_delta,
                                       p_chunk_count);
  }
};

template <i32 N, typename TupleType>
void tuple_data_memmoveup_loop(TupleType *thiz, uimax p_break_index,
                               uimax p_move_delta, uimax p_chunk_count) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);

    sys::memmove_up_t(*l_element, p_break_index, p_move_delta, p_chunk_count);

    tuple_data_memmoveup_loop<N + 1>(thiz, p_break_index, p_move_delta,
                                     p_chunk_count);
  }
};

template <typename TupleType> struct slice_impl {
  uimax m_count;
  TupleType m_data;
};

template <typename T0> using slice_1 = slice_impl<tuple<1, T0 *>>;
template <typename T0, typename T1>
using slice_2 = slice_impl<tuple<2, T0 *, T1 *>>;

template <typename TupleType>
void slice_copy_to(slice_impl<TupleType> *from, slice_impl<TupleType> *to) {
  assert_debug(from->m_count <= to->m_count);
  tuple_data_copy_to_loop<0>(&from->m_data, &to->m_data, from->m_count);
};

template <typename TupleType>
void slice_slide_self(slice_impl<TupleType> *thiz, uimax p_count) {
  assert_debug(thiz->m_count >= p_count);
  thiz->m_count -= p_count;
};

template <typename TupleType>
slice_impl<TupleType> slice_slide(slice_impl<TupleType> *thiz, uimax p_count) {
  assert_debug(thiz->m_count >= p_count);
  slice_impl<TupleType> l_return = *thiz;
  slice_slide_self(&l_return, p_count);
  return l_return;
};

template <typename TupleType>
void slice_shrink_self(slice_impl<TupleType> *thiz, uimax p_shrink_count) {
  assert_debug(thiz->m_count >= p_shrink_count);
  thiz->m_count -= p_shrink_count;
};

template <typename TupleType>
slice_impl<TupleType> slice_shrink(slice_impl<TupleType> *thiz,
                                   uimax p_shrink_count) {
  slice_impl<TupleType> l_return = *thiz;
  slice_shrink_self(&l_return, p_shrink_count);
  return l_return;
};

template <typename TupleType> void slice_zero(slice_impl<TupleType> *thiz) {
  tuple_data_zero_loop<0>(&thiz->m_data, thiz->m_count);
};

template <typename TupleType> struct span_impl {
  uimax m_count;
  TupleType m_data;
};

template <typename T0> using span_1 = span_impl<tuple<1, T0 *>>;
template <typename T0, typename T1>
using span_2 = span_impl<tuple<2, T0 *, T1 *>>;

template <typename TupleType>
void span_allocate(span_impl<TupleType> *thiz, uimax p_count) {
  tuple_data_allocate_loop<0>(&thiz->m_data, p_count);
  thiz->m_count = p_count;
};

template <typename TupleType> void span_free(span_impl<TupleType> *thiz) {
  tuple_data_free_loop<0>(&thiz->m_data);
};

template <typename TupleType>
slice_impl<TupleType> span_to_slice(span_impl<TupleType> *thiz) {
  return slice_impl<TupleType>{.m_count = thiz->m_count,
                               .m_data = thiz->m_data};
};

template <typename TupleType> struct vector_impl {
  uimax m_count;
  uimax m_capacity;
  TupleType m_data;
};

template <typename TupleType>
void vector_allocate(vector_impl<TupleType> *thiz, uimax p_count) {
  tuple_data_allocate_loop<0>(&thiz->m_data, p_count);
  thiz->m_capacity = p_count;
  thiz->m_count = 0;
};

template <typename TupleType> void vector_free(vector_impl<TupleType> *thiz) {
  tuple_data_free_loop<0>(&thiz->m_data);
};

template <typename TupleType>
slice_impl<TupleType> vector_to_slice(vector_impl<TupleType> *thiz) {
  return slice_impl<TupleType>{.m_count = thiz->m_count,
                               .m_data = thiz->m_data};
};

template <typename TupleType>
void vector_expand_delta(vector_impl<TupleType> *thiz, uimax p_delta_count) {
  uimax l_new_capacity = thiz->m_capacity + p_delta_count;
  uimax l_calculated_new_capacity = thiz->m_capacity;
  if (l_calculated_new_capacity == 0) {
    l_calculated_new_capacity = 1;
  };
  while (l_calculated_new_capacity < l_new_capacity) {
    l_calculated_new_capacity *= 2;
  }
  thiz->m_capacity = l_calculated_new_capacity;

  tuple_data_realloc_loop<0>(&thiz->m_data, thiz->m_capacity);
};

template <typename TupleType>
void vector_insert_at(vector_impl<TupleType> *thiz, uimax p_index,
                      uimax p_insert_count) {
  assert_debug(p_index <= thiz->m_capacity);
  if (thiz->m_count + p_insert_count > thiz->m_capacity) {
    uimax l_delta = (p_index + p_insert_count) - thiz->m_capacity;
    vector_expand_delta(thiz, l_delta);
  }
  uimax l_chunk_count = thiz->m_count - p_index;
  tuple_data_memmovedown_loop<0>(&thiz->m_data, p_index, p_insert_count,
                                 l_chunk_count);
  thiz->m_count += p_insert_count;
};

template <typename TupleType>
void vector_push(vector_impl<TupleType> *thiz, uimax p_delta_count) {
  if ((thiz->m_count + p_delta_count) >= thiz->m_capacity) {
    vector_expand_delta(thiz, p_delta_count);
  }
  thiz->m_count += p_delta_count;
};

template <typename TupleType>
void vector_pop(vector_impl<TupleType> *thiz, uimax p_delta_count) {
  assert_debug(p_delta_count <= thiz->m_count);
  thiz->m_count -= p_delta_count;
};

template <typename TupleType>
void vector_remove_at(vector_impl<TupleType> *thiz, uimax p_index,
                      uimax p_remove_count) {
  assert_debug(p_index < thiz->m_count && thiz->m_count > 0);
  if (p_index < thiz->m_count - 1) {
    tuple_data_memmoveup_loop<0>(&thiz->m_data, p_index + p_remove_count,
                                 p_remove_count,
                                 thiz->m_count - (p_index + p_remove_count));
  }
  thiz->m_count -= p_remove_count;
};

template <typename T0> using vector_1 = vector_impl<tuple<1, T0 *>>;
template <typename T0, typename T1>
using vector_2 = vector_impl<tuple<2, T0 *, T1 *>>;

template <typename TupleType> struct pool_impl {
  vector_1<uimax> m_free_elements;
  vector_impl<TupleType> m_elements;
};

template <typename TupleType>
void pool_allocate(pool_impl<TupleType> *thiz, uimax p_capacity) {
  vector_allocate(&thiz->m_elements, p_capacity);
  vector_allocate(&thiz->m_free_elements, 0);
};

template <typename TupleType> void pool_free(pool_impl<TupleType> *thiz) {
  vector_free(&thiz->m_elements);
  vector_free(&thiz->m_free_elements);
};

template <typename TupleType> uimax pool_push(pool_impl<TupleType> *thiz) {
  if (thiz->m_free_elements.m_count > 0) {
    uimax l_free_index =
        thiz->m_free_elements.m_data.m_0[thiz->m_free_elements.m_count - 1];
    vector_pop(&thiz->m_free_elements, 1);
    return l_free_index;
  } else {
    vector_push(&thiz->m_elements, 1);
    return thiz->m_elements.m_count - 1;
  }
};

template <typename TupleType>
i8 pool_is_element_allocated(pool_impl<TupleType> *thiz, uimax p_index) {
  for (uimax i = 0; i < thiz->m_free_elements.m_count; ++i) {
    if (thiz->m_free_elements.m_data.m_0[i] == p_index) {
      return 0;
    }
  }
  return 1;
};

template <typename TupleType>
void pool_remove(pool_impl<TupleType> *thiz, uimax p_index) {
  assert_debug(pool_is_element_allocated(thiz, p_index));
  vector_push(&thiz->m_free_elements, 1);
  thiz->m_free_elements.m_data.m_0[thiz->m_free_elements.m_count - 1] = p_index;
};

template <typename T0> using pool_1 = pool_impl<tuple<1, T0 *>>;
template <typename T0, typename T1>
using pool_2 = pool_impl<tuple<2, T0 *, T1 *>>;

// TODO -> improve this macro
#define container_declare_alias_2(p_prefix, type_0, name_0, type_1, name_1)    \
  struct slice_##p_prefix {                                                    \
    union {                                                                    \
      slice_2<type_0, type_1> m_slice;                                         \
      struct {                                                                 \
        uimax m_count;                                                         \
        struct {                                                               \
          type_0 *name_0;                                                      \
          type_1 *name_1;                                                      \
        };                                                                     \
      };                                                                       \
    };                                                                         \
  };                                                                           \
  struct vector_##p_prefix {                                                   \
    union {                                                                    \
      vector_2<type_0, type_1> m_vector;                                       \
      struct {                                                                 \
        uimax m_count;                                                         \
        uimax m_capacity;                                                      \
        struct {                                                               \
          type_0 *name_0;                                                      \
          type_1 *name_1;                                                      \
        };                                                                     \
      };                                                                       \
    };                                                                         \
  };                                                                           \
                                                                               \
  void vector_##p_prefix##_allocate(vector_##p_prefix *thiz, uimax p_count) {  \
    vector_allocate(&thiz->m_vector, p_count);                                 \
  };                                                                           \
                                                                               \
  void vector_##p_prefix##_free(vector_##p_prefix *thiz) {                     \
    vector_free(&thiz->m_vector);                                              \
  };                                                                           \
  slice_##p_prefix vector_##p_prefix##_to_slice(vector_##p_prefix *thiz) {     \
    return slice_##p_prefix{.m_slice = vector_to_slice(&thiz->m_vector)};      \
  };

}; // namespace v2