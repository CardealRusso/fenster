#include "fenster.h"

static int run() {
    struct fenster f = {.title = "Hello World", .width = 320, .height = 240};
    fenster_open(&f);

    int frame = 0;
    int fs = 0;
    char frametext[100];
    FensterFontList fonts = fenster_loadfontlist();
    FensterFont* font = fenster_loadfont(fonts.paths[0]);

    while (fenster_loop(&f) == 0 && f.keys[27] == 0) {
        fenster_fill(&f, 0);

        fenster_drawtext(&f, font, "\\c0x526D82 Hello \\s32 \\c0xDDE6ED BIG \\s16 \\c0x526D82 World \\n From FensterB!", 10, 10);

        sprintf(frametext, "\\s%d Frame: %d", f.width/16, frame);
        fenster_drawtext(&f, font, frametext, f.mpos[0], f.mpos[1]);
        fenster_sync(&f, 30);

        if (f.keys[70] == 1) {
          if (fs == 0) {
            fenster_fullscreen(&f, 1);
            fs = 1;
            fenster_sleep(100);
          } else {
            fenster_fullscreen(&f, 0);
            fs = 0;
            fenster_sleep(100);
          }
        }
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
