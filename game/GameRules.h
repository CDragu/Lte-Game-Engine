#ifndef TOWER_GAME_RULES_H
#define TOWER_GAME_RULES_H

#include "Physics2D.h"
#include "SoundSynth.h"
#include "Tuning.h"
#include <math.h>

namespace tower {

// Apply live settings at the fixed-step boundary so a partially edited frame
// cannot mix old and new physics values.
inline void applyTuning(World& world,const Tuning& tuning)
{
    world.setDynamics(tuning.get(Tuning::GRAVITY),
                      tuning.get(Tuning::LINEAR_DAMPING),
                      tuning.get(Tuning::ANGULAR_DAMPING));
    for(int i=0;i<world.bodyCount();++i) {
        world.body(i).friction=tuning.get(Tuning::FRICTION);
        world.body(i).restitution=tuning.get(Tuning::RESTITUTION);
    }
}

// Counts a released block once it is supported by the existing tower and has
// remained nearly still for the configured settling window.
class StackProgress {
public:
    StackProgress() : stacked_(0),candidate_(-1),settleTime_(0.0f),
                      counted_(false),requiredSettlingTime_(0.35f) {}
    void reset(int initialBoxes) {
        stacked_=initialBoxes<0 ? 0 : initialBoxes;
        candidate_=-1;
        settleTime_=0.0f;
        counted_=false;
    }
    void beginDrop(int bodyId) {
        candidate_=bodyId;
        settleTime_=0.0f;
        counted_=false;
    }
    void setRequiredSettlingTime(float seconds) {
        if(seconds>=0.05f && seconds<=3.0f) requiredSettlingTime_=seconds;
    }
    float requiredSettlingTime() const { return requiredSettlingTime_; }
    int stackedCount() const { return stacked_; }
    void step(const World& world,float dt) {
        if(!(dt>0.0f && dt<=0.05f) || candidate_<=0 ||
           candidate_>=world.bodyCount() || counted_) return;

        const Body& block=world.body(candidate_);
        bool supportedByTower=false;
        // Body zero is static ground and cannot award a successful stack.
        for(int i=1;i<candidate_;++i) {
            if(world.isSupportedBy(candidate_,i)) {
                supportedByTower=true;
                break;
            }
        }
        const bool calm=fabsf(block.velocity.x)<0.35f &&
                        fabsf(block.velocity.y)<0.35f &&
                        fabsf(block.angularVelocity)<0.40f;
        if(supportedByTower && calm) {
            settleTime_+=dt;
            if(settleTime_>=requiredSettlingTime_) {
                ++stacked_;
                counted_=true;
            }
        } else {
            settleTime_=0.0f;
        }
    }
private:
    int stacked_,candidate_;
    float settleTime_;
    bool counted_;
    float requiredSettlingTime_;
};

// Detects an upper body falling below the seeded foundation's top surface.
class TowerFailureRule {
public:
    TowerFailureRule() : planeY_(0.0f),foundationIndex_(-1),armed_(false) {}
    void reset(const World& world,int foundationIndex) {
        armed_=foundationIndex>0 && foundationIndex<world.bodyCount();
        foundationIndex_=armed_ ? foundationIndex : -1;
        planeY_=armed_ ? world.body(foundationIndex_).position.y+
                         world.body(foundationIndex_).halfHeight : 0.0f;
    }
    float planeY() const { return planeY_; }
    bool hasFailed(const World& world) const {
        if(!armed_ || foundationIndex_>=world.bodyCount()) return false;
        for(int i=foundationIndex_+1;i<world.bodyCount();++i) {
            const Body& body=world.body(i);
            if(body.isDynamic() && body.position.y<planeY_-0.01f) return true;
        }
        return false;
    }
private:
    float planeY_;
    int foundationIndex_;
    bool armed_;
};

// Emits one impact cue for each released body, not one per contact frame.
class SoundCues {
public:
    SoundCues() : releasedBody_(-1),impactPlayed_(false) {}
    void reset() { releasedBody_=-1; impactPlayed_=false; }
    void beginDrop(int bodyId) { releasedBody_=bodyId; impactPlayed_=false; }
    SoundSynth::Effect afterPhysicsStep(const World& world) {
        if(impactPlayed_ || releasedBody_<0 ||
           releasedBody_>=world.bodyCount()) return SoundSynth::NONE;
        if(!world.hasContact(releasedBody_)) return SoundSynth::NONE;
        impactPlayed_=true;
        return SoundSynth::IMPACT;
    }
private:
    int releasedBody_;
    bool impactPlayed_;
};

} // namespace tower
#endif
