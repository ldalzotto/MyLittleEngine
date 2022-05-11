#pragma once

extern "C" {

typedef struct engine_handle {
  void *m_ptr;
} engine_handle;

typedef struct window_handle {
  void *m_ptr;
} window_handle;

extern engine_handle engine_allocate();
extern void engine_free(engine_handle);
extern void engine_update(engine_handle);

extern window_handle engine_widow_open(engine_handle, unsigned short width,
                                       unsigned short height);
extern void engine_window_close(engine_handle, window_handle);
}