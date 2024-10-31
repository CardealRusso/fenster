#ifndef FENSTER_LINUX_H
#define FENSTER_LINUX_H

// clang-format off
static int FENSTER_KEYCODES[124] = {XK_BackSpace,8,XK_Delete,127,XK_Down,18,XK_End,5,XK_Escape,27,XK_Home,2,XK_Insert,26,XK_Left,20,XK_Page_Down,4,XK_Page_Up,3,XK_Return,10,XK_Right,19,XK_Tab,9,XK_Up,17,XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_a,65,XK_b,66,XK_c,67,XK_d,68,XK_e,69,XK_f,70,XK_g,71,XK_h,72,XK_i,73,XK_j,74,XK_k,75,XK_l,76,XK_m,77,XK_n,78,XK_o,79,XK_p,80,XK_q,81,XK_r,82,XK_s,83,XK_t,84,XK_u,85,XK_v,86,XK_w,87,XK_x,88,XK_y,89,XK_z,90,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};
static Atom wmDeleteWindow;
static Atom wm_state;
static Atom wm_fullscreen;
static Cursor cursors[6];
static int cursors_initialized = 0;
static int current_cursor_type = 1;
// clang-format on

FENSTER_API int fenster_open(struct fenster *f) {
  f->buf = (uint32_t*)malloc(f->width * f->height * sizeof(uint32_t));
  f->dpy = XOpenDisplay(NULL);
  int screen = DefaultScreen(f->dpy);
  f->w = XCreateSimpleWindow(f->dpy, RootWindow(f->dpy, screen), 0, 0, f->width,
                             f->height, 0, BlackPixel(f->dpy, screen),
                             WhitePixel(f->dpy, screen));
  f->gc = XCreateGC(f->dpy, f->w, 0, 0);

  wmDeleteWindow = XInternAtom(f->dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(f->dpy, f->w, &wmDeleteWindow, 1);
  wm_state = XInternAtom(f->dpy, "_NET_WM_STATE", False);
  wm_fullscreen = XInternAtom(f->dpy, "_NET_WM_STATE_FULLSCREEN", False);

  XSelectInput(f->dpy, f->w,
               ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                   ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
  XStoreName(f->dpy, f->w, f->title);
  XMapWindow(f->dpy, f->w);
  XSync(f->dpy, f->w);
  f->img = XCreateImage(f->dpy, DefaultVisual(f->dpy, 0), 24, ZPixmap, 0,
                        (char *)f->buf, f->width, f->height, 32, 0);
  return 0;
}
FENSTER_API void fenster_close(struct fenster *f) {
  XDestroyWindow(f->dpy, f->w);
  if (cursors_initialized) {
    for (int i = 1; i <= 5; i++) {
      XFreeCursor(f->dpy, cursors[i]);
    }
    cursors_initialized = 0;
  }
  XCloseDisplay(f->dpy);
}
FENSTER_API int fenster_loop(struct fenster *f) {
  XEvent ev;
  XPutImage(f->dpy, f->w, f->gc, f->img, 0, 0, 0, 0, f->width, f->height);
  XFlush(f->dpy);
  memset(f->mclick, 0, sizeof(f->mclick));
  memset(f->keysp, 0, sizeof(f->keysp));
  f->resized = 0;
  while (XPending(f->dpy)) {
    XNextEvent(f->dpy, &ev);
    switch (ev.type) {
case ConfigureNotify: {
    if (ev.xconfigure.width != f->width || ev.xconfigure.height != f->height) {
        uint32_t *new_buf = realloc(f->buf, ev.xconfigure.width * ev.xconfigure.height * sizeof(uint32_t));
        if (!new_buf) break;

        f->resized = 1;
        f->img->data = NULL;
        XDestroyImage(f->img);

        f->buf = new_buf;
        f->width = ev.xconfigure.width;
        f->height = ev.xconfigure.height;

        f->img = XCreateImage(f->dpy, DefaultVisual(f->dpy, 0), 24, ZPixmap, 0,
                            (char *)f->buf, f->width, f->height, 32, 0);
    }
} break;
    case ClientMessage:
      if ((Atom)ev.xclient.data.l[0] == wmDeleteWindow) {
        return 1;
      } 
    break;
    case ButtonPress:
      switch (ev.xbutton.button) {
        case Button1: f->mclick[0] = f->mhold[0] = 1; break;
        case Button3: f->mclick[1] = f->mhold[1] = 1; break;
        case Button2: f->mclick[2] = f->mhold[2] = 1; break;
        case Button4: f->mclick[3] = 1; break;
        case Button5: f->mclick[4] = 1; break;
      }
      break;
    case ButtonRelease:
      switch (ev.xbutton.button) {
        case Button1: f->mhold[0] = 0; break;
        case Button3: f->mhold[1] = 0; break;
        case Button2: f->mhold[2] = 0; break;
      }
      break;
    case MotionNotify:
      f->mpos[0] = ev.xmotion.x;
      f->mpos[1] = ev.xmotion.y;
      break;
case KeyPress: {
  int m = ev.xkey.state;
  int k = XkbKeycodeToKeysym(f->dpy, ev.xkey.keycode, 0, 0);
  for (unsigned int i = 0; i < 124; i += 2) {
    if (FENSTER_KEYCODES[i] == k) {
      f->keys[FENSTER_KEYCODES[i + 1]] = 1;  // Set keys
      f->keysp[FENSTER_KEYCODES[i + 1]] = 1; // Set keysp
      break;
    }
  }
  f->modkeys[0] = !!(m & ControlMask);
  f->modkeys[1] = !!(m & ShiftMask);
  f->modkeys[2] = !!(m & Mod1Mask);
  f->modkeys[3] = !!(m & Mod4Mask);
} break;
case KeyRelease: {
  int m = ev.xkey.state;
  int k = XkbKeycodeToKeysym(f->dpy, ev.xkey.keycode, 0, 0);
  for (unsigned int i = 0; i < 124; i += 2) {
    if (FENSTER_KEYCODES[i] == k) {
      f->keys[FENSTER_KEYCODES[i + 1]] = 0;  // Only update keys
      break;
    }
  }
  f->modkeys[0] = !!(m & ControlMask);
  f->modkeys[1] = !!(m & ShiftMask);
  f->modkeys[2] = !!(m & Mod1Mask);
  f->modkeys[3] = !!(m & Mod4Mask);
} break;
    }
  }
  return 0;
}

FENSTER_API void fenster_sleep(int64_t ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

FENSTER_API int64_t fenster_time(void) {
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  return time.tv_sec * 1000 + (time.tv_nsec / 1000000);
}

FENSTER_API void fenster_sync(struct fenster *f, int fps) {
  int64_t frame_time = 1000 / fps;
  int64_t elapsed = fenster_time() - f->lastsync;
  if (elapsed < frame_time) {
    fenster_sleep(frame_time - elapsed);
  }

  f->lastsync = fenster_time();
}

FENSTER_API void fenster_resize(struct fenster *f, int width, int height) {
    XResizeWindow(f->dpy, f->w, width, height);
    XFlush(f->dpy);
}

FENSTER_API void fenster_fullscreen(struct fenster *f, int enabled) {
    XEvent xev = {0};
    xev.type = ClientMessage;
    xev.xclient.window = f->w;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = enabled ? 1 : 0;  // 1 = add, 0 = remove
    xev.xclient.data.l[1] = wm_fullscreen;
    xev.xclient.data.l[2] = 0;

    XSendEvent(f->dpy, DefaultRootWindow(f->dpy), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &xev);
    XFlush(f->dpy);
}

FENSTER_API void fenster_cursor(struct fenster *f, int type) {
    if (type == current_cursor_type) return;
    if (!cursors_initialized) {
        cursors[0] = 0;                                           // None/hidden
        cursors[1] = XCreateFontCursor(f->dpy, XC_left_ptr);      // Normal arrow
        cursors[2] = XCreateFontCursor(f->dpy, XC_hand2);         // Pointer/hand
        cursors[3] = XCreateFontCursor(f->dpy, XC_watch);         // Progress
        cursors[4] = XCreateFontCursor(f->dpy, XC_crosshair);     // Crosshair
        cursors[5] = XCreateFontCursor(f->dpy, XC_xterm);         // Text
        cursors_initialized = 1;
    }

    if (type == 0) {
        // Hide cursor
        XColor dummy;
        Pixmap blank = XCreateBitmapFromData(f->dpy, f->w, "\0\0\0\0\0\0\0\0", 1, 1);
        Cursor invisible = XCreatePixmapCursor(f->dpy, blank, blank, &dummy, &dummy, 0, 0);
        XDefineCursor(f->dpy, f->w, invisible);
        XFreeCursor(f->dpy, invisible);
        XFreePixmap(f->dpy, blank);
    } else {
        // Set the cursor from our pre-created cursors
        XDefineCursor(f->dpy, f->w, cursors[type]);
    }

    current_cursor_type = type;
    XFlush(f->dpy);
}
#endif /* FENSTER_LINUX_H */
