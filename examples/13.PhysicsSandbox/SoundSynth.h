#ifndef TOWER_SOUND_SYNTH_H
#define TOWER_SOUND_SYNTH_H

// Procedural, original arcade effects, independent of PSP/LTE.
// 44,100 Hz mono PCM is duplicated to stereo by the PSP audio callback.
namespace tower {
class SoundSynth {
public:
    enum Effect {
        NONE = -1,
        MENU_MOVE = 0,
        MENU_SELECT,
        MENU_BACK,
        DROP,
        IMPACT,
        STACK,
        GAME_OVER,
        PAUSE,
        TUNE_CHANGE,
        TUNE_SAVED,
        EFFECT_COUNT
    };
    enum { SAMPLE_RATE = 44100, MAX_FRAMES = 20000 };
    static int build(Effect effect, short* dst, int capacity);
};
}
#endif
