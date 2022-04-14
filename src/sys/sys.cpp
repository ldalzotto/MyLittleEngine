#include <sys/sys.hpp>

#include <cstdlib>

namespace {

void *sys::malloc(uimax p_size) { return std::malloc(p_size.value); };
void sys::free(void *p_ptr) { std::free(p_ptr); };
void *sys::realloc(void *p_ptr, uimax p_new_size) {
  return std::realloc(p_ptr, p_new_size.value);
};
void sys::sassert(bool p_condition) {
  if (!p_condition) {
    std::abort();
  }
};
} // namespace