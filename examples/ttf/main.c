#include "fenster.h"

static int run() {
  struct fenster f = {.title = "Hello World", .width = 320, .height = 240};
  fenster_open(&f);

  int frame = 0;
  int fs = 0;
  FensterFontList fonts = fenster_loadfontlist();
  FensterFont* font = fenster_loadfont(fonts.paths[0]);

  while (fenster_loop(&f) == 0 && f.keys[27] == 0) {

    if (f.resized == 1 || frame == 0) {
      fenster_fill(&f, 0);
      fenster_drawtext(&f, font, "\\c0x526D82 Hello \\s32 \\c0xDDE6ED BIG \\s16 \\c0x526D82 World \\n From FensterB!", 10, 10);

      fenster_drawtext(&f, font, vsformat("\\s16 \\c0xFFFFFF Press F to %s Full screen", fs == 0 ? "enter" : "leave"), 0, 200);
      fenster_drawtext(&f, font, vsformat("\\c%d To save CPU cycles, we\\n reprint this text only\\n when window is resized.", rand()), 0, 100);
    }

    fenster_drawtext(&f, font, vsformat("\\b%d \\s16 Frame: %d", rand(), frame), 0, f.height-16);
    fenster_sync(&f, 30);

    if (f.keysp[70] == 1) fenster_fullscreen(&f, fs ^= 1);

    frame++;
  }

  fenster_freefont(font);
  fenster_freefontlist(&fonts);
  fenster_close(&f);
  return 0;
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    (void)hInstance, (void)hPrevInstance, (void)pCmdLine, (void)nCmdShow;
    return run();
}
#else
int main() { return run(); }
#endif
