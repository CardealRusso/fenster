#include "fenster.h"

static int run() {
  struct fenster f = {.title = "FensterB Shapes Demo", .width = 800, .height = 600};
  fenster_open(&f);
  
  int frame = 0;
  int fs = 0;
  
  FensterFontList fonts = fenster_loadfontlist();
  FensterFont* font = fenster_loadfont(fonts.paths[0]);
  
  const uint32_t RED = 0xFF0000;
  const uint32_t BLUE = 0x0000FF;
  const uint32_t GREEN = 0x00FF00;
  const uint32_t WHITE = 0xFFFFFF;

  int circleRadius = 50;
  int direction = 1;

  fenster_cursor(&f, 0);
  while (fenster_loop(&f) == 0 && f.keys[27] == 0) {
    fenster_fill(&f, 0);

    int mouseX = f.mpos[0];
    int mouseY = f.mpos[1];

    circleRadius += direction;
    if (circleRadius >= 100 || circleRadius <= 30) direction *= -1;

    fenster_drawcircline(&f, mouseX, mouseY-(f.height/4), circleRadius, BLUE);

    fenster_drawrec(&f, 50, 50, 100, 80, RED);
    fenster_drawrec(&f, f.width-150, 50, 100, 80, GREEN);
    fenster_drawrec(&f, 50, f.height-130, 100, 80, WHITE);
    fenster_drawrec(&f, f.width-150, f.height-130, 100, 80, WHITE);
    
    fenster_drawline(&f, 0, 0, mouseX, mouseY, RED);
    fenster_drawline(&f, f.width, 0, mouseX, mouseY, GREEN);
    fenster_drawline(&f, 0, f.height, mouseX, mouseY, WHITE);
    fenster_drawline(&f, f.width, f.height, mouseX, mouseY, WHITE);

    vsformat("Frame: %d", frame);
    fenster_drawtext(&f, font, vsbuff, 10, 10);

    vsformat("Press F for fullscreen | ESC to exit | Mouse: (%d, %d)", mouseX, mouseY);
    fenster_drawtext(&f, font, vsbuff, 10, f.height - 30);
    
    fenster_sync(&f, 60);
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
