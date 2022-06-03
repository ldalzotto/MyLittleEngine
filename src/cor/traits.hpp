#pragma once

template <bool B, class T = void> struct enable_if {};

template <class T> struct enable_if<true, T> { using type = T; };

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

namespace traits {

/*
template <typename T> struct is_const { static constexpr bool value = false; };
template <typename T> struct is_const<const T> {
  static constexpr bool value = false;
};
*/

template <typename T> struct remove_const { using type = T; };
template <typename T> struct remove_const<const T> { using type = T; };

}; // namespace traits