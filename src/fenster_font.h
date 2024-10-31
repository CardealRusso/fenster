#ifndef FENSTER_FONT_H
#define FENSTER_FONT_H

#define STB_TRUETYPE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"
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

#define MAX_FONT_SIZES 255
#define MAX_CHARS 255

typedef struct {
  unsigned char* bitmap;
  int width;
  int height;
  int x0;
  int y0;
  int advance;
} CachedGlyph;

typedef struct {
  CachedGlyph*** cache;
  float* sizes;
  int num_sizes;
} GlyphCache;

typedef struct {
  stbtt_fontinfo info;
  unsigned char* buffer;
  GlyphCache cache;
} FensterFont;

typedef struct {
  char **paths;
  size_t count;
} FensterFontList;

static void init_cache(GlyphCache* cache) {
  cache->cache = calloc(MAX_FONT_SIZES, sizeof(CachedGlyph**));
  cache->sizes = calloc(MAX_FONT_SIZES, sizeof(float));
  cache->num_sizes = 0;
}

static int get_size_index(GlyphCache* cache, float size) {
  for (int i = 0; i < cache->num_sizes; i++) {
    if (cache->sizes[i] == size) return i;
  }
  
  if (cache->num_sizes < MAX_FONT_SIZES) {
    int idx = cache->num_sizes;
    cache->sizes[idx] = size;
    cache->cache[idx] = calloc(MAX_CHARS, sizeof(CachedGlyph*));
    cache->num_sizes++;
    return idx;
  }
  
  cache->sizes[0] = size;
  for (int i = 0; i < MAX_CHARS; i++) {
    if (cache->cache[0][i]) {
      free(cache->cache[0][i]->bitmap);
      free(cache->cache[0][i]);
      cache->cache[0][i] = NULL;
    }
  }
  return 0;
}

static void free_cache(GlyphCache* cache) {
  if (!cache->cache) return;
  
  for (int i = 0; i < MAX_FONT_SIZES; i++) {
    if (cache->cache[i]) {
      for (int j = 0; j < MAX_CHARS; j++) {
        if (cache->cache[i][j]) {
          free(cache->cache[i][j]->bitmap);
          free(cache->cache[i][j]);
        }
      }
      free(cache->cache[i]);
    }
  }
  free(cache->cache);
  free(cache->sizes);
}

static void add_font_path(FensterFontList *fonts, const char *path) {
  char **new_paths = realloc(fonts->paths, (fonts->count + 1) * sizeof(char*));
  if (new_paths && (new_paths[fonts->count] = strdup(path))) {
    fonts->paths = new_paths;
    fonts->count++;
  }
}

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
  
  init_cache(&font->cache);
  fclose(f);
  return font;
}

void fenster_freefont(FensterFont* font) {
  if (font) {
    free_cache(&font->cache);
    free(font->buffer);
    free(font);
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
  char base_path[MAX_PATH];
  if (GetEnvironmentVariable("SYSTEMROOT", base_path, MAX_PATH)) {
    strcat(base_path, "\\Fonts");
    char search_path[MAX_PATH];
    sprintf(search_path, "%s\\*.ttf", base_path);
    
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(search_path, &fd);
    if (h != INVALID_HANDLE_VALUE) {
      char full[MAX_PATH];
      do {
        sprintf(full, "%s\\%s", base_path, fd.cFileName);
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
    uint32_t color, background;
    float size, scale, spacing, line_spacing;
    int x, y;
  } state = {0xFFFFFF, 0xFFFFFFFF, 20.0f, 
             stbtt_ScaleForPixelHeight(&font->info, 20.0f), 
             1.0f, 1.0f, x, y};
  
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);
  state.y += ascent * state.scale;
  
  unsigned char *temp_bitmap = NULL;
  size_t temp_bitmap_size = 0;
  
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
        case 'b':
          sscanf(p, "%x", &state.background);
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
    
    const unsigned char c = *p;
    if (c >= MAX_CHARS) continue;
    if (c == ' ') {
      state.x += state.size * state.spacing;
      continue;
    }
    
    int size_idx = get_size_index(&font->cache, state.size);
    CachedGlyph* glyph = font->cache.cache[size_idx][(int)c];
    
    if (!glyph) {
      int advance, lsb, x0, y0, x1, y1;
      stbtt_GetCodepointHMetrics(&font->info, c, &advance, &lsb);
      stbtt_GetCodepointBitmapBox(&font->info, c, state.scale, state.scale, 
                                 &x0, &y0, &x1, &y1);
      
      int width = x1 - x0;
      int height = y1 - y0;
      
      if (width <= 0 || height <= 0) continue;
      
      size_t new_size = width * height;
      if (new_size > temp_bitmap_size) {
        unsigned char* new_bitmap = realloc(temp_bitmap, new_size);
        if (!new_bitmap) continue;
        temp_bitmap = new_bitmap;
        temp_bitmap_size = new_size;
      }
      
      stbtt_MakeCodepointBitmap(&font->info, temp_bitmap, width, height,
                               width, state.scale, state.scale, c);
      
      glyph = malloc(sizeof(CachedGlyph));
      glyph->bitmap = malloc(width * height);
      memcpy(glyph->bitmap, temp_bitmap, width * height);
      glyph->width = width;
      glyph->height = height;
      glyph->x0 = x0;
      glyph->y0 = y0;
      glyph->advance = advance;
      
      font->cache.cache[size_idx][(int)c] = glyph;
    }
    
    for (int i = 0; i < glyph->height; i++) {
      for (int j = 0; j < glyph->width; j++) {
        int px = state.x + glyph->x0 + j;
        int py = state.y + glyph->y0 + i;
        if (px >= 0 && px < f->width && py >= 0 && py < f->height) {
          if (glyph->bitmap[i * glyph->width + j] > 127) {
            fenster_pixel(f, px, py) = state.color;
          } else {
            if (state.background != 0xFFFFFFFF) {
              fenster_pixel(f, px, py) = state.background;
            }
          }
        }
      }
    }
    
    state.x += (glyph->advance * state.scale) + state.spacing;
  }
  
  free(temp_bitmap);
}

#endif /* FENSTER_FONT_H */
