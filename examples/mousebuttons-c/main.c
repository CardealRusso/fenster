#include "fenster.h"
#include <stdio.h>
#include <string.h>

static int run() {
  struct fenster f = {.title = "Mouse Buttons",.width = 320,.height = 240,};
  fenster_open(&f);
  while (fenster_loop(&f) == 0) {
    for (int i = 0; i < f.width; i++) {
      for (int j = 0; j < f.height; j++) {
        fenster_pixel(&f, i, j) = rand();
      }
    }
    printf("Click: L:%d R:%d M:%d SU:%d SD:%d | \n",f.mclick[0], f.mclick[1], f.mclick[2], f.mclick[3], f.mclick[4]);
    fenster_sync(&f, 60);
  }

  fenster_close(&f);
  return 0;
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine,
                   int nCmdShow) {
  (void)hInstance, (void)hPrevInstance, (void)pCmdLine, (void)nCmdShow;
  return run();
}
#else
int main() { return run(); }
#endif
