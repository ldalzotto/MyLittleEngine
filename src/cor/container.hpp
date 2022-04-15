#pragma once

#include <cor/cor.hpp>
#include <sys/sys.hpp>

#define BOOST_NO_EXCEPTIONS
#include <boost/container/vector.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/simple_segregated_storage.hpp>
#include <boost/range/algorithm.hpp>

namespace eng {

template <typename T> struct object_pool_indexed {
private:
  boost::container::vector<T> data;
  boost::container::vector<uimax> free_elements;

public:
  object_pool_indexed() = default;
  object_pool_indexed(const object_pool_indexed &) = default;

  object_pool_indexed(uimax p_capacity) { data.reserve(p_capacity); };

public:
  uimax malloc(const T &p_element) {
    if (free_elements.size() > 0) {
      auto l_index = free_elements.at(free_elements.size() - 1);
      free_elements.pop_back();
      data.at(l_index.value) = p_element;
      return l_index;
    } else {
      data.push_back(p_element);
      return uimax(data.size() - 1);
    }
  };

  void free(uimax p_index) {
    sys::sassert(is_element_allocated(p_index)); // TODO add debug for this
    free_elements.push_back(p_index);
  };

  T &at(uimax p_index) {
    sys::sassert(is_element_allocated(p_index)); // TODO add debug for this
    return data.at(p_index.value);
  };

  ui32_t free_elements_size() { return free_elements.size(); };

private:
  bool is_element_allocated(uimax p_index) {
    auto l_found = boost::range::find_if(free_elements, [&](auto l_free_index) {
      return l_free_index == p_index;
    });
    return l_found == free_elements.end();
  };
};

} // namespace eng
