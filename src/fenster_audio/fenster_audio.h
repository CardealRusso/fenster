#ifndef FENSTER_AUDIO_H
#define FENSTER_AUDIO_H

#ifndef FENSTER_SAMPLE_RATE
#define FENSTER_SAMPLE_RATE 44100
#endif

#ifndef FENSTER_AUDIO_BUFSZ
#define FENSTER_AUDIO_BUFSZ 8192
#endif

struct fenster_audio {
  void *audio_data;
  float buf[FENSTER_AUDIO_BUFSZ];
  size_t pos;
};

#ifndef FENSTER_API
#define FENSTER_API extern
#endif

FENSTER_API int fenster_audio_open(struct fenster_audio *f);
FENSTER_API int fenster_audio_available(struct fenster_audio *f);
FENSTER_API void fenster_audio_write(struct fenster_audio *f, float *buf, size_t n);
FENSTER_API void fenster_audio_close(struct fenster_audio *f);

#if defined(__APPLE__)
#include "fenster_audio_mac.h"
#elif defined(_WIN32)
#include "fenster_audio_windows.h"
#elif defined(__linux__)
#include "fenster_audio_linux.h"
#endif

#endif /* FENSTER_AUDIO_H */
