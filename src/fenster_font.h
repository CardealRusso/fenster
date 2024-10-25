#ifndef FENSTER_FONT_H
#define FENSTER_FONT_H

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "fenster/fenster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(__linux__) || defined(__APPLE__)
#include <glob.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

typedef struct {
  stbtt_fontinfo info;
  unsigned char* buffer;
} FensterFont;

typedef struct {
  char **paths;
  size_t count;
} FensterFontList;

FensterFont* fenster_loadfont(const char* filename) {
  FILE* f = fopen(filename, "rb");
  if (!f) return NULL;
  
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  FensterFont* font = malloc(sizeof(FensterFont));
  if (!font || !(font->buffer = malloc(size)) || 
      fread(font->buffer, size, 1, f) != 1 ||
      !stbtt_InitFont(&font->info, font->buffer, 0)) {
    if (font) {
      if (font->buffer) free(font->buffer);
      free(font);
    }
    fclose(f);
    return NULL;
  }
  fclose(f);
  return font;
}

void fenster_freefont(FensterFont* font) {
  if (font) { free(font->buffer); free(font); }
}

static void add_font_path(FensterFontList *fonts, const char *path) {
  char **new_paths = realloc(fonts->paths, (fonts->count + 1) * sizeof(char*));
  if (new_paths && (new_paths[fonts->count] = strdup(path))) {
    fonts->paths = new_paths;
    fonts->count++;
  }
}

FensterFontList fenster_loadfontlist(void) {
  FensterFontList fonts = {0};
  
  #if defined(__linux__) || defined(__APPLE__)
  const char *patterns[] = {
    "~/.local/share/fonts/**/*.ttf", "~/.fonts/**/*.ttf",
    "/usr/*/fonts/**/*.ttf", "/usr/*/*/fonts/**/*.ttf",
    "/usr/*/*/*/fonts/**/*.ttf", "/usr/*/*/*/*/fonts/**/*.ttf",
    "/Library/Fonts/**/*.ttf", "/System/Library/Fonts/**/*.ttf"
  };
  glob_t glob_res;
  for (size_t i = 0; i < sizeof(patterns)/sizeof(*patterns); i++) {
    if (glob(patterns[i], GLOB_TILDE | GLOB_NOSORT, NULL, &glob_res) == 0) {
      for (size_t j = 0; j < glob_res.gl_pathc; j++)
        add_font_path(&fonts, glob_res.gl_pathv[j]);
      globfree(&glob_res);
    }
  }
  #elif defined(_WIN32)
  char path[MAX_PATH];
  if (GetEnvironmentVariable("SYSTEMROOT", path, MAX_PATH)) {
    strcat(path, "\\Fonts\\*.ttf");
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(path, &fd);
    if (h != INVALID_HANDLE_VALUE) {
      char full[MAX_PATH];
      do {
        sprintf(full, "%s\\Fonts\\%s", path, fd.cFileName);
        add_font_path(&fonts, full);
      } while (FindNextFile(h, &fd));
      FindClose(h);
    }
  }
  #endif
  return fonts;
}

int fenster_findfontinlist(FensterFontList *fonts, const char *term) {
  if (!fonts || !term) return -1;
  
  char *term_lower = strdup(term);
  if (!term_lower) return -1;
  
  for (size_t i = 0; i < fonts->count; i++) {
    char *path = strdup(fonts->paths[i]);
    if (path) {
      for (char *p = term_lower; *p; p++) *p = tolower(*p);
      for (char *p = path; *p; p++) *p = tolower(*p);
      if (strstr(path, term_lower)) {
        free(path);
        free(term_lower);
        return i;
      }
      free(path);
    }
  }
  free(term_lower);
  return -1;
}

void fenster_freefontlist(FensterFontList *fonts) {
  if (fonts) {
    for (size_t i = 0; i < fonts->count; i++) free(fonts->paths[i]);
    free(fonts->paths);
    *fonts = (FensterFontList){0};
  }
}

void fenster_drawtext(struct fenster* f, FensterFont* font, const char* text, int x, int y) {
  if (!font || !text) return;
  
  struct {
    uint32_t color;
    float size, scale, spacing, line_spacing;
    int x, y;
  } state = {0xFFFFFF, 20.0f, 
             stbtt_ScaleForPixelHeight(&font->info, 20.0f), 
             1.0f, 1.0f, x, y};
  
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);
  state.y += ascent * state.scale;
  
  unsigned char *bitmap = NULL;
  size_t bitmap_size = 0;
  
  for (const char *p = text; *p; p++) {
    if (*p == '\\' && *(p + 1)) {
      p++;
      char cmd = *p++;
      float num;
      switch (cmd) {
        case 'c':
          sscanf(p, "%x", &state.color);
          while (*p && *p != ' ') p++;
          if (*p == ' ') p++;
          break;
        case 's':
          sscanf(p, "%f", &num);
          state.size = num;
          state.scale = stbtt_ScaleForPixelHeight(&font->info, num);
          while (*p && *p != ' ') p++;
          if (*p == ' ') p++;
          break;
        case 'p':
          sscanf(p, "%f", &state.spacing);
          while (*p && *p != ' ') p++;
          if (*p == ' ') p++;
          break;
        case 'h':
          sscanf(p, "%f", &state.line_spacing);
          while (*p && *p != ' ') p++;
          if (*p == ' ') p++;
          break;
        case 'n':
          state.y += (ascent - descent + lineGap) * state.scale * state.line_spacing;
          state.x = x;
          if (*p == ' ') p++;
          break;
      }
      p--;
      continue;
    }
    
    int advance, lsb, x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(&font->info, *p, &advance, &lsb);
    stbtt_GetCodepointBitmapBox(&font->info, *p, state.scale, state.scale, 
                               &x0, &y0, &x1, &y1);
    
    size_t new_size = (x1 - x0) * (y1 - y0);
    if (new_size > bitmap_size) {
      bitmap = realloc(bitmap, new_size);
      bitmap_size = new_size;
    }
    
    if (bitmap) {
      stbtt_MakeCodepointBitmap(&font->info, bitmap, x1 - x0, y1 - y0,
                               x1 - x0, state.scale, state.scale, *p);
      
      for (int i = 0; i < y1 - y0; i++) {
        for (int j = 0; j < x1 - x0; j++) {
          int px = state.x + x0 + j;
          int py = state.y + y0 + i;
          if (px >= 0 && px < f->width && py >= 0 && py < f->height && 
              bitmap[i * (x1 - x0) + j] > 127) {
            fenster_pixel(f, px, py) = state.color;
          }
        }
      }
      
      state.x += (advance * state.scale) + state.spacing;
    }
  }
  
  free(bitmap);
}

#endif /* FENSTER_FONT_H */
