#include "fenster.h"
#include <math.h>
#include <stdio.h>

static int run() {
    struct fenster f = {.title = "Mouse Buttons", .width = 320, .height = 240};
    fenster_open(&f);
    
    int frame = 0;

    while (fenster_loop(&f) == 0) {
        // Limpa a tela
        memset(f.buf, 0, f.width * f.height * sizeof(uint32_t));

        int centerX = f.width / 2;
        int centerY = f.height / 2;
        int radius = f.width / 4;

        for (int y = 0; y < f.height; y++) {
            for (int x = 0; x < f.width; x++) {
                int dx = x - centerX;
                int dy = y - centerY;
                int distSquared = dx * dx + dy * dy;

                if (distSquared <= radius * radius) {
                    int dist = (int)sqrt(distSquared);
                    int expandFactor = (frame + dist) % 256; // Movimento baseado em frame

                    // Cor que se expande
                    uint32_t color = (expandFactor % 256) << 16 | (expandFactor % 256) << 8 | (255 - expandFactor % 256);
                    fenster_pixel(&f, x, y) = color;
                }
            }
        }

        printf("Click: L:%d R:%d M:%d SU:%d SD:%d | \n", f.mclick[0], f.mclick[1], f.mclick[2], f.mclick[3], f.mclick[4]);
        fenster_sync(&f, 60);

        frame++;
    }

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
