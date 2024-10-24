#ifndef FENSTER_H
#define FENSTER_H

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>
#include <ApplicationServices/ApplicationServices.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#define _DEFAULT_SOURCE 1
#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <time.h>
#endif

#include <stdint.h>
#include <string.h>

struct fenster {
  const char *title;
  int width;
  int height;
  uint32_t *buf;
  int keys[256];      // keys are mostly ASCII, but arrows are 17..20
  int keysp[256];     // similar to mclick but for keys
  int modkeys[4];     // ctrl, shift, alt, meta
  int mpos[2];        // mouse x, y
  int mclick[5];      // left, right, middle, scroll up, scroll down (one click)
  int mhold[3];       // left, right, middle (persistent until release)
  int64_t lastsync;   // last sync time
  int resized;
#if defined(__APPLE__)
  id wnd;
#elif defined(_WIN32)
  HWND hwnd;
#else
  Display *dpy;
  Window w;
  GC gc;
  XImage *img;
#endif
};

#ifndef FENSTER_API
#define FENSTER_API extern
#endif

FENSTER_API int fenster_open(struct fenster *f);
FENSTER_API int fenster_loop(struct fenster *f);
FENSTER_API void fenster_close(struct fenster *f);
FENSTER_API void fenster_sleep(int64_t ms);
FENSTER_API int64_t fenster_time(void);
FENSTER_API void fenster_sync(struct fenster *f, int fps);
FENSTER_API void fenster_resize(struct fenster *f, int width, int height);
FENSTER_API void fenster_fullscreen(struct fenster *f, int enabled);
FENSTER_API void fenster_cursor(struct fenster *f, int type);

#define fenster_pixel(f, x, y) ((f)->buf[((y) * (f)->width) + (x)])

#if defined(__APPLE__)
#include "fenster_mac.h"
#elif defined(_WIN32)
#include "fenster_windows.h"
#else
#include "fenster_linux.h"
#endif

#include "../fenster_addons.h"

#ifdef USE_FONTS
#include "../fenster_font.h"
#endif

#endif /* FENSTER_H */
