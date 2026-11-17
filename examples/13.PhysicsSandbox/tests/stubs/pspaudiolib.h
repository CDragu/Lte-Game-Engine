#ifndef TEST_PSPAUDIO_LIB_H
#define TEST_PSPAUDIO_LIB_H
#define PSP_VOLUME_MAX 32767
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*pspAudioCallback_t)(void*,unsigned int,void*);
void pspAudioSetChannelCallback(int channel,pspAudioCallback_t callback,void* user);
void pspAudioSetVolume(int channel,int left,int right);
#ifdef __cplusplus
}
#endif
#endif
