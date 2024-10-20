#ifndef FENSTER_FONT_H
#define FENSTER_FONT_H

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "fenster/fenster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    stbtt_fontinfo info;
    unsigned char* buffer;
} FensterFont;

typedef struct {
    uint32_t color;
    float size;
    float curr_x;
    float curr_y;
    float scale;
    float spacing;
    float line_spacing;
} RenderState;

static const char* skip_space(const char* str);
static const char* read_number(const char* str, float* number);
static const char* read_color(const char* str, uint32_t* color);
static void render_char(struct fenster* f, FensterFont* font, char c, RenderState* state);
static const char* handle_command(const char* p, RenderState* state, float base_x, 
                                  stbtt_fontinfo* info, int ascent, int descent, int lineGap);

FensterFont* fenster_loadfont(const char* filename);
void fenster_drawtext(struct fenster* f, FensterFont* font, const char* text, int x, int y);
void fenster_freefont(FensterFont* font);

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

FensterFont* fenster_loadfont(const char* filename) {
    FILE* font_file = fopen(filename, "rb");
    if (!font_file) return NULL;

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

    fread(font->buffer, buffer_size, 1, font_file);
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

void fenster_freefont(FensterFont* font) {
    if (font) {
        free(font->buffer);
        free(font);
    }
}

#endif /* FENSTER_FONT_H */
