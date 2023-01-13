#ifndef FENSTER_AUDIO_WINDOWS_H
#define FENSTER_AUDIO_WINDOWS_H

#include <windows.h>
#include <mmsystem.h>

typedef struct {
  HWAVEOUT wo;
  WAVEHDR hdr[2];
  int16_t bufs[2][FENSTER_AUDIO_BUFSZ];
} WindowsAudioData;

int fenster_audio_open(struct fenster_audio *fa) {
  WindowsAudioData *wad = (WindowsAudioData *)malloc(sizeof(WindowsAudioData));
  if (!wad) return -1;
  fa->audio_data = wad;

  WAVEFORMATEX wfx = {WAVE_FORMAT_PCM, 1, FENSTER_SAMPLE_RATE, FENSTER_SAMPLE_RATE * 2, 2, 16, 0};
  waveOutOpen(&wad->wo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
  for (int i = 0; i < 2; i++) {
    wad->hdr[i].lpData = (LPSTR)wad->bufs[i];
    wad->hdr[i].dwBufferLength = FENSTER_AUDIO_BUFSZ * 2;
    waveOutPrepareHeader(wad->wo, &wad->hdr[i], sizeof(WAVEHDR));
    waveOutWrite(wad->wo, &wad->hdr[i], sizeof(WAVEHDR));
  }
  return 0;
}

int fenster_audio_available(struct fenster_audio *fa) {
  WindowsAudioData *wad = (WindowsAudioData *)fa->audio_data;
  for (int i = 0; i < 2; i++)
    if (wad->hdr[i].dwFlags & WHDR_DONE)
      return FENSTER_AUDIO_BUFSZ;
  return 0;
}

void fenster_audio_write(struct fenster_audio *fa, float *buf, size_t n) {
  WindowsAudioData *wad = (WindowsAudioData *)fa->audio_data;
  for (int i = 0; i < 2; i++) {
    if (wad->hdr[i].dwFlags & WHDR_DONE) {
      for (unsigned j = 0; j < n; j++) {
        wad->bufs[i][j] = (int16_t)(buf[j] * 32767);
      }
      waveOutWrite(wad->wo, &wad->hdr[i], sizeof(WAVEHDR));
      return;
    }
  }
}

void fenster_audio_close(struct fenster_audio *fa) {
  WindowsAudioData *wad = (WindowsAudioData *)fa->audio_data;
  waveOutClose(wad->wo);
  free(wad);
}

#endif /* FENSTER_AUDIO_WINDOWS_H */
