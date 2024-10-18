#include "fenster.h"
#include <stdio.h>
#include <string.h>

#define W 320
#define H 240

/* ============================================================
 * Mouse buttons states example:
 * - Opens a window
 * - Starts a loop
 * - Print current state of mouse buttons (clicked and holding)
 *   only if there's a change
 * - Closes a window
 * ============================================================ */
static int run() {
  uint32_t buf[W * H];
  struct fenster f = {
      .title = "Mouse Buttons",
      .width = W,
      .height = H,
      .buf = buf,
  };
  fenster_open(&f);
  while (fenster_loop(&f) == 0) {
        printf("Click: L:%d R:%d M:%d SU:%d SD:%d | \n",
               f.mclick[0], f.mclick[1], f.mclick[2], f.mclick[3], f.mclick[4]);
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
