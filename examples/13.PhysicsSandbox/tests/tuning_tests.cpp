#include "../Tuning.h"
#include "../TuningPhysics.h"
#include "../StackProgress.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace tower;
static int failures=0;
static void check(bool okay,const char* name) {
    printf("%s %s\n",okay?"PASS":"FAIL",name);
    if(!okay) failures++;
}
static bool closeTo(float a,float b) { return fabsf(a-b)<0.001f; }
int main() {
    const char* path="tuning_test_output.cfg";
    {
        Tuning t;
        check(Tuning::FIELD_COUNT==13,"13 tweakable fields registered");
        check(closeTo(t.get(Tuning::GRAVITY),12.0f) &&
              closeTo(t.get(Tuning::SETTLING_TIME),0.35f) &&
              t.getInt(Tuning::SCORE_PER_BLOCK)==100,"expected startup defaults");
        TuningEditor editor;
        check(editor.selected()==0,"editor selects gravity first");
        editor.up();
        check(editor.selected()==0,"editor clamps first row");
        editor.change(t,-1);
        check(closeTo(t.get(Tuning::GRAVITY),11.5f),"negative edit applies immediately");
        editor.down(); editor.change(t,1);
        check(closeTo(t.get(Tuning::FRICTION),0.83f),"friction editable");
        for(int i=0;i<100;i++) editor.down();
        check(editor.selected()==Tuning::FIELD_COUNT-1,"editor clamps last row");
        editor.change(t,1);
        check(t.getInt(Tuning::WIND_DROPS)==2,"weather drop interval editable");
        check(t.set(Tuning::WIND_CYCLE,15.0f),"gust period is editable");
        check(t.set(Tuning::SCORE_PER_BLOCK,110.0f) &&
              t.getInt(Tuning::SCORE_PER_BLOCK)==110,
              "integer scoring uses steps");
        check(t.set(Tuning::WIND_STRENGTH,2.2f),"wind force is configurable");
        check(t.set(Tuning::WIND_GUSTINESS,0.65f),"wind gustiness is configurable");
        check(!t.set(Tuning::GRAVITY,-100.0f) &&
              !t.set(Tuning::GRAVITY,1.0f/0.0f),"invalid values rejected");
        check(t.save(path),"writes configuration file");
    }
    {
        Tuning reloaded;
        check(reloaded.load(path),"reads the same configuration file");
        check(closeTo(reloaded.get(Tuning::GRAVITY),11.5f) &&
              closeTo(reloaded.get(Tuning::FRICTION),0.83f) &&
              reloaded.getInt(Tuning::SCORE_PER_BLOCK)==110 &&
              closeTo(reloaded.get(Tuning::WIND_CYCLE),15.0f) &&
              closeTo(reloaded.get(Tuning::WIND_STRENGTH),2.2f) &&
              closeTo(reloaded.get(Tuning::WIND_GUSTINESS),0.65f) &&
              reloaded.getInt(Tuning::WIND_DROPS)==2,
              "all edited values survived round trip");
        check(reloaded.set(Tuning::SETTLING_TIME,1.25f),
              "settling time can change live");
        check(reloaded.save(path),"writes updated setting back to original path");
        Tuning checkAgain;
        check(checkAgain.load(path) && closeTo(checkAgain.get(Tuning::SETTLING_TIME),1.25f),
              "second save preserves changed settling time");
    }
    {
        Tuning missing;
        check(!missing.load("file_that_does_not_exist.cfg"),"missing file leaves defaults");
        check(closeTo(missing.get(Tuning::GRAVITY),12.0f),
              "defaults retained when load fails");
        check(!missing.save(""),"invalid save path rejected");
    }
    {
        FILE* f=fopen(path,"w");
        check(f!=0,"can write malformed input fixture");
        if(f) {
            fputs("# comment\nfriction=1.20\nunknown_future_setting=42\n"
                  "gravity=NaN\nsettling_time=hello\n"
                  "score_per_block=250\n",f);
            fclose(f);
            Tuning changed;
            check(changed.load(path) &&
                  closeTo(changed.get(Tuning::FRICTION),1.2f) &&
                  closeTo(changed.get(Tuning::GRAVITY),12.0f) &&
                  closeTo(changed.get(Tuning::SETTLING_TIME),0.35f) &&
                  changed.getInt(Tuning::SCORE_PER_BLOCK)==250,
                  "comments unknown invalid and valid values handled safely");
        }
    }
    {
        Tuning t;
        World w;
        int id=w.addBody(Body::box(1.0f,1.0f,1.0f,0.0f,4.0f));
        t.set(Tuning::GRAVITY,0.0f);
        t.set(Tuning::FRICTION,1.50f);
        t.set(Tuning::RESTITUTION,0.70f);
        applyTuning(w,t);
        w.step(1.0f/60.0f);
        check(fabsf(w.body(id).velocity.y)<0.001f,
              "zero gravity takes effect next physics step");
        check(closeTo(w.body(id).friction,1.5f) &&
              closeTo(w.body(id).restitution,0.7f),
              "friction and restitution apply to active bodies");
        t.set(Tuning::GRAVITY,30.0f);
        applyTuning(w,t);
        w.step(1.0f/60.0f);
        check(w.body(id).velocity.y<-0.45f,
              "changing gravity affects existing body immediately");
        StackProgress p;
        p.setRequiredSettlingTime(1.25f);
        check(closeTo(p.requiredSettlingTime(),1.25f),
              "tuning settling time reaches stacking rule");
    }
    remove(path);
    printf("Tuning tests: %s\n",failures?"FAILED":"PASSED");
    return failures?1:0;
}
