#ifndef TOWER_TUNING_PHYSICS_H
#define TOWER_TUNING_PHYSICS_H

#include "Tuning.h"
#include "Physics2D.h"

// Keeps the gameplay-to-physics binding portable for native unit tests.
// Called each fixed step so edits apply without restarting the game.
namespace tower {
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
}
#endif
