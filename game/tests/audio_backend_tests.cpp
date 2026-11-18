#include "../GameSounds.h"
#include <pspaudiolib.h>
#include <stdio.h>
#include <stdlib.h>

static int errors=0,channelRequests=0,releases=0,stops=0,volume=0;
static pspAudioCallback_t callback=0;
static void* callbackUser=0;
static void check(bool okay,const char* what) {
    printf("%s %s\n",okay?"PASS":"FAIL",what);
    if(!okay) errors++;
}

extern "C" int pspAudioChannelSearchFree() { ++channelRequests; return 2; }
extern "C" int pspAudioReleaseChannel(int c) { if(c==2) ++releases; return 1; }
extern "C" void pspAudioStopThisThread(int c) { if(c==2) ++stops; }
extern "C" void pspAudioSetVolume(int c,int l,int r) {
    if(c==2 && l>0 && l==r) volume=l;
}
extern "C" void pspAudioSetChannelCallback(int c,pspAudioCallback_t cb,void* user) {
    if(c==2) {callback=cb;callbackUser=user;}
}
extern "C" int sceKernelDelayThread(unsigned int) { return 0; }

int main()
{
    static tower::GameSounds audio; // like PSP: wavebank lives in static memory
    check(!audio.available(),"audio backend starts inactive");
    check(audio.initialize(),"audio acquires a free LTE mixer slot");
    check(audio.initialize() && channelRequests==1,"initialization is idempotent");
    check(audio.available() && callback!=0 && volume>0,
          "audio registers callback with audible volume");
    check(callbackUser==&audio,"audio callback receives its owning instance");
    const unsigned int frames=1024;
    short stereo[frames*2];
    for(unsigned int i=0;i<frames*2;i++) stereo[i]=42;
    callback(stereo,frames,callbackUser);
    bool silent=true;
    for(unsigned int i=0;i<frames*2;i++) if(stereo[i]) silent=false;
    check(silent,"idle callback writes silence");
    for(unsigned int i=0;i<frames*2;i++) stereo[i]=42;
    callback(stereo,frames,0);
    silent=true;
    for(unsigned int i=0;i<frames*2;i++) if(stereo[i]) silent=false;
    check(silent,"callback without an owner fails safely to silence");

    audio.play(tower::SoundSynth::MENU_SELECT);
    callback(stereo,frames,callbackUser);
    bool nonzero=false,stereoMatches=true;
    for(unsigned int i=0;i<frames;i++) {
        if(stereo[i*2]) nonzero=true;
        if(stereo[i*2]!=stereo[i*2+1]) stereoMatches=false;
    }
    check(nonzero && stereoMatches,"menu cue plays nonblocking stereo PCM");
    audio.play(tower::SoundSynth::GAME_OVER);
    callback(stereo,frames,callbackUser);
    nonzero=false;
    for(unsigned int i=0;i<frames*2;i++) if(stereo[i]) nonzero=true;
    check(nonzero,"new cue interrupts an earlier effect");
    for(int j=0;j<60;j++) callback(stereo,frames,callbackUser);
    silent=true;
    for(unsigned int i=0;i<frames*2;i++) if(stereo[i]) silent=false;
    check(silent,"finished cue returns to silence");
    audio.shutdown();
    check(stops==1 && releases==1 && !audio.available(),
          "shutdown frees callback channel");
    audio.play(tower::SoundSynth::MENU_MOVE); // benign no-op
    printf("Audio backend tests: %s\n",errors?"FAILED":"PASSED");
    return errors?1:0;
}
