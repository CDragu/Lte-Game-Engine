#include "GameSounds.h"
#include <pspaudiolib.h>
#include <pspkernel.h>

// These helpers are supplied by LTE's bundled pspaudiolib implementation,
// which is already linked into libengine.a and owns all PSP audio channels.
extern "C" {
    int pspAudioChannelSearchFree();
    int pspAudioReleaseChannel(int chan);
    void pspAudioStopThisThread(int channel);
}

namespace tower {

GameSounds* GameSounds::active_=0;

GameSounds::GameSounds()
    : channel_(-1),requested_((int)SoundSynth::NONE),
      current_((int)SoundSynth::NONE),frame_(0)
{
    for(int i=0;i<SoundSynth::EFFECT_COUNT;i++) lengths_[i]=0;
}

bool GameSounds::initialize()
{
    if(channel_>=0) return true;
    // The engine device must already be initialized, since LTE's
    // CAudioDriver owns pspAudioInit() and the backing audio threads.
    for(int i=0;i<SoundSynth::EFFECT_COUNT;i++)
        lengths_[i]=SoundSynth::build((SoundSynth::Effect)i,
                                     samples_[i],SoundSynth::MAX_FRAMES);
    channel_=pspAudioChannelSearchFree();
    if(channel_<0) return false;
    requested_=(int)SoundSynth::NONE;
    current_=(int)SoundSynth::NONE;
    frame_=0;
    active_=this;
    // Conservative UI/SFX level, leaving headroom for later music.
    pspAudioSetVolume(channel_,PSP_VOLUME_MAX*55/100,PSP_VOLUME_MAX*55/100);
    // One callback worker continuously plays a silent buffer while idle.
    // No blocking audio output or decoder work occurs in the 60 Hz game loop.
    pspAudioSetChannelCallback(channel_,&GameSounds::callback,0);
    return true;
}

void GameSounds::play(SoundSynth::Effect effect)
{
    if(channel_<0 || effect<0 || effect>=SoundSynth::EFFECT_COUNT) return;
    // Last event wins if several occur in one ~23ms audio chunk. The newest
    // user action interrupts any older cue, avoiding unbounded queued audio.
    requested_=(int)effect;
}

void GameSounds::callback(void* buffer,unsigned int sampleCount,void* /*userdata*/)
{
    GameSounds* sounds=active_;
    if(sounds) sounds->fill((short*)buffer,sampleCount);
    else {
        short* output=(short*)buffer;
        for(unsigned int i=0;i<sampleCount*2;i++) output[i]=0;
    }
}

void GameSounds::fill(short* output,unsigned int sampleCount)
{
    const int next=requested_;
    if(next>=0 && next<SoundSynth::EFFECT_COUNT) {
        requested_=(int)SoundSynth::NONE;
        current_=next;
        frame_=0;
    }
    for(unsigned int i=0;i<sampleCount;i++) {
        short s=0;
        if(current_>=0 && current_<SoundSynth::EFFECT_COUNT) {
            if(frame_<lengths_[current_]) s=samples_[current_][frame_++];
            if(frame_>=lengths_[current_]) current_=(int)SoundSynth::NONE;
        }
        output[2*i]=s;
        output[2*i+1]=s;
    }
}

void GameSounds::shutdown()
{
    if(channel_<0) return;
    // Stop the callback before the engine's audio driver destroys its threads.
    pspAudioStopThisThread(channel_);
    // Give any in-flight callback time to return before releasing our slot.
    sceKernelDelayThread(30000);
    active_=0;
    pspAudioReleaseChannel(channel_);
    channel_=-1;
}
}
