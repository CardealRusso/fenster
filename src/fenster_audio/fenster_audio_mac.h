#ifndef FENSTER_AUDIO_MAC_H
#define FENSTER_AUDIO_MAC_H

#include <AudioToolbox/AudioQueue.h>
#include <dispatch/dispatch.h>

typedef struct {
  AudioQueueRef queue;
  dispatch_semaphore_t drained;
  dispatch_semaphore_t full;
} MacAudioData;

static void fenster_audio_cb(void *p, AudioQueueRef q, AudioQueueBufferRef b) {
  struct fenster_audio *fa = (struct fenster_audio *)p;
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  dispatch_semaphore_wait(mad->full, DISPATCH_TIME_FOREVER);
  memmove(b->mAudioData, fa->buf, sizeof(fa->buf));
  dispatch_semaphore_signal(mad->drained);
  AudioQueueEnqueueBuffer(q, b, 0, NULL);
}

int fenster_audio_open(struct fenster_audio *fa) {
  MacAudioData *mad = (MacAudioData *)malloc(sizeof(MacAudioData));
  if (!mad) return -1;
  fa->audio_data = mad;

  AudioStreamBasicDescription format = {0};
  format.mSampleRate = FENSTER_SAMPLE_RATE;
  format.mFormatID = kAudioFormatLinearPCM;
  format.mFormatFlags = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsPacked;
  format.mBitsPerChannel = 32;
  format.mFramesPerPacket = format.mChannelsPerFrame = 1;
  format.mBytesPerPacket = format.mBytesPerFrame = 4;
  mad->drained = dispatch_semaphore_create(1);
  mad->full = dispatch_semaphore_create(0);
  AudioQueueNewOutput(&format, fenster_audio_cb, fa, NULL, NULL, 0, &mad->queue);
  for (int i = 0; i < 2; i++) {
    AudioQueueBufferRef buffer = NULL;
    AudioQueueAllocateBuffer(mad->queue, FENSTER_AUDIO_BUFSZ * 4, &buffer);
    buffer->mAudioDataByteSize = FENSTER_AUDIO_BUFSZ * 4;
    memset(buffer->mAudioData, 0, buffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(mad->queue, buffer, 0, NULL);
  }
  AudioQueueStart(mad->queue, NULL);
  return 0;
}

void fenster_audio_close(struct fenster_audio *fa) {
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  AudioQueueStop(mad->queue, false);
  AudioQueueDispose(mad->queue, false);
  free(mad);
}

int fenster_audio_available(struct fenster_audio *fa) {
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  if (dispatch_semaphore_wait(mad->drained, DISPATCH_TIME_NOW))
    return 0;
  return FENSTER_AUDIO_BUFSZ - fa->pos;
}

void fenster_audio_write(struct fenster_audio *fa, float *buf, size_t n) {
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  while (fa->pos < FENSTER_AUDIO_BUFSZ && n > 0) {
    fa->buf[fa->pos++] = *buf++, n--;
  }
  if (fa->pos >= FENSTER_AUDIO_BUFSZ) {
    fa->pos = 0;
    dispatch_semaphore_signal(mad->full);
  }
}

#endif /* FENSTER_AUDIO_MAC_H */
