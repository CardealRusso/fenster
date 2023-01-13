#ifndef FENSTER_LINUX_H
#define FENSTER_LINUX_H

// clang-format off
static int FENSTER_KEYCODES[124] = {XK_BackSpace,8,XK_Delete,127,XK_Down,18,XK_End,5,XK_Escape,27,XK_Home,2,XK_Insert,26,XK_Left,20,XK_Page_Down,4,XK_Page_Up,3,XK_Return,10,XK_Right,19,XK_Tab,9,XK_Up,17,XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_a,65,XK_b,66,XK_c,67,XK_d,68,XK_e,69,XK_f,70,XK_g,71,XK_h,72,XK_i,73,XK_j,74,XK_k,75,XK_l,76,XK_m,77,XK_n,78,XK_o,79,XK_p,80,XK_q,81,XK_r,82,XK_s,83,XK_t,84,XK_u,85,XK_v,86,XK_w,87,XK_x,88,XK_y,89,XK_z,90,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};
// clang-format on

FENSTER_API int fenster_open(struct fenster *f) {
  f->dpy = XOpenDisplay(NULL);
  int screen = DefaultScreen(f->dpy);
  f->w = XCreateSimpleWindow(f->dpy, RootWindow(f->dpy, screen), 0, 0, f->width,
                             f->height, 0, BlackPixel(f->dpy, screen),
                             WhitePixel(f->dpy, screen));
  f->gc = XCreateGC(f->dpy, f->w, 0, 0);
  XSelectInput(f->dpy, f->w,
               ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                   ButtonReleaseMask | PointerMotionMask);
  XStoreName(f->dpy, f->w, f->title);
  XMapWindow(f->dpy, f->w);
  XSync(f->dpy, f->w);
  f->img = XCreateImage(f->dpy, DefaultVisual(f->dpy, 0), 24, ZPixmap, 0,
                        (char *)f->buf, f->width, f->height, 32, 0);
  return 0;
}
FENSTER_API void fenster_close(struct fenster *f) { XCloseDisplay(f->dpy); }
FENSTER_API int fenster_loop(struct fenster *f) {
  XEvent ev;
  XPutImage(f->dpy, f->w, f->gc, f->img, 0, 0, 0, 0, f->width, f->height);
  XFlush(f->dpy);
  memset(f->mclick, 0, sizeof(f->mclick));
  while (XPending(f->dpy)) {
    XNextEvent(f->dpy, &ev);
    switch (ev.type) {
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
      f->x = ev.xmotion.x, f->y = ev.xmotion.y;
      break;
    case KeyPress:
    case KeyRelease: {
      int m = ev.xkey.state;
      int k = XkbKeycodeToKeysym(f->dpy, ev.xkey.keycode, 0, 0);
      for (unsigned int i = 0; i < 124; i += 2) {
        if (FENSTER_KEYCODES[i] == k) {
          f->keys[FENSTER_KEYCODES[i + 1]] = (ev.type == KeyPress);
          break;
        }
      }
      f->mod = (!!(m & ControlMask)) | (!!(m & ShiftMask) << 1) |
               (!!(m & Mod1Mask) << 2) | (!!(m & Mod4Mask) << 3);
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

#endif /* FENSTER_LINUX_H */
