#include "fenster.h"

static int run() {
  struct fenster f = {.title = "FensterB Shapes Gallery", .width = 800, .height = 600};
  fenster_open(&f);

  const uint32_t SHAPE_COLOR = 0x00FFFF;    // Cyan
  const uint32_t BG_COLOR = 0x202030;       // Dark blue-gray

  const int SHAPE_SIZE = 80;                // Size of each shape
  const int MARGIN = 40;                    // Space between shapes
  const int START_X = 50;                   // Starting X position
  const int START_Y = 80;                   // Starting Y position
  
  FensterFontList fonts = fenster_loadfontlist();
  FensterFont* font = fenster_loadfont(fonts.paths[0]);
  
  float rotation = 0.0f;
  
  while (fenster_loop(&f) == 0 && f.keys[27] == 0) {
    fenster_fill(&f, BG_COLOR);
    
    // Title
    fenster_drawtext(&f, font, "FensterB Shapes Gallery - Press ESC to exit", 
                     MARGIN, MARGIN);
    
    int x = START_X;
    int y = START_Y;
    const int row_height = SHAPE_SIZE + MARGIN * 2;
    
    // First Row - Filled Shapes
    // Circle
    fenster_drawcirc(&f, f.mpos[0] + SHAPE_SIZE/2, f.mpos[1] + SHAPE_SIZE/2, SHAPE_SIZE/3, SHAPE_COLOR);
    fenster_drawcirc(&f, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2, 
                     SHAPE_SIZE/3, SHAPE_COLOR);
    fenster_drawtext(&f, font, "Circle", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Rectangle
    fenster_drawrec(&f, x, y, SHAPE_SIZE, SHAPE_SIZE, SHAPE_COLOR);
    fenster_drawtext(&f, font, "Rectangle", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Triangle
    fenster_drawtri(&f,
                    x + SHAPE_SIZE/2, y + 10,
                    x + 10, y + SHAPE_SIZE - 10,
                    x + SHAPE_SIZE - 10, y + SHAPE_SIZE - 10,
                    SHAPE_COLOR);
    fenster_drawtext(&f, font, "Triangle", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Regular Polygon (Pentagon)
    fenster_drawpoly(&f, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2, 
                     5, SHAPE_SIZE/3, rotation, SHAPE_COLOR);
    fenster_drawtext(&f, font, "Pentagon", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Regular Polygon (Hexagon)
    fenster_drawpoly(&f, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2, 
                     6, SHAPE_SIZE/3, rotation, SHAPE_COLOR);
    fenster_drawtext(&f, font, "Hexagon", x, y + SHAPE_SIZE + 5);
    
    // Second Row - Outlined Shapes
    x = START_X;
    y += row_height;
    
    // Circle outline
    fenster_drawcircline(&f, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2, 
                        SHAPE_SIZE/3, SHAPE_COLOR);
    fenster_drawtext(&f, font, "LCircle", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Rectangle outline
    fenster_drawrecline(&f, x, y, SHAPE_SIZE, SHAPE_SIZE, SHAPE_COLOR);
    fenster_drawtext(&f, font, "LRect", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Triangle outline
    fenster_drawtriline(&f,
                        x + SHAPE_SIZE/2, y + 10,
                        x + 10, y + SHAPE_SIZE - 10,
                        x + SHAPE_SIZE - 10, y + SHAPE_SIZE - 10,
                        SHAPE_COLOR);
    fenster_drawtext(&f, font, "LTri", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Pentagon outline
    fenster_drawpolyline(&f, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2, 
                        5, SHAPE_SIZE/3, rotation, SHAPE_COLOR);
    fenster_drawtext(&f, font, "LPenta", x, y + SHAPE_SIZE + 5);
    x += SHAPE_SIZE + MARGIN;
    
    // Hexagon outline
    fenster_drawpolyline(&f, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2, 
                        6, SHAPE_SIZE/3, rotation, SHAPE_COLOR);
    fenster_drawtext(&f, font, "LHexa", x, y + SHAPE_SIZE + 5);
    
    // Additional Demo Info
    fenster_drawtext(&f, font, 
                     "All shapes are drawn with the same size parameters for comparison", 
                     MARGIN, f.height - 40);
    
    rotation += 0.02f;
    fenster_sync(&f, 60);
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
