#ifndef TOWER_GAME_SOUNDS_H
#define TOWER_GAME_SOUNDS_H

#include "SoundSynth.h"

// PSP-only audio adapter. LTE initializes pspaudiolib and reserves the
// hardware channels; this claims ONE of its free callback channels.
// Effect samples are synthesized on startup, not loaded from disk.
namespace tower {
class GameSounds {
public:
    GameSounds();
    bool initialize(); // false means no channel; gameplay remains functional
    void play(SoundSynth::Effect effect); // never blocks the game loop
    void shutdown();
    bool available() const { return channel_>=0; }

    // Called by LTE's PSP audio worker thread.
    static void callback(void* buffer,unsigned int sampleCount,void* userdata);
private:
    GameSounds(const GameSounds&);
    GameSounds& operator=(const GameSounds&);
    void fill(short* stereo,unsigned int sampleCount);

    static GameSounds* active_;
    short samples_[SoundSynth::EFFECT_COUNT][SoundSynth::MAX_FRAMES];
    int lengths_[SoundSynth::EFFECT_COUNT];
    int channel_;
    volatile int requested_;
    int current_;
    int frame_;
};
}
#endif
