#ifndef TOWER_STACK_PROGRESS_H
#define TOWER_STACK_PROGRESS_H

#include "Physics2D.h"
#include <math.h>

// Gameplay accounting, not a rendering or HUD concern.
// Count only a newly released box that remains on top of an existing
// tower box with sufficiently small motion for a short settling window.
// The starting three demo boxes count as already stacked.
namespace tower {
class StackProgress {
public:
    StackProgress() { reset(0); }
    void reset(int initialBoxes) {
        initial_ = initialBoxes < 0 ? 0 : initialBoxes;
        stacked_ = initial_;
        candidate_ = -1;
        settleTime_ = 0.0f;
        counted_ = false;
    }
    void beginDrop(int bodyId) {
        candidate_ = bodyId;
        settleTime_ = 0.0f;
        counted_ = false;
    }
    int stackedCount() const { return stacked_; }
    bool countedCurrentDrop() const { return counted_; }
    void step(const World& world,float dt) {
        if (dt<=0.0f || candidate_ <= 0 || candidate_ >= world.bodyCount() || counted_)
            return;
        const Body& block = world.body(candidate_);
        bool supportedByTower = false;
        // Body 0 is the ground, and is deliberately not an eligible support.
        // For this one-drop slice all lower body IDs are established tower boxes.
        for(int i=1;i<candidate_;++i) {
            if(world.isSupportedBy(candidate_,i)) {
                supportedByTower = true;
                break;
            }
        }
        const bool calm=fabsf(block.velocity.x)<0.35f &&
                        fabsf(block.velocity.y)<0.35f &&
                        fabsf(block.angularVelocity)<0.40f;
        if(supportedByTower && calm) {
            settleTime_+=dt;
            if(settleTime_>=0.35f) {
                ++stacked_;
                counted_=true;
            }
        } else {
            settleTime_=0.0f;
        }
    }
private:
    int initial_,stacked_,candidate_;
    float settleTime_;
    bool counted_;
};
}
#endif
