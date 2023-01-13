#ifndef FENSTER_AUDIO_LINUX_H
#define FENSTER_AUDIO_LINUX_H

int snd_pcm_open(void **, const char *, int, int);
int snd_pcm_set_params(void *, int, int, int, int, int, int);
int snd_pcm_avail(void *);
int snd_pcm_writei(void *, const void *, unsigned long);
int snd_pcm_recover(void *, int, int);
int snd_pcm_close(void *);

typedef struct {
  void *pcm;
} LinuxAudioData;

int fenster_audio_open(struct fenster_audio *fa) {
  LinuxAudioData *lad = (LinuxAudioData *)malloc(sizeof(LinuxAudioData));
  if (!lad) return -1;
  fa->audio_data = lad;

  if (snd_pcm_open(&lad->pcm, "default", 0, 0))
    return -1;
  int fmt = (*(unsigned char *)(&(uint16_t){1})) ? 14 : 15;
  return snd_pcm_set_params(lad->pcm, fmt, 3, 1, FENSTER_SAMPLE_RATE, 1, 100000);
}

int fenster_audio_available(struct fenster_audio *fa) {
  LinuxAudioData *lad = (LinuxAudioData *)fa->audio_data;
  int n = snd_pcm_avail(lad->pcm);
  if (n < 0)
    snd_pcm_recover(lad->pcm, n, 0);
  return n;
}

void fenster_audio_write(struct fenster_audio *fa, float *buf, size_t n) {
  LinuxAudioData *lad = (LinuxAudioData *)fa->audio_data;
  int r = snd_pcm_writei(lad->pcm, buf, n);
  if (r < 0)
    snd_pcm_recover(lad->pcm, r, 0);
}

void fenster_audio_close(struct fenster_audio *fa) {
  LinuxAudioData *lad = (LinuxAudioData *)fa->audio_data;
  snd_pcm_close(lad->pcm);
  free(lad);
}

#endif /* FENSTER_AUDIO_LINUX_H */
