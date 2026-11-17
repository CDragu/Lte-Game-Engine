#include "../HudStats.h"
#include "../StackProgress.h"
#include <stdio.h>
using namespace tower;
static int fails=0;
static void check(bool ok,const char* name) {
    printf("%s %s\n",ok?"PASS":"FAIL",name);
    if(!ok) ++fails;
}
static World startWorld()
{
    World w;
    w.addBody(Body::box(14.0f,0.8f,0.0f,0.0f,-0.4f));
    w.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,0.52f));
    w.addBody(Body::box(1.6f,1.0f,1.2f,0.04f,1.56f));
    w.addBody(Body::box(1.6f,1.0f,1.2f,-0.03f,2.60f));
    return w;
}
static void run(World& w,StackProgress& t,int steps) {
    for(int i=0;i<steps;i++) {
        w.step(1.0f/60.0f);
        t.step(w,1.0f/60.0f);
    }
}
int main()
{
    {
        HudStats stats;
        int boxes=stats.addCounter(L"BOXES STACKED",3);
        int score=stats.addCounter(L"SCORE",0);
        check(boxes==0 && score==1 && stats.metricCount()==2,
              "HUD accepts modular counters");
        check(stats.metric(boxes).value==3 && stats.metric(score).value==0,
              "HUD holds independent values");
        check(stats.setValue(boxes,4) && stats.metric(boxes).value==4,
              "HUD value updates");
        check(!stats.setValue(-1,123) && !stats.setValue(99,123),
              "HUD rejects unknown counter");
        check(stats.addCounter(L"HEIGHT",0)==2 &&
              stats.addCounter(L"BEST",0)==3 &&
              stats.addCounter(L"OVERFLOW",0)==-1,
              "HUD protects its fixed widget capacity");
    }
    {
        World w=startWorld();
        StackProgress t;
        t.reset(w.bodyCount()-1);
        check(t.stackedCount()==3,"three seeded tower blocks shown at boot");
        run(w,t,90);
        check(t.stackedCount()==3,"no drop does not increment counter");
        const int id=w.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,3.70f));
        t.beginDrop(id);
        check(t.stackedCount()==3,"drop is not counted on release");
        run(w,t,600);
        check(t.stackedCount()==4,"box settled on tower increments stacked count");
        run(w,t,600);
        check(t.stackedCount()==4,"settled box counted only once");
        t.reset(3);
        check(t.stackedCount()==3,"restart resets stacked count to initial three");
    }
    {
        World w=startWorld();
        StackProgress t;
        t.reset(3);
        const int id=w.addBody(Body::box(1.6f,1.0f,1.2f,5.0f,3.7f));
        t.beginDrop(id);
        run(w,t,600);
        check(t.stackedCount()==3,"a block landing on bare ground is not stacked");
    }
    printf("HUD/stack tracking tests: %s\n",fails?"FAILED":"PASSED");
    return fails?1:0;
}
