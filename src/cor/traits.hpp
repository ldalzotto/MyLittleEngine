#pragma once

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

}; // namespace traits