#include "fenster.h"

static int run() {
    struct fenster f = {.title = "Hello World", .width = 320, .height = 240};
    fenster_open(&f);

    int frame = 0;
    char frametext[100];
    FensterFont* font = fenster_loadfont("LiberationMono-Regular.ttf");

    while (fenster_loop(&f) == 0) {
        // fast screen clear
        memset(f.buf, 0, f.width * f.height * sizeof(uint32_t));

        fenster_drawtext(&f, font, "\\c0x526D82 Hello \\s32 \\c0xDDE6ED BIG \\s16 \\c0x526D82 World \\n From FensterB!", 10, 10);

        sprintf(frametext, "\\s%d Frame: %d", f.width/16, frame);
        fenster_drawtext(&f, font, frametext, f.mpos[0], f.mpos[1]);
        fenster_sync(&f, 30);
        frame++;
    }

    fenster_freefont(font);
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
