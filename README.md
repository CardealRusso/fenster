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
- Window resize (fenster_resize)
- Cursor pointer (fenster_cursor = 0 hidden, 1 normal, 2 pointer, 3 progress, 4 crosshair, 5 text)
- Fast fill (memset for 0, avx2 for non-zero)
- TTF text drawing with basic formatting syntax (\\\c(color) \\\b(ackground color) \\\s(font size) \\\n(newline)  \\\p(spacing) \\\h(line spacing))
- Added: fenster_fill, fenster_fullscreen, fenster_sync, fenster_resize, fenster_loadfont, fenster_(loadfont, findfontin, freefont)list, fenster_drawtext
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

FENSTER_API void fenster_sync(struct fenster *f, int fps);
FENSTER_API void fenster_resize(struct fenster *f, int width, int height);
FENSTER_API void fenster_fullscreen(struct fenster *f, int enabled);
FENSTER_API void fenster_cursor(struct fenster *f, int type);
static inline void fenster_fill(struct fenster *f, uint32_t color)

#ifdef USE_FONTS
FensterFont* fenster_loadfont(const char* filename);
void fenster_drawtext(struct fenster* f, FensterFont* font, const char* text, int x, int y);
void fenster_freefont(FensterFont* font);

FensterFontList fenster_loadfontlist();
int fenster_findfontinlist(FensterFontList *fonts, const char *term);
void fenster_freefontlist(FensterFontList *fonts);
#endif

```
