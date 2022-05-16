#include <sys/sys.hpp>

#include <cmath>
#include <cstdlib>
#include <cstring>

FORCE_INLINE void *sys::malloc(uimax p_size) { return std::malloc(p_size); };
FORCE_INLINE void sys::free(void *p_ptr) { std::free(p_ptr); };
FORCE_INLINE void *sys::realloc(void *p_ptr, uimax p_new_size) {
  return std::realloc(p_ptr, p_new_size);
};
FORCE_INLINE void sys::memmove(void *p_dest, void *p_src, uimax p_n) {
  ::memmove(p_dest, p_src, p_n);
};
FORCE_INLINE void sys::memcpy(void *p_dest, void *p_src, uimax p_n) {
  ::memcpy(p_dest, p_src, p_n);
};
FORCE_INLINE void sys::memset(void *p_dest, ui32 p_value, uimax p_n) {
  ::memset(p_dest, p_value, p_n);
};
FORCE_INLINE ui8 sys::memcmp(void *p_left, void *p_right, uimax p_n) {
  return ::memcmp(p_left, p_right, p_n);
};
FORCE_INLINE void sys::sassert(bool p_condition) {
  if (!p_condition) {
    std::abort();
  }
};
FORCE_INLINE void sys::abort() { std::abort(); };

FORCE_INLINE f32 sys::sin(f32 p_angle) { return std::sin(p_angle); };
FORCE_INLINE f32 sys::cos(f32 p_angle) { return std::cos(p_angle); };