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

template <ui8 Condition, typename TrueType, typename FalseType> struct meta_if;
template <typename TrueType, typename FalseType>
struct meta_if<0, TrueType, FalseType> {
  using Type = FalseType;
};
template <typename TrueType, typename FalseType>
struct meta_if<1, TrueType, FalseType> {
  using Type = TrueType;
};

template <ui8 Condition, typename TrueType, typename FalseType>
using meta_if_t = typename meta_if<Condition, TrueType, FalseType>::Type;

template <typename T> struct meta_remove_ptr { using Type = T; };
template <typename T> struct meta_remove_ptr<T *> { using Type = T; };

template <i32 N, typename... T> struct tuple;
template <> struct tuple<0> {};
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

template <uimax CountValue, ui8 *Values> struct split_mask {
  static constexpr uimax Count = CountValue;
  static constexpr ui8 *Val = Values;
};

template <i32 Index, typename SplitMaskType>
constexpr ui8 split_mask_get_value() {
  return SplitMaskType::Val[Index];
};

template <typename SplitMaskType> constexpr uimax split_mask_get_count() {
  return SplitMaskType::Count;
};

template <uimax N, typename SplitMaskType>
constexpr ui8 split_mask_is_one_loop() {
  if constexpr (N == split_mask_get_count<SplitMaskType>()) {
    return true;
  } else {
    return split_mask_get_value<N, SplitMaskType>() == 1 &&
           split_mask_is_one_loop<N + 1, SplitMaskType>();
  }
};

template <typename SplitMaskType> constexpr ui8 split_mask_is_one() {
  return split_mask_is_one_loop<0, SplitMaskType>();
};

template <ui8...> struct split_mask_array;
template <uimax Count> constexpr auto split_mask_one();

template <ui8 T0> struct split_mask_array<T0> {
  static constexpr ui8 s_split_mask_arr[] = {T0};
  static constexpr ui8 *s_split_mask = (ui8 *)s_split_mask_arr;
};

template <ui8 T0> constexpr auto split_mask_make() {
  return split_mask<1, split_mask_array<T0>::s_split_mask>{};
};

template <> constexpr auto split_mask_one<1>() {
  return split_mask<1, split_mask_array<1>::s_split_mask>{};
};

template <ui8 T0, ui8 T1> struct split_mask_array<T0, T1> {
  static constexpr ui8 s_split_mask_arr[] = {T0, T1};
  static constexpr ui8 *s_split_mask = (ui8 *)s_split_mask_arr;
};

template <ui8 T0, ui8 T1> constexpr auto split_mask_make() {
  return split_mask<2, split_mask_array<T0, T1>::s_split_mask>{};
};

template <> constexpr auto split_mask_one<2>() {
  return split_mask<2, split_mask_array<1, 1>::s_split_mask>{};
};

template <typename TupleType, typename T> struct tuple_promote;
template <typename T0> struct tuple_promote<tuple<0>, T0> {
  using Type = tuple<1, T0>;
};
template <typename T0, typename T1> struct tuple_promote<tuple<1, T0>, T1> {
  using Type = tuple<2, T0, T1>;
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
void tuple_data_at_loop(TupleType *thiz, uimax p_index, TupleType *out) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);
    T **l_out_element = tuple_get_element<TupleType, N>{}(out);

    *l_out_element = (*l_element) + p_index;

    tuple_data_at_loop<N + 1>(thiz, p_index, out);
  }
};

template <i32 N, typename TupleType>
void tuple_data_swap_loop(TupleType *left, TupleType *right) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    T **l_left_element = tuple_get_element<TupleType, N>{}(left);
    T **l_right_element = tuple_get_element<TupleType, N>{}(right);
    T l_tmp = **l_left_element;
    **l_left_element = **l_right_element;
    **l_right_element = l_tmp;

    tuple_data_swap_loop<N + 1>(left, right);
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

template <i32 N, typename TupleType, typename TargetTupleType>
void tuple_data_cast_loop(TupleType *thiz, TargetTupleType *p_to) {
  constexpr i32 l_tuple_count = tuple_get_count<TupleType>{}();
  if constexpr (N < l_tuple_count) {
    using TPtr = typename tuple_get_element_type<TupleType, N>::Type;
    using T = typename meta_remove_ptr<TPtr>::Type;

    using TargetPtr = typename tuple_get_element_type<TargetTupleType, N>::Type;
    using Target = typename meta_remove_ptr<TargetPtr>::Type;

    T **l_element = tuple_get_element<TupleType, N>{}(thiz);
    Target **l_target_element = tuple_get_element<TargetTupleType, N>{}(p_to);

    *l_target_element = (Target *)*l_element;

    tuple_data_cast_loop<N + 1>(thiz, p_to);
  }
};

template <ui8 CurrentIndex, typename CurrentTupleType, typename SourceTupleType,
          ui8 SourceTupleTypeCount, typename SplitMaskType>
struct tuple_split_type_loop {
  using PromotedTupleType =
      typename tuple_promote<CurrentTupleType,
                             typename tuple_get_element_type<
                                 SourceTupleType, CurrentIndex>::Type>::Type;
  using IntermediaryTupleType =
      meta_if_t<split_mask_get_value<CurrentIndex, SplitMaskType>(),
                PromotedTupleType, CurrentTupleType>;

  using Type =
      typename tuple_split_type_loop<CurrentIndex + 1, IntermediaryTupleType,
                                     SourceTupleType, SourceTupleTypeCount,
                                     SplitMaskType>::Type;
};

template <typename CurrentTupleType, typename SourceTupleType,
          ui8 SourceTupleTypeCount, typename SplitMaskType>
struct tuple_split_type_loop<SourceTupleTypeCount, CurrentTupleType,
                             SourceTupleType, SourceTupleTypeCount,
                             SplitMaskType> {
  using Type = CurrentTupleType;
};

template <typename TupleType, typename SplitMaskType> struct tuple_split_type {
  using Type = typename tuple_split_type_loop<0, tuple<0>, TupleType,
                                              tuple_get_count<TupleType>{}(),
                                              SplitMaskType>::Type;
};

template <i32 SourceTupleIndex, typename SourceTupleType, i32 TargetTupleIndex,
          typename TargetTupleType, typename SplitMaskType>
void tuple_split_copy_loop(SourceTupleType *p_source,
                           TargetTupleType *p_target) {
  if constexpr (SourceTupleIndex < tuple_get_count<SourceTupleType>{}()) {
    if constexpr (split_mask_get_value<SourceTupleIndex, SplitMaskType>()) {
      using T = typename tuple_get_element_type<SourceTupleType,
                                                SourceTupleIndex>::Type;
      T *l_source_element =
          tuple_get_element<SourceTupleType, SourceTupleIndex>{}(p_source);
      T *l_target_element =
          tuple_get_element<TargetTupleType, TargetTupleIndex>{}(p_target);
      *l_target_element = *l_source_element;
      tuple_split_copy_loop<SourceTupleIndex + 1, SourceTupleType,
                            TargetTupleIndex + 1, TargetTupleType,
                            SplitMaskType>(p_source, p_target);
    } else {
      tuple_split_copy_loop<SourceTupleIndex + 1, SourceTupleType,
                            TargetTupleIndex, TargetTupleType, SplitMaskType>(
          p_source, p_target);
    }
  }
};

template <typename TupleType, typename SplitMaskType>
typename tuple_split_type<TupleType, SplitMaskType>::Type
tuple_split(TupleType *thiz, SplitMaskType) {
  static_assert(!split_mask_is_one<SplitMaskType>(), "Not needed tuple split");
  using TargetTupleType =
      typename tuple_split_type<TupleType, SplitMaskType>::Type;
  TargetTupleType l_target_tuple;
  tuple_split_copy_loop<0, TupleType, 0, TargetTupleType, SplitMaskType>(
      thiz, &l_target_tuple);
  return l_target_tuple;
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

template <typename T, typename TT>
slice_impl<tuple<1, TT *>> slice_reinterpret(slice_impl<tuple<1, T *>> *thiz) {
  slice_impl<tuple<1, TT *>> l_return;
  tuple_data_cast_loop<0>(&thiz->m_data, &l_return.m_data);
  l_return.m_count = (thiz->m_count * sizeof(T)) / sizeof(TT);
  return l_return;
};

template <typename TupleType> void slice_zero(slice_impl<TupleType> *thiz) {
  tuple_data_zero_loop<0>(&thiz->m_data, thiz->m_count);
};

template <typename TupleType, typename SplitMaskType>
slice_impl<typename tuple_split_type<TupleType, SplitMaskType>::Type>
slice_split(slice_impl<TupleType> *thiz, SplitMaskType) {
  using TargetSliceType =
      slice_impl<typename tuple_split_type<TupleType, SplitMaskType>::Type>;
  TargetSliceType l_slice;
  l_slice.m_count = thiz->m_count;
  l_slice.m_data = tuple_split(&thiz->m_data, SplitMaskType{});
  return l_slice;
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

template <typename TupleType>
void span_expand_delta(span_impl<TupleType> *thiz, uimax p_delta_count) {
  uimax l_new_count = thiz->m_count + p_delta_count;
  tuple_data_realloc_loop<0>(&thiz->m_data, l_new_count);
  thiz->m_count = l_new_count;
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

// region_begin algorithm

template <typename TupleType, typename PredicateFunction>
void slice_sort(slice_impl<TupleType> *thiz,
                const PredicateFunction &p_predicate) {
  uimax l_count = thiz->m_count;
  TupleType *l_tuple = &thiz->m_data;
  for (uimax l_left_index = 0; l_left_index < l_count; ++l_left_index) {
    TupleType l_left_tuple;
    tuple_data_at_loop<0>(l_tuple, l_left_index, &l_left_tuple);
    for (uimax l_right_index = l_left_index + 1; l_right_index < l_count;
         ++l_right_index) {
      TupleType l_right_tuple;
      tuple_data_at_loop<0>(l_tuple, l_right_index, &l_right_tuple);
      if (p_predicate(&l_left_tuple, &l_right_tuple)) {
        tuple_data_swap_loop<0>(&l_left_tuple, &l_right_tuple);
      }
    }
  }
};

static uimax algo_alignment_offset(uimax p_begin, uimax p_alignment) {
  uimax l_chunk_alignment_offset = 0;
  while ((p_begin + l_chunk_alignment_offset) % p_alignment != 0) {
    l_chunk_alignment_offset += 1;
  }
  return l_chunk_alignment_offset;
};

// region_end algorithm

struct heap_chunk {
  uimax m_begin;
  uimax m_size;
};

static ui8 heap_chunks_find_next_block(slice_1<heap_chunk> *p_chunks,
                                       uimax p_size, uimax p_alignment,
                                       uimax *out_chunk_index,
                                       uimax *out_chunk_alignment_offset) {
  assert_debug(p_size > 0);
  assert_debug(p_alignment > 0);
  for (auto l_chunk_it = 0; l_chunk_it < p_chunks->m_count; ++l_chunk_it) {
    heap_chunk *l_chunk = &p_chunks->m_data.m_0[l_chunk_it];
    uimax l_chunk_alignment_offset =
        algo_alignment_offset(l_chunk->m_begin, p_alignment);
    if ((l_chunk->m_size > l_chunk_alignment_offset) &&
        ((l_chunk->m_size - l_chunk_alignment_offset) >= p_size)) {
      *out_chunk_index = l_chunk_it;
      *out_chunk_alignment_offset = l_chunk_alignment_offset;
      return 1;
    }
  }
  return 0;
};

static void heap_chunks_defragment(vector_1<heap_chunk> *p_chunks) {
  if (p_chunks->m_count > 0) {
    slice_1<heap_chunk> l_slice = vector_to_slice(p_chunks);
    using HeapChunkTuple = tuple<1, heap_chunk *>;
    slice_sort(&l_slice, [&](HeapChunkTuple *p_left, HeapChunkTuple *p_right) {
      return p_left->m_0->m_begin < p_right->m_0->m_begin;
    });

    for (uimax l_range_reverse = p_chunks->m_count - 1; l_range_reverse >= 1;
         l_range_reverse--) {
      heap_chunk *l_next = &p_chunks->m_data.m_0[l_range_reverse];
      heap_chunk *l_previous = &p_chunks->m_data.m_0[l_range_reverse - 1];
      if (l_previous->m_begin + l_previous->m_size == l_next->m_begin) {
        l_previous->m_size += l_next->m_size;
        vector_pop(p_chunks, 1);
        l_range_reverse -= 1;
      }
    }
  }
};

template <typename TupleType> struct heap_impl {
  vector_1<heap_chunk> m_free_chunks;
  pool_1<heap_chunk> m_allocated_chunk;
  span_impl<TupleType> m_buffers;
};

template <typename TupleType>
void heap_allocate(heap_impl<TupleType> *thiz, uimax p_initial_size) {
  vector_allocate(&thiz->m_free_chunks, 0);
  pool_allocate(&thiz->m_allocated_chunk, 0);
  span_allocate(&thiz->m_buffers, 0);

  if (p_initial_size > 0) {
    heap_push_new_free_chunk(thiz, p_initial_size);
  }
};

template <typename TupleType> void heap_free(heap_impl<TupleType> *thiz) {
  vector_free(&thiz->m_free_chunks);
  pool_free(&thiz->m_allocated_chunk);
  span_free(&thiz->m_buffers);
};

template <typename TupleType>
void heap_push_new_free_chunk(heap_impl<TupleType> *thiz, uimax p_chunk_size) {
  heap_chunk l_chunk;
  l_chunk.m_begin = thiz->m_buffers.m_count;
  l_chunk.m_size = p_chunk_size;
  vector_push(&thiz->m_free_chunks, 1);
  thiz->m_free_chunks.m_data.m_0[thiz->m_free_chunks.m_count - 1] = l_chunk;
  span_expand_delta(&thiz->m_buffers, l_chunk.m_size);
};

template <typename TupleType>
uimax heap_push_new_allocated_chunk(heap_impl<TupleType> *thiz, uimax p_size,
                                    uimax p_free_chunk_index) {

  heap_chunk *l_free_chunk =
      &thiz->m_free_chunks.m_data.m_0[p_free_chunk_index];
  heap_chunk l_chunk;
  l_chunk.m_begin = l_free_chunk->m_begin;
  l_chunk.m_size = p_size;
  uimax l_allocated_chunk_index = pool_push(&thiz->m_allocated_chunk);
  thiz->m_allocated_chunk.m_elements.m_data.m_0[l_allocated_chunk_index] =
      l_chunk;
  if (l_free_chunk->m_size > p_size) {
    l_free_chunk->m_size -= p_size;
    l_free_chunk->m_begin += p_size;
  } else {
    vector_remove_at(&thiz->m_free_chunks, p_free_chunk_index, 1);
  }
  return l_allocated_chunk_index;
};

template <typename TupleType>
void heap_split_free_chunk(heap_impl<TupleType> *thiz, uimax p_chunk_index,
                           uimax p_relative_begin) {
  heap_chunk *l_chunk_to_split = &thiz->m_free_chunks.m_data.m_0[p_chunk_index];
  heap_chunk l_chunk_end = *l_chunk_to_split;
  l_chunk_end.m_begin += p_relative_begin;
  l_chunk_end.m_size -= p_relative_begin;

  l_chunk_to_split->m_size = p_relative_begin;
  vector_push(&thiz->m_free_chunks, 1);
  thiz->m_free_chunks.m_data.m_0[thiz->m_free_chunks.m_count - 1] = l_chunk_end;
};

template <typename TupleType>
uimax heap_allocate_chunk_aligned(heap_impl<TupleType> *thiz, uimax p_size,
                                  uimax p_alignment) {
  uimax l_free_chunk_index = -1;
  uimax l_alignment_offset = -1;
  slice_1<heap_chunk> l_free_chunks_slice =
      vector_to_slice(&thiz->m_free_chunks);
  if (!heap_chunks_find_next_block(&l_free_chunks_slice, p_size, p_alignment,
                                   &l_free_chunk_index, &l_alignment_offset)) {
    heap_chunks_defragment(&thiz->m_free_chunks);
    l_free_chunks_slice = vector_to_slice(&thiz->m_free_chunks);
    if (!heap_chunks_find_next_block(&l_free_chunks_slice, p_size, p_alignment,
                                     &l_free_chunk_index,
                                     &l_alignment_offset)) {
      // TODO -> pushing a chunk by multiplying capacity by 2 like vector ?
      heap_push_new_free_chunk(thiz, p_size);

      l_free_chunks_slice = vector_to_slice(&thiz->m_free_chunks);
      heap_chunks_find_next_block(&l_free_chunks_slice, p_size, p_alignment,
                                  &l_free_chunk_index, &l_alignment_offset);
    }
  }
  assert_debug(l_free_chunk_index != -1);

  if (l_alignment_offset > 0) {
    heap_split_free_chunk(thiz, l_free_chunk_index, l_alignment_offset);
    l_free_chunk_index += 1;
  }

  uimax l_chunk_index =
      heap_push_new_allocated_chunk(thiz, p_size, l_free_chunk_index);
  return l_chunk_index;
};

template <typename TupleType>
uimax heap_allocate_chunk(heap_impl<TupleType> *thiz, uimax p_size) {
  return heap_allocate_chunk_aligned(thiz, p_size, 1);
};

template <typename TupleType>
uimax heap_allocate_chunk_aligned_no_realloc(heap_impl<TupleType> *thiz,
                                             uimax p_size, uimax p_alignment) {
  uimax l_free_chunk_index = -1;
  uimax l_alignment_offset = -1;
  slice_1<heap_chunk> l_free_chunks_slice =
      vector_to_slice(&thiz->m_free_chunks);
  if (!heap_chunks_find_next_block(&l_free_chunks_slice, p_size, p_alignment,
                                   &l_free_chunk_index, &l_alignment_offset)) {
    heap_chunks_defragment(&thiz->m_free_chunks);
    l_free_chunks_slice = vector_to_slice(&thiz->m_free_chunks);
    heap_chunks_find_next_block(&l_free_chunks_slice, p_size, p_alignment,
                                &l_free_chunk_index, &l_alignment_offset);
  }

  if (l_free_chunk_index == -1) {
    return -1;
  }

  if (l_alignment_offset > 0) {
    heap_split_free_chunk(thiz, l_free_chunk_index, l_alignment_offset);
    l_free_chunk_index += 1;
  }

  uimax l_chunk_index =
      heap_push_new_allocated_chunk(thiz, p_size, l_free_chunk_index);
  return l_chunk_index;
};

template <typename TupleType>
void heap_free_chunk(heap_impl<TupleType> *thiz, uimax p_chunk_index) {
  heap_chunk *l_chunk =
      &thiz->m_allocated_chunk.m_elements.m_data.m_0[p_chunk_index];
  vector_push(&thiz->m_free_chunks, 1);
  thiz->m_free_chunks.m_data.m_0[thiz->m_free_chunks.m_count - 1] = *l_chunk;
  pool_remove(&thiz->m_allocated_chunk, p_chunk_index);
};

template <typename TupleType>
slice_impl<TupleType> heap_chunk_to_slice(heap_impl<TupleType> *thiz,
                                          uimax p_chunk_index) {
  heap_chunk *l_chunk =
      &thiz->m_allocated_chunk.m_elements.m_data.m_0[p_chunk_index];
  TupleType *l_buffer_tuple = &thiz->m_buffers.m_data;

  slice_impl<TupleType> l_return;
  tuple_data_at_loop<0>(l_buffer_tuple, l_chunk->m_begin, &l_return.m_data);
  l_return.m_count = l_chunk->m_size;

  return l_return;
};

template <typename TupleType>
ui8 heap_check_consistency(heap_impl<TupleType> *thiz) {

  ui8 l_is_consistent = 1;
  vector_1<heap_chunk> l_chunks;
  vector_allocate(&l_chunks, 0);
  for (auto i = 0; i < thiz->m_allocated_chunk.m_elements.m_count; ++i) {
    if (pool_is_element_allocated(&thiz->m_allocated_chunk, i)) {
      vector_push(&l_chunks, 1);
      l_chunks.m_data.m_0[l_chunks.m_count - 1] =
          thiz->m_allocated_chunk.m_elements.m_data.m_0[i];
    }
  }

  for (auto i = 0; i < thiz->m_free_chunks.m_count; ++i) {
    vector_push(&l_chunks, 1);
    l_chunks.m_data.m_0[l_chunks.m_count - 1] =
        thiz->m_free_chunks.m_data.m_0[i];
  }

  // Ensure that there is no overlaps in chunks

  for (auto i = 0; i < l_chunks.m_count; ++i) {
    heap_chunk *l_left_chunk = &l_chunks.m_data.m_0[i];

    uimax l_left_begin = l_left_chunk->m_begin;
    uimax l_left_end = l_left_chunk->m_begin + l_left_chunk->m_size;

    for (auto j = i + 1; j < l_chunks.m_count; ++j) {
      heap_chunk *l_right_chunk = &l_chunks.m_data.m_0[j];

      uimax l_right_begin = l_right_chunk->m_begin;
      uimax l_right_end = l_right_chunk->m_begin + l_right_chunk->m_size;

      if ((l_right_begin - l_left_begin) > 0 &&
          (l_right_begin - l_left_end) < 0) {
        l_is_consistent = 0;
        goto heap_check_consistency_end;
      }
      if ((l_right_end - l_left_begin) > 0 && (l_right_end - l_left_end) < 0) {
        l_is_consistent = 0;
        goto heap_check_consistency_end;
      }
    }
  }

  {
    // Ensure total count
    uimax l_total_count = 0;
    for (auto i = 0; i < l_chunks.m_count; ++i) {
      heap_chunk *l_chunk = &l_chunks.m_data.m_0[i];
      l_total_count += l_chunk->m_size;
    }

    if (l_total_count != thiz->m_buffers.m_count) {
      l_is_consistent = 0;
      goto heap_check_consistency_end;
    }
  }

heap_check_consistency_end:
  vector_free(&l_chunks);
  return l_is_consistent;
};

template <typename T0> using heap_1 = heap_impl<tuple<1, T0 *>>;
template <typename T0, typename T1>
using heap_2 = heap_impl<tuple<2, T0 *, T1 *>>;

struct heap_paged_chunk {
  uimax m_page_index;
  uimax m_chunk_index;
};

template <typename TupleType> struct heap_paged_impl {
  uimax m_single_page_capacity;
  vector_1<heap_impl<TupleType>> m_pages;
  pool_1<heap_paged_chunk> m_allocated_chunks;
};

template <typename TupleType>
void heap_paged_allocate(heap_paged_impl<TupleType> *thiz,
                         uimax p_page_capacity) {
  thiz->m_single_page_capacity = p_page_capacity;
  vector_allocate(&thiz->m_pages, 0);
  pool_allocate(&thiz->m_allocated_chunks, 0);
};

template <typename TupleType>
void heap_paged_free(heap_paged_impl<TupleType> *thiz) {
  for (uimax i = 0; i < thiz->m_pages.m_count; ++i) {
    heap_free(&thiz->m_pages.m_data.m_0[i]);
  }
  vector_free(&thiz->m_pages);
  pool_free(&thiz->m_allocated_chunks);
};

template <typename TupleType>
void heap_paged_push_new_page(heap_paged_impl<TupleType> *thiz) {
  vector_push(&thiz->m_pages, 1);
  heap_impl<TupleType> *l_page =
      &thiz->m_pages.m_data.m_0[thiz->m_pages.m_count - 1];
  heap_allocate(l_page, thiz->m_single_page_capacity);
};

template <typename TupleType>
uimax heap_paged_allocate_chunk(heap_paged_impl<TupleType> *thiz, uimax p_count,
                                uimax p_alignment) {

  uimax l_page_index = -1;
  uimax l_chunk_index = -1;
  for (uimax i = 0; i < thiz->m_pages.m_count; ++i) {
    heap_impl<TupleType> *l_page = &thiz->m_pages.m_data.m_0[i];
    l_chunk_index =
        heap_allocate_chunk_aligned_no_realloc(l_page, p_count, p_alignment);
    if (l_chunk_index != -1) {
      l_page_index = i;
      break;
    }
  }

  if (l_page_index == -1) {
    heap_paged_push_new_page(thiz);

    for (uimax i = 0; i < thiz->m_pages.m_count; ++i) {
      heap_impl<TupleType> *l_page = &thiz->m_pages.m_data.m_0[i];
      l_chunk_index =
          heap_allocate_chunk_aligned_no_realloc(l_page, p_count, p_alignment);
      if (l_chunk_index != -1) {
        l_page_index = i;
        break;
      }
    }
  }

  assert_debug(l_page_index != -1);
  assert_debug(l_chunk_index != -1);

  heap_paged_chunk l_allocated_chunk;
  l_allocated_chunk.m_page_index = l_page_index;
  heap_impl<TupleType> *l_page = &thiz->m_pages.m_data.m_0[l_page_index];
  l_allocated_chunk.m_chunk_index = l_chunk_index;

  uimax l_paged_chunk_index = pool_push(&thiz->m_allocated_chunks);
  thiz->m_allocated_chunks.m_elements.m_data.m_0[l_paged_chunk_index] =
      l_allocated_chunk;
  return l_paged_chunk_index;
};

template <typename TupleType>
void heap_paged_free_chunk(heap_paged_impl<TupleType> *thiz, uimax p_index) {
  assert_debug(pool_is_element_allocated(&thiz->m_allocated_chunks, p_index));
  heap_paged_chunk *l_paged_chunk =
      &thiz->m_allocated_chunks.m_elements.m_data.m_0[p_index];
  heap_impl<TupleType> *l_page =
      &thiz->m_pages.m_data.m_0[l_paged_chunk->m_page_index];
  heap_free_chunk(l_page, l_paged_chunk->m_chunk_index);
};

template <typename TupleType>
slice_impl<TupleType>
heap_paged_chunk_to_slice(heap_paged_impl<TupleType> *thiz, uimax p_index) {
  assert_debug(pool_is_element_allocated(&thiz->m_allocated_chunks, p_index));
  heap_paged_chunk *l_paged_chunk =
      &thiz->m_allocated_chunks.m_elements.m_data.m_0[p_index];
  heap_impl<TupleType> *l_page =
      &thiz->m_pages.m_data.m_0[l_paged_chunk->m_page_index];
  return heap_chunk_to_slice(l_page, l_paged_chunk->m_chunk_index);
};

template <typename TupleType>
ui8 heap_paged_check_consistency(heap_paged_impl<TupleType> *thiz) {
  for (auto i = 0; i < thiz->m_pages.m_count; ++i) {
    heap_impl<TupleType> *l_page = &thiz->m_pages.m_data.m_0[i];
    if (!heap_check_consistency(l_page)) {
      return 0;
    }
  }
  return 1;
};

template <typename T0> using heap_paged_1 = heap_paged_impl<tuple<1, T0 *>>;
template <typename T0, typename T1>
using heap_paged_2 = heap_paged_impl<tuple<2, T0 *, T1 *>>;

}; // namespace v2