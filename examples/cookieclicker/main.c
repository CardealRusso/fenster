#include "fenster.h"
#include <math.h>

struct AutoClicker {
  const char* name;
  __uint128_t cost, cps, owned;
  int unlocked, bonus;
};

static struct AutoClicker autoclickers[] = {
  {"Cursor", 15, 1, 0, 0, 1},
  {"Grandma", 100, 4, 0, 0, 1},
  {"Farm", 1100, 8, 0, 0, 1},
  {"Mine", 12000, 47, 0, 0, 1},
  {"Factory", 130000, 260, 0, 0, 1},
  {"Bank", 1400000, 1400, 0, 0, 1},
  {"Temple", 20000000, 7800, 0, 0, 1},
  {"Wizard Tower", 330000000, 44000, 0, 0, 1},
  {"Shipment", 5000000000, 260000, 0, 0, 1}
};

int fs = 0;
uint32_t bonus;

int run() {
  struct fenster f = {.title = "Cookie Clicker", .width = 800, .height = 600};
  fenster_open(&f);

  FensterFontList fonts = fenster_loadfontlist();
  FensterFont* font = fenster_loadfont(fonts.paths[0]);

  __uint128_t cookies = 0, last_time = 0, cookies_per_sec = 0;
  float cookie_size = 60.f, cookie_grow_rate = 1.1f, cookie_max_size = 100.f;
  int cookie_growing = 0;
  int bonus = 0;

  while (fenster_loop(&f) == 0 && f.keys[27] == 0) {
    fenster_fill(&f, 0x2A2A2A);

    int hovering_cursor = 1;
    int cookie_x = f.width / 2, cookie_y = f.height / 2;

    if (cookie_growing) {
      cookie_size = fminf(cookie_size * cookie_grow_rate, cookie_max_size);
    } else {
      cookie_size = 60.f;
    }
    fenster_drawcirc(&f, cookie_x, cookie_y, (int)cookie_size, 0x8B4513);

    if (fenster_point_in_circle(f.mpos[0], f.mpos[1], cookie_x, cookie_y, (int)cookie_size)) {
      hovering_cursor = 2;
      if (f.mclick[0]) {
        cookies += 1 + (autoclickers[0].owned * autoclickers[0].bonus);
        cookie_growing = 0;
      } else {
        cookie_growing = 1;
      }
    } else {
      cookie_growing = 0;
    }

    int64_t current_time = f.lastsync;
    if (current_time - last_time >= 1000) {
      cookies_per_sec = 0;
      for (int i = 0; i < 9; i++) {
        cookies_per_sec += autoclickers[i].owned * autoclickers[i].cps * autoclickers[i].bonus;
      }
      bonus = (int)(sqrt(cookies_per_sec) / 10) > 0 ? (int)(sqrt(cookies_per_sec) / 10) : 1;
      cookies += cookies_per_sec * bonus;
      last_time = current_time;
    }

    vsformat("Cookies: %llu \\n Per Second: %llu", (unsigned long long)cookies, (unsigned long long)cookies_per_sec);
    if (bonus > 1) {
      vsformat_concat("\\c0x3299a8 \\s16 *%d", bonus);
    }
    fenster_drawtext(&f, font, vsbuff, 10, 30);

    int button_w = 200, button_h = 80, start_y = 100;
    for (int i = 0; i < 9; i++) {
      int x = 10, y = start_y + i * (button_h + 10);
      int button_color = 0x404040;

      if (cookies >= autoclickers[i].cost && !autoclickers[i].unlocked) {
        autoclickers[i].unlocked = 1;
      }

      if (autoclickers[i].unlocked) {
        if (cookies >= autoclickers[i].cost) {
          if (fenster_point_in_rect(f.mpos[0], f.mpos[1], x, y, button_w, button_h)) {
            button_color = 0x517554;
            hovering_cursor = 2;
            if (f.mclick[0]) {
              cookies -= autoclickers[i].cost;
              autoclickers[i].owned++;
              autoclickers[i].bonus = (int)(sqrt(autoclickers[i].owned) / 2) > 0 ? (int)(sqrt(autoclickers[i].owned) / 2) : 1;
              autoclickers[i].cost = autoclickers[i].cost * 115 / 100;
            }
          } else {
            button_color = 0x475c49;
          }
        }
        fenster_drawrec(&f, x, y, button_w, button_h, button_color);

        vsformat("\\c0xA9A9A9 %s", autoclickers[i].name);
        if (autoclickers[i].bonus > 1) {
          vsformat_concat("\\c0x3299a8 *%d", autoclickers[i].bonus);
        }
        vsformat_concat("\\n \\c0xFFFFFF Cost: %llu\\n Owned: %llu", 
          (unsigned long long)autoclickers[i].cost, 
          (unsigned long long)autoclickers[i].owned);

        fenster_drawtext(&f, font, vsbuff, x + 10, y + 10);
      }
    }

    fenster_cursor(&f, hovering_cursor);
    fenster_sync(&f, 60);
    if (f.keysp[70] == 1) fenster_fullscreen(&f, fs ^= 1);
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
