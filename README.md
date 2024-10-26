# FensterB

[Fenster](https://github.com/zserge/fenster) with some bloated code

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.

## Main changes
- Source split by system
- Platform-agnostic structure for audio
- Mouse: Left, Right, Middle, Scrollup/down
- Full Screen mode (fenster_fullscreen = 0 to windowed, 1 to fullscreen)
- Buf size is handled internally
- Basic 2d shapes
- Window resize (fenster_resize)
- Cursor pointer (fenster_cursor = 0 hidden, 1 normal, 2 pointer, 3 progress, 4 crosshair, 5 text)
- Sync command
- Fast fill (memset for 0, avx2 (windows, linux) or neon (mac, arm, etc) for non-zero)
- TTF text drawing with basic formatting syntax (\\\c(color) \\\b(ackground color (defaults to magic number 0xFFFFFFFF for transparency) \\\s(font size) \\\n(newline)  \\\p(spacing) \\\h(line spacing))
- Changed: array on modkeys, mouse pos

```C
struct fenster {
  const char *title;
  int width;
  int height;
  int keys[256];      // keys are mostly ASCII, but arrows are 17..20
  int keysp[256];     // similar to mclick but for keys. Unexpected behavior. On Linux it always returns 1 if held for more than 1~2 seconds.
  int modkeys[4];     // ctrl, shift, alt, meta
  int mpos[2];        // mouse x, y
  int mclick[5];      // left, right, middle, scroll up, scroll down (refreshed after loop)
  int mhold[3];       // left, right, middle (persistent until button release)
  int64_t lastsync;   // last sync time
  int resized;
};

void fenster_sync(struct fenster *f, int fps);
void fenster_resize(struct fenster *f, int width, int height);
void fenster_fullscreen(struct fenster *f, int enabled);
void fenster_cursor(struct fenster *f, int type);

#ifdef USE_FONTS
FensterFont* fenster_loadfont(const char* filename);
void fenster_drawtext(struct fenster* f, FensterFont* font, const char* text, int x, int y);
void fenster_freefont(FensterFont* font);

FensterFontList fenster_loadfontlist();
int fenster_findfontinlist(FensterFontList *fonts, const char *term);
void fenster_freefontlist(FensterFontList *fonts);
#endif

/* FENSTER_ADDONS_H */
void fenster_drawline(struct fenster *f, int x1, int y1, int x2, int y2, uint32_t color);
void fenster_drawcirc(line)(struct fenster *f, int x0, int y0, int radius, uint32_t color);
void fenster_drawrec(line)(struct fenster *f, int x, int y, int width, int height, uint32_t color);
void fenster_drawpoly(line)(struct fenster *f, int cx, int cy, int sides, int radius, float rotation, uint32_t color);
void fenster_drawtri(line)(struct fenster *f, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void fenster_fill(struct fenster *f, uint32_t color);
```
