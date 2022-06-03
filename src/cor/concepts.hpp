#pragma once

#include <sys/sys.hpp>

#if DEBUG_PREPROCESS
#define CONCEPTS_ENABLED 1
#else
#define CONCEPTS_ENABLED 0
#endif

#if CONCEPTS_ENABLED
#define CONCEPT_OVERLOAD(p_code) p_code
#else
#define CONCEPT_OVERLOAD(p_code)
#endif

namespace container {

#if CONCEPTS_ENABLED

template <typename RangeType,
          typename _element_type = typename RangeType::element_type>
struct range_v_c {
  RangeType m_value;
  using element_type = _element_type;
  // using element_type = element_type;

  operator RangeType &() { return m_value; };
  range_v_c() = default;
  range_v_c(const RangeType &p_value) : m_value(p_value){};

  uimax &count() { return m_value.count(); };
  const uimax &count() const { return m_value.count(); };
  element_type &at(uimax p_index) { return m_value.at(p_index); };

  range_v_c slide(uimax p_offset) const { return m_value.slide(p_offset); };
  range_v_c shrink_to(uimax p_count) const {
    return m_value.shrink_to(p_count);
  };

  static range_v_c make(element_type *p_value, uimax p_count) {
    return RangeType::make(p_value, p_count);
  };
};

template <typename RangeType,
          typename element_type = typename RangeType::element_type>
using range_c = range_v_c<RangeType &, element_type>;

template <typename RangeType,
          typename element_type = typename RangeType::element_type>
struct range_const_c {
  const RangeType &m_value;

  operator RangeType &() { return m_value; };
  range_const_c(const RangeType &p_value) : m_value(p_value){};

  const uimax &count() { return m_value.count(); };
  const element_type &at(uimax p_index) { return m_value.at(p_index); };

  range_const_c slide(uimax p_offset) const { return m_value.slide(p_offset); };
  range_const_c shrink_to(uimax p_count) const {
    return m_value.shrink_to(p_count);
  };
};

#else


template <typename RangeType,
          typename element_type = typename RangeType::element_type>
using range_v_c = RangeType;

template <typename RangeType,
          typename element_type = typename RangeType::element_type>
using range_c = RangeType;

template <typename RangeType,
          typename element_type = typename RangeType::element_type>
using range_const_c = RangeType;

#endif

} // namespace container