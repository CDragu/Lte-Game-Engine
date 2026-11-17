#ifndef TOWER_FAILURE_RULE_H
#define TOWER_FAILURE_RULE_H

#include "Physics2D.h"

// Loss detection belongs to gameplay rules, not LTE rendering.
// Current sandbox does not implement spline-frozen levels yet.
// Use the top surface of the lowest seeded 'spine'/foundation cuboid
// as a fixed Y reference. Upper dynamic block centers crossing below
// that plane indicate a missed drop or a collapsing upper stack.
namespace tower {
class TowerFailureRule {
public:
    TowerFailureRule() : planeY_(0.0f), foundationIndex_(-1), armed_(false) {}

    void reset(const World& world,int foundationIndex) {
        armed_=foundationIndex>0 && foundationIndex<world.bodyCount();
        foundationIndex_=armed_ ? foundationIndex : -1;
        planeY_=armed_ ? world.body(foundationIndex_).position.y+
                         world.body(foundationIndex_).halfHeight : 0.0f;
    }

    float planeY() const { return planeY_; }

    bool hasFailed(const World& world) const {
        if(!armed_ || foundationIndex_>=world.bodyCount()) return false;
        // The foundation block is exempt because it STARTS below its own
        // top-surface plane; only tower blocks above it can fall through.
        for(int i=foundationIndex_+1;i<world.bodyCount();++i) {
            const Body& b=world.body(i);
            if(b.isDynamic() && b.position.y < planeY_-0.01f)
                return true;
        }
        return false;
    }
private:
    float planeY_;
    int foundationIndex_;
    bool armed_;
};
}
#endif
