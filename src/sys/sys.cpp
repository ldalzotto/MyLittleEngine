#include <sys/sys.hpp>

#include <cstdlib>
#include <cstring>

void *sys::malloc(uimax p_size) { return std::malloc(p_size); };
void sys::free(void *p_ptr) { std::free(p_ptr); };
void *sys::realloc(void *p_ptr, uimax p_new_size) {
  return std::realloc(p_ptr, p_new_size);
};
void sys::memmove(void *p_dest, void *p_src, uimax p_n) {
  ::memmove(p_dest, p_src, p_n);
};
void sys::memcpy(void *p_dest, void *p_src, uimax p_n) {
  ::memcpy(p_dest, p_src, p_n);
};
void sys::memset(void *p_dest, ui32 p_value, uimax p_n) {
  ::memset(p_dest, p_value, p_n);
};
ui8 sys::memcmp(void *p_left, void *p_right, uimax p_n) {
  return ::memcmp(p_left, p_right, p_n);
};
void sys::sassert(bool p_condition) {
  if (!p_condition) {
    std::abort();
  }
};
void sys::abort() { std::abort(); };