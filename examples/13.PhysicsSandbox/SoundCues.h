#ifndef TOWER_SOUND_CUES_H
#define TOWER_SOUND_CUES_H

#include "Physics2D.h"
#include "SoundSynth.h"

// Gameplay-impact gating; never buzz on continuous contacts.
namespace tower {
class SoundCues {
public:
    SoundCues() : releasedBody_(-1), impactPlayed_(false) {}
    void reset() { releasedBody_=-1; impactPlayed_=false; }
    void beginDrop(int bodyId) {
        releasedBody_=bodyId;
        impactPlayed_=false;
    }
    SoundSynth::Effect afterPhysicsStep(const World& world) {
        if(impactPlayed_ || releasedBody_<0 ||
           releasedBody_>=world.bodyCount()) return SoundSynth::NONE;
        if(world.hasContact(releasedBody_)) {
            impactPlayed_=true;
            return SoundSynth::IMPACT;
        }
        return SoundSynth::NONE;
    }
private:
    int releasedBody_;
    bool impactPlayed_;
};
}
#endif
