#ifndef FENSTER_ADDONS_H
#define FENSTER_ADDONS_H

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    #include <immintrin.h>
    #define USE_AVX2
#elif defined(__arm64__) || defined(__aarch64__)
    #include <arm_neon.h>
    #define USE_NEON
#endif

// Fast buffer fill function optimized for different architectures
static inline void fenster_fill(struct fenster *f, uint32_t color) {
    if (!f || !f->buf) return;
    
    const size_t total_pixels = (size_t)f->width * f->height;
    
    // For zero, use memset which is highly optimized on all platforms
    if (color == 0) {
        memset(f->buf, 0, total_pixels * sizeof(uint32_t));
        return;
    }

#if defined(USE_AVX2)
    // AVX2 implementation for x86/x64
    __m256i color_vec = _mm256_set1_epi32(color);
    size_t vec_count = total_pixels / 8;
    size_t remainder = total_pixels % 8;
    
    __m256i *buf_vec = (__m256i*)f->buf;
    for (size_t i = 0; i < vec_count; i++) {
        _mm256_storeu_si256(&buf_vec[i], color_vec);
    }
    
    if (remainder) {
        uint32_t *remaining = (uint32_t*)&f->buf[vec_count * 8];
        for (size_t i = 0; i < remainder; i++) {
            remaining[i] = color;
        }
    }

#elif defined(USE_NEON)
    // NEON implementation for ARM
    uint32x4_t color_vec = vdupq_n_u32(color);
    size_t vec_count = total_pixels / 4;
    size_t remainder = total_pixels % 4;
    
    uint32_t *buf = f->buf;
    for (size_t i = 0; i < vec_count; i++) {
        vst1q_u32(buf + i * 4, color_vec);
    }
    
    if (remainder) {
        for (size_t i = vec_count * 4; i < total_pixels; i++) {
            buf[i] = color;
        }
    }

#else
    // Fallback implementation for other architectures
    for (size_t i = 0; i < total_pixels; i++) {
        f->buf[i] = color;
    }
#endif
}

#endif /* FENSTER_ADDONS_H */
