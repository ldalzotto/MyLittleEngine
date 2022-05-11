
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cor/assertions.hpp>
#include <stdio.h>
#include <sys/sys.hpp>
#include <sys/win.hpp>

inline static Display *s_display = 0;

int handler(Display *d, XErrorEvent *e) {
  sys::sassert(false);
  return 0;
}

void *win::create_window(ui32 p_width, ui32 p_height) {
  if (!s_display) {
    s_display = XOpenDisplay(0);
    XSetErrorHandler(handler);
  }
  assert_debug(s_display);

  int l_screen = DefaultScreen(s_display);
  Window l_parent_window = DefaultRootWindow(s_display);
  Window l_window = XCreateSimpleWindow(
      s_display, l_parent_window, 0, 0, p_width, p_height, 1,
      BlackPixel(s_display, l_screen), WhitePixel(s_display, l_screen));

  long event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                    ButtonPressMask | ButtonReleaseMask | FocusChangeMask;
  XSelectInput(s_display, l_window, event_mask);

  return (void *)l_window;
};

void win::show_window(void *p_window) {
  assert_debug(s_display);
  XMapWindow(s_display, (Window)p_window);
};

void win::close_window(void *p_window) { XCloseDisplay(s_display); };

void *win::allocate_image(void *p_window, void *p_buffer, ui32 p_width,
                          ui32 p_height) {
  Window l_window = (Window)p_window;

  return XCreateImage(s_display,
                      DefaultVisual(s_display, DefaultScreen(s_display)),
                      DefaultDepth(s_display, DefaultScreen(s_display)),
                      ZPixmap, 0, (char *)p_buffer, p_width, p_height, 8, 0);
};

void win::draw(void *p_window, void *p_image, ui32 p_width, ui32 p_height) {
  XPutImage(s_display, (Window)p_window,
            DefaultGC(s_display, DefaultScreen(s_display)), (XImage *)p_image,
            0, 0, 0, 0, p_width, p_height);
};

container::vector<win::event> *
get_events_from_window(Window p_window,
                       container::vector<win::events> &p_events) {
  for (auto i = 0; i < p_events.count(); ++i) {
    if (p_events.at(i).m_window == (void *)p_window) {
      return &p_events.at(i).m_events;
    }
  }
  return 0;
};

eng::input::Key native_key_to_input(KeySym p_key_sym) {
  switch (p_key_sym) {
  case XK_Up:
    return eng::input::Key::ARROW_UP;
  case XK_Down:
    return eng::input::Key::ARROW_DOWN;
  case XK_Left:
    return eng::input::Key::ARROW_LEFT;
  case XK_Right:
    return eng::input::Key::ARROW_RIGHT;
  default:
    return eng::input::Key::UNDEFINED;
  }
};

void win::fetch_events(container::vector<events> &in_out_events) {
  int l_count = XPending(s_display);
  for (auto i = 0; i < l_count; ++i) {
    XEvent l_event;
    XNextEvent(s_display, &l_event);
    if (l_event.type == KeyPress) {
      auto *l_events =
          get_events_from_window(l_event.xkey.window, in_out_events);
      event l_engine_event;
      l_engine_event.m_type = event::type::InputPress;
      l_engine_event.m_input.m_key =
          native_key_to_input(XLookupKeysym(&l_event.xkey, 0));
      l_events->push_back(l_engine_event);
    } else if (l_event.type == KeyRelease) {
      auto *l_events =
          get_events_from_window(l_event.xkey.window, in_out_events);
      event l_engine_event;
      l_engine_event.m_type = event::type::InputRelease;
      l_engine_event.m_input.m_key =
          native_key_to_input(XLookupKeysym(&l_event.xkey, 0));
      l_events->push_back(l_engine_event);
    } else if (l_event.type == Expose) {
      auto *l_events =
          get_events_from_window(l_event.xkey.window, in_out_events);
      /*
  XImage *l_image =
      XGetImage(l_event.xexpose.display, l_event.xexpose.window,
                l_event.xexpose.x, l_event.xexpose.y, l_event.xexpose.width,
                l_event.xexpose.height, AllPlanes, ZPixmap);

  ui8 *l_buffer = (ui8 *)l_image->data;
  for (auto j = 0; j < l_image->height; j++) {
    for (auto i = 0; i < l_image->width; i++) {
      l_buffer[i + (j * l_image->width)] = 0;
    }
  }
*/
      /*
            XCopyArea(l_event.xexpose.display, l_image, l_image, 0, 0, 0, 0, 0,
         0, 0);
      */
      event l_engine_event;
      l_engine_event.m_type = event::type::Redraw;
      /*
      l_engine_event.m_draw.m_buffer = l_image->data;
      */
      l_engine_event.m_draw.m_width = l_event.xexpose.width;
      l_engine_event.m_draw.m_height = l_event.xexpose.height;

      l_events->push_back(l_engine_event);
    }
  }
};
