#include "../SoundSynth.h"
#include "../GameRules.h"
#include <stdio.h>
#include <stdlib.h>

using namespace tower;
static int errors=0;
static void check(bool value,const char* what) {
    printf("%s %s\n",value?"PASS":"FAIL",what);
    if(!value) errors++;
}
int main()
{
    short buffer[SoundSynth::MAX_FRAMES];
    for(int effect=0;effect<SoundSynth::EFFECT_COUNT;++effect) {
        const int frames=SoundSynth::build((SoundSynth::Effect)effect,
                                          buffer,SoundSynth::MAX_FRAMES);
        int peak=0;
        for(int i=0;i<frames;++i) {
            const int v=abs((int)buffer[i]);
            if(v>peak) peak=v;
        }
        check(frames>SoundSynth::SAMPLE_RATE/40 &&
              frames<=SoundSynth::MAX_FRAMES && peak>400 && peak<=30000,
              "each sound has short, non-silent, unclipped PCM");
        check(abs((int)buffer[frames-1])<250,"each sound ends quietly");
    }
    check(SoundSynth::build(SoundSynth::NONE,buffer,SoundSynth::MAX_FRAMES)==0,
          "no sound effect produces no samples");
    check(SoundSynth::build(SoundSynth::MENU_MOVE,0,100)==0,
          "null output buffer rejected");
    {
        World world;
        world.addBody(Body::box(10,1,0,0,-0.5f));
        const int drop=world.addBody(Body::box(1,1,1,0,2.0f));
        SoundCues cues;
        check(cues.afterPhysicsStep(world)==SoundSynth::NONE,
              "no drop is silent");
        cues.beginDrop(drop);
        SoundSynth::Effect e=SoundSynth::NONE;
        for(int i=0;i<180;i++) {
            world.step(1.0f/60.0f);
            if(cues.afterPhysicsStep(world)==SoundSynth::IMPACT) {
                e=SoundSynth::IMPACT;
                break;
            }
        }
        check(e==SoundSynth::IMPACT,"first impact produces a sound cue");
        bool repeated=false;
        for(int i=0;i<120;i++) {
            world.step(1.0f/60.0f);
            if(cues.afterPhysicsStep(world)!=SoundSynth::NONE) repeated=true;
        }
        check(!repeated,"resting box cannot repeat the impact sound");
        cues.reset();
        check(cues.afterPhysicsStep(world)==SoundSynth::NONE,
              "reset clears the watched drop");
        cues.beginDrop(drop);
        check(cues.afterPhysicsStep(world)==SoundSynth::IMPACT,
              "new tracked drop can trigger again");
    }
    printf("Sound tests: %s\n",errors?"FAILED":"PASSED");
    return errors?1:0;
}
