#pragma once

struct none {};

namespace traits {

template <bool B, class T = void> struct enable_if {};

template <class T> struct enable_if<true, T> { using type = T; };

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename T> struct remove_const { using type = T; };
template <typename T> struct remove_const<const T> { using type = T; };

template <typename T> struct remove_ref { using type = T; };
template <typename T> struct remove_ref<T &> { using type = T; };

template <typename T> struct remove_ptr { using type = T; };
template <typename T> struct remove_ptr<T *> { using type = T; };

template <typename T> struct remove_ptr_ref {
  using type = typename remove_ptr<typename remove_ref<T>::type>::type;
};

template <typename T> using remove_ptr_ref_t = typename remove_ptr_ref<T>::type;

template <bool B, class T, class F> struct conditional { using type = T; };

template <class T, class F> struct conditional<false, T, F> { using type = F; };

template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

template <typename T> struct is_none { static constexpr bool value = 0; };

template <> struct is_none<none> { static constexpr bool value = 1; };

}; // namespace traits

#define api_decltype(api_type, var_name, code)                                 \
  api_type<traits::remove_ptr_ref_t<decltype(code)>> var_name = code