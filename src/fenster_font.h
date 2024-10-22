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

typedef struct {
    uint32_t color;
    float size;
    float curr_x;
    float curr_y;
    float scale;
    float spacing;
    float line_spacing;
} RenderState;

FensterFont* fenster_loadfont(const char* filename);
void fenster_freefont(FensterFont* font);
FensterFontList fenster_loadfontlist(void);
void fenster_freefontlist(FensterFontList *fonts);
int fenster_findfontinlist(FensterFontList *fonts, const char *term);

void fenster_drawtext(struct fenster* f, FensterFont* font, const char* text, int x, int y);

static void to_lowercase(char *str);
static void add_font(FensterFontList *fonts, const char *font);
static const char* skip_space(const char* str);
static const char* read_number(const char* str, float* number);
static const char* read_color(const char* str, uint32_t* color);
static void render_char(struct fenster* f, FensterFont* font, char c, RenderState* state);
static const char* handle_command(const char* p, RenderState* state, float base_x, 
                                stbtt_fontinfo* info, int ascent, int descent, int lineGap);

#if defined(__linux__) || defined(__APPLE__)
static void get_fonts_posix(FensterFontList *fonts);
#elif defined(_WIN32)
static void get_fonts_windows(FensterFontList *fonts);
#endif

/* String manipulation functions */
static void to_lowercase(char *str) {
    for (size_t i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

static const char* skip_space(const char* str) {
    if (*str == ' ') str++;
    return str;
}

static const char* read_number(const char* str, float* number) {
    char buffer[32] = {0};
    int i = 0;
    while (*str && *str != ' ' && i < 31) {
        buffer[i++] = *str++;
    }
    *number = atof(buffer);
    return str;
}

static const char* read_color(const char* str, uint32_t* color) {
    char buffer[32] = {0};
    int i = 0;
    while (*str && *str != ' ' && i < 31) {
        buffer[i++] = *str++;
    }
    if (strncmp(buffer, "0x", 2) == 0) {
        sscanf(buffer, "%x", color);
    } else {
        *color = (uint32_t)atoi(buffer);
    }
    return str;
}

/* Font list management functions */
static void add_font(FensterFontList *fonts, const char *font) {
    fonts->paths = realloc(fonts->paths, (fonts->count + 1) * sizeof(char *));
    if (fonts->paths) {
        fonts->paths[fonts->count] = strdup(font);
        fonts->count++;
    }
}

#if defined(__linux__) || defined(__APPLE__)
static void get_fonts_posix(FensterFontList *fonts) {
    const char *searchPatterns[] = {
        "~/.local/share/fonts/**/*.ttf",
        "~/.fonts/**/*.ttf",
        "/usr/*/fonts/**/*.ttf",
        "/usr/*/*/fonts/**/*.ttf",
        "/usr/*/*/*/fonts/**/*.ttf",
        "/usr/*/*/*/*/fonts/**/*.ttf",
        "/Library/Fonts/**/*.ttf",
        "/System/Library/Fonts/**/*.ttf"
    };
    
    glob_t results;
    for (size_t i = 0; i < sizeof(searchPatterns) / sizeof(searchPatterns[0]); i++) {
        if (glob(searchPatterns[i], GLOB_TILDE | GLOB_NOSORT, NULL, &results) == 0) {
            for (size_t j = 0; j < results.gl_pathc; j++) {
                add_font(fonts, results.gl_pathv[j]);
            }
            globfree(&results);
        }
    }
}
#elif defined(_WIN32)
static void get_fonts_windows(FensterFontList *fonts) {
    char systemRoot[MAX_PATH];
    char fontPath[MAX_PATH];
    
    if (GetEnvironmentVariable("SYSTEMROOT", systemRoot, sizeof(systemRoot)) == 0) {
        return;
    }
    
    snprintf(fontPath, sizeof(fontPath), "%s\\Fonts\\*.ttf", systemRoot);
    
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fontPath, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            char fullPath[MAX_PATH];
            snprintf(fullPath, sizeof(fullPath), "%s\\Fonts\\%s", 
                    systemRoot, findFileData.cFileName);
            add_font(fonts, fullPath);
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}
#endif

/* Text rendering functions */
static void render_char(struct fenster* f, FensterFont* font, char c, RenderState* state) {
    int advance, lsb, x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(&font->info, c, &advance, &lsb);
    stbtt_GetCodepointBitmapBox(&font->info, c, state->scale, state->scale, 
                               &x0, &y0, &x1, &y1);

    unsigned char* bitmap = malloc((x1 - x0) * (y1 - y0));
    stbtt_MakeCodepointBitmap(&font->info, bitmap, x1 - x0, y1 - y0,
                             x1 - x0, state->scale, state->scale, c);

    int bitmap_x = state->curr_x + x0;
    int bitmap_y = state->curr_y + y0;

    for (int i = 0; i < y1 - y0; i++) {
        for (int j = 0; j < x1 - x0; j++) {
            int px = bitmap_x + j;
            int py = bitmap_y + i;
            
            if (px >= 0 && px < f->width && py >= 0 && py < f->height) {
                unsigned char alpha = bitmap[i * (x1 - x0) + j];
                if (alpha > 127) {
                    fenster_pixel(f, px, py) = state->color;
                }
            }
        }
    }

    free(bitmap);
    state->curr_x += (advance * state->scale) + state->spacing;
}

static const char* handle_command(const char* p, RenderState* state, float base_x, 
                                stbtt_fontinfo* info, int ascent, int descent, int lineGap) {
    switch (*p) {
        case 'c':
            p++;
            p = skip_space(read_color(p, &state->color));
            break;
        case 's':
            p++;
            float new_size;
            p = skip_space(read_number(p, &new_size));
            state->size = new_size;
            state->scale = stbtt_ScaleForPixelHeight(info, new_size);
            break;
        case 'p':
            p++;
            p = skip_space(read_number(p, &state->spacing));
            break;
        case 'h':
            p++;
            p = skip_space(read_number(p, &state->line_spacing));
            break;
        case 'n':
            p++;
            float line_height = (ascent - descent + lineGap) * state->scale * state->line_spacing;
            state->curr_y += line_height;
            state->curr_x = base_x;
            p = skip_space(p);
            break;
    }
    return p;
}

/* Public API Implementation */
FensterFont* fenster_loadfont(const char* filename) {
    FILE* font_file = fopen(filename, "rb");
    if (!font_file) {
        return NULL;
    }

    FensterFont* font = malloc(sizeof(FensterFont));
    if (!font) {
        fclose(font_file);
        return NULL;
    }

    fseek(font_file, 0, SEEK_END);
    long buffer_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    font->buffer = malloc(buffer_size);
    if (!font->buffer) {
        free(font);
        fclose(font_file);
        return NULL;
    }

    if (fread(font->buffer, buffer_size, 1, font_file) != 1) {
        free(font->buffer);
        free(font);
        fclose(font_file);
        return NULL;
    }

    fclose(font_file);

    if (!stbtt_InitFont(&font->info, font->buffer, 0)) {
        free(font->buffer);
        free(font);
        return NULL;
    }

    return font;
}

void fenster_drawtext(struct fenster* f, FensterFont* font, const char* text, int x, int y) {
    if (!font || !text) return;

    RenderState state = {
        .color = 0xFFFFFF,
        .size = 20.0f,
        .curr_x = x,
        .curr_y = y,
        .scale = stbtt_ScaleForPixelHeight(&font->info, 20.0f),
        .spacing = 1.0f,
        .line_spacing = 1.0f
    };

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);
    state.curr_y += ascent * state.scale;

    const char* p = text;
    while (*p) {
        if (*p == '\\') {
            p++;
            p = handle_command(p, &state, x, &font->info, ascent, descent, lineGap);
            continue;
        }
        render_char(f, font, *p, &state);
        p++;
    }
}

FensterFontList fenster_loadfontlist(void) {
    FensterFontList fonts = {NULL, 0};
    
    #if defined(__linux__) || defined(__APPLE__)
    get_fonts_posix(&fonts);
    #elif defined(_WIN32)
    get_fonts_windows(&fonts);
    #endif

    return fonts;
}

int fenster_findfontinlist(FensterFontList *fonts, const char *term) {
    if (!fonts || !term) {
        return -1;
    }

    char *term_lower = strdup(term);
    if (!term_lower) {
        return -1;
    }
    
    to_lowercase(term_lower);
    int found_index = -1;

    for (size_t i = 0; i < fonts->count && found_index == -1; i++) {
        char *path_lower = strdup(fonts->paths[i]);
        if (path_lower) {
            to_lowercase(path_lower);
            if (strstr(path_lower, term_lower)) {
                found_index = i;
            }
            free(path_lower);
        }
    }

    free(term_lower);
    return found_index;
}

void fenster_freefontlist(FensterFontList *fonts) {
    if (!fonts) {
        return;
    }
    
    for (size_t i = 0; i < fonts->count; i++) {
        free(fonts->paths[i]);
    }
    
    free(fonts->paths);
    fonts->paths = NULL;
    fonts->count = 0;
}

void fenster_freefont(FensterFont* font) {
    if (font) {
        free(font->buffer);
        free(font);
    }
}

#endif /* FENSTER_FONT_H */
