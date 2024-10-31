#ifndef FENSTER_ADDONS_H
#define FENSTER_ADDONS_H

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
  #include <immintrin.h>
  #define USE_AVX2
  #define VECTOR_SIZE 8
#elif defined(__arm64__) || defined(__aarch64__)
  #include <arm_neon.h>
  #define USE_NEON
  #define VECTOR_SIZE 4
#endif

#define PI 3.14159265358979323846

#define VSBUFF_SIZE (16 * 1024)

char vsbuff[VSBUFF_SIZE];

void vsformat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(vsbuff, VSBUFF_SIZE, fmt, args);
  va_end(args);
}

void vsformat_concat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  size_t len = strlen(vsbuff);
  if (len < VSBUFF_SIZE - 1) {
    vsnprintf(vsbuff + len, VSBUFF_SIZE - len, fmt, args);
  }
  va_end(args);
}

static inline void vector_fill(uint32_t* buf, size_t count, uint32_t color) {
  if (!buf || count == 0) return;
  
  size_t vec_count = count / VECTOR_SIZE;
  size_t remainder = count % VECTOR_SIZE;
  
#if defined(USE_AVX2)
  __m256i color_vec = _mm256_set1_epi32(color);
  __m256i *buf_vec = (__m256i*)buf;
  for (size_t i = 0; i < vec_count; i++) {
    _mm256_storeu_si256(&buf_vec[i], color_vec);
  }
#elif defined(USE_NEON)
  uint32x4_t color_vec = vdupq_n_u32(color);
  for (size_t i = 0; i < vec_count; i++) {
    vst1q_u32(buf + i * 4, color_vec);
  }
#endif

  uint32_t *remainder_ptr = buf + (vec_count * VECTOR_SIZE);
  for (size_t i = 0; i < remainder; i++) {
    remainder_ptr[i] = color;
  }
}

static inline void fenster_setpixel(struct fenster *f, int x, int y, uint32_t color) {
  if (x >= 0 && x < f->width && y >= 0 && y < f->height) {
    f->buf[y * f->width + x] = color;
  }
}

static inline void fenster_drawhline(struct fenster *f, int x1, int x2, int y, uint32_t color) {
  if (y < 0 || y >= f->height || x1 >= f->width || x2 < 0) return;
  
  if (x1 < 0) x1 = 0;
  if (x2 >= f->width) x2 = f->width - 1;
  if (x1 > x2) return;
  
  size_t offset = y * f->width + x1;
  size_t length = x2 - x1 + 1;
  vector_fill(&f->buf[offset], length, color);
}

void fenster_drawline(struct fenster *f, int x1, int y1, int x2, int y2, uint32_t color) {
  int dx = abs(x2 - x1), dy = abs(y2 - y1);
  int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2;
  
  while (1) {
    fenster_setpixel(f, x1, y1, color);
    if (x1 == x2 && y1 == y2) break;
    
    int e2 = err;
    if (e2 > -dx) { err -= dy; x1 += sx; }
    if (e2 < dy) { err += dx; y1 += sy; }
  }
}

void fenster_drawcircline(struct fenster *f, int x0, int y0, int radius, uint32_t color) {
  int x = radius, y = 0, err = 0;
  
  while (x >= y) {
    fenster_setpixel(f, x0 + x, y0 + y, color);
    fenster_setpixel(f, x0 + y, y0 + x, color);
    fenster_setpixel(f, x0 - y, y0 + x, color);
    fenster_setpixel(f, x0 - x, y0 + y, color);
    fenster_setpixel(f, x0 - x, y0 - y, color);
    fenster_setpixel(f, x0 - y, y0 - x, color);
    fenster_setpixel(f, x0 + y, y0 - x, color);
    fenster_setpixel(f, x0 + x, y0 - y, color);

    if (err <= 0) { y += 1; err += 2*y + 1; }
    if (err > 0) { x -= 1; err -= 2*x + 1; }
  }
}

void fenster_drawcirc(struct fenster *f, int x0, int y0, int radius, uint32_t color) {
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x*x + y*y <= radius*radius) {
        fenster_setpixel(f, x0 + x, y0 + y, color);
      }
    }
  }
}

void fenster_drawrecline(struct fenster *f, int x, int y, int width, int height, uint32_t color) {
  int x2 = x + width - 1, y2 = y + height - 1;
  fenster_drawline(f, x, y, x2, y, color);
  fenster_drawline(f, x2, y, x2, y2, color);
  fenster_drawline(f, x2, y2, x, y2, color);
  fenster_drawline(f, x, y2, x, y, color);
}

void fenster_drawtri(struct fenster *f, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
  if (y1 > y2) { int tx = x1, ty = y1; x1 = x2; y1 = y2; x2 = tx; y2 = ty; }
  if (y1 > y3) { int tx = x1, ty = y1; x1 = x3; y1 = y3; x3 = tx; y3 = ty; }
  if (y2 > y3) { int tx = x2, ty = y2; x2 = x3; y2 = y3; x3 = tx; y3 = ty; }

  if (y1 >= f->height || y3 < 0) return;

  int start_y = fmax(0, y1);
  int end_y = fmin(f->height - 1, y3);

  for (int y = start_y; y <= end_y; y++) {
    int second_half = (y > y2 || y2 == y1);
    int segment_height = second_half ? y3 - y2 : y2 - y1;
    if (segment_height == 0) continue;

    float alpha = (float)(y - y1) / (y3 - y1);
    float beta = (float)(y - (second_half ? y2 : y1)) / segment_height;

    int xa = x1 + (x3 - x1) * alpha;
    int xb = second_half ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;

    if (xa > xb) { int t = xa; xa = xb; xb = t; }
    fenster_drawhline(f, xa, xb, y, color);
  }
}

void fenster_drawrec(struct fenster *f, int x, int y, int width, int height, uint32_t color) {
  if (x >= f->width || y >= f->height || x + width <= 0 || y + height <= 0) return;
  
  int start_y = fmax(0, y);
  int end_y = fmin(f->height - 1, y + height - 1);
  int start_x = fmax(0, x);
  int end_x = fmin(f->width - 1, x + width - 1);
  
  for (int iy = start_y; iy <= end_y; iy++) {
    fenster_drawhline(f, start_x, end_x, iy, color);
  }
}

void fenster_drawtriline(struct fenster *f, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
  fenster_drawline(f, x1, y1, x2, y2, color);
  fenster_drawline(f, x2, y2, x3, y3, color);
  fenster_drawline(f, x3, y3, x1, y1, color);
}

void fenster_drawpolyline(struct fenster *f, int cx, int cy, int sides, int radius, float rotation, uint32_t color) {
  if (sides < 3) return;
  
  float angle_step = 2 * PI / sides;
  int prev_x = cx + radius * cos(rotation);
  int prev_y = cy + radius * sin(rotation);
  
  for (int i = 1; i <= sides; i++) {
    float angle = rotation + i * angle_step;
    int x = cx + radius * cos(angle);
    int y = cy + radius * sin(angle);
    fenster_drawline(f, prev_x, prev_y, x, y, color);
    prev_x = x;
    prev_y = y;
  }
}

void fenster_drawpoly(struct fenster *f, int cx, int cy, int sides, int radius, float rotation, uint32_t color) {
  if (sides < 3) return;
  
  float angle_step = 2 * PI / sides;
  int *points_x = (int*)malloc(sides * sizeof(int));
  int *points_y = (int*)malloc(sides * sizeof(int));
  
  for (int i = 0; i < sides; i++) {
    float angle = rotation + i * angle_step;
    points_x[i] = cx + radius * cos(angle);
    points_y[i] = cy + radius * sin(angle);
  }
  
  for (int i = 1; i < sides - 1; i++) {
    fenster_drawtri(f, points_x[0], points_y[0], 
                    points_x[i], points_y[i],
                    points_x[i + 1], points_y[i + 1], color);
  }
  
  free(points_x);
  free(points_y);
}

static inline void fenster_fill(struct fenster *f, uint32_t color) {
  const size_t total_pixels = (size_t)f->width * f->height;
  if (color == 0) {
    memset(f->buf, 0, total_pixels * sizeof(uint32_t));
    return;
  }
  
  vector_fill(f->buf, total_pixels, color);
}

static inline int fenster_point_in_circle(int x, int y, int cx, int cy, int radius) {
  int dx = x - cx;
  int dy = y - cy;
  return dx * dx + dy * dy <= radius * radius;
}

static inline int fenster_point_in_rect(int x, int y, int rect_x, int rect_y, int rect_width, int rect_height) {
  return x >= rect_x && x <= rect_x + rect_width &&
    y >= rect_y && y <= rect_y + rect_height;
}

#endif /* FENSTER_ADDONS_H */
