#pragma once

extern "C" {

typedef struct engine_handle {
  void *m_ptr;
} engine_handle;

typedef struct window_handle {
  void *m_ptr;
} window_handle;

extern engine_handle engine_allocate(unsigned short width,
                                       unsigned short height);
extern void engine_free(engine_handle);
extern unsigned char engine_update(engine_handle);
}