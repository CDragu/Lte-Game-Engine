#include "../GameRules.h"
#include <stdio.h>

using namespace tower;
static int failures=0;
static void check(bool ok,const char* name) {
    printf("%s %s\n",ok?"PASS":"FAIL",name);
    if(!ok) ++failures;
}
static World towerWorld() {
    World w;
    w.addBody(Body::box(14.0f,0.8f,0.0f,0.0f,-0.4f)); // ground
    w.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,0.52f)); // reference base
    w.addBody(Body::box(1.6f,1.0f,1.2f,0.04f,1.56f));
    w.addBody(Body::box(1.6f,1.0f,1.2f,-0.03f,2.60f));
    return w;
}
int main() {
    {
        World w=towerWorld();
        TowerFailureRule rule;
        check(!rule.hasFailed(w),"unarmed detector never fails");
        rule.reset(w,1);
        check(rule.planeY()>1.0f && rule.planeY()<1.1f,
              "reference plane is top of lowest foundation block");
        check(!rule.hasFailed(w),"seeded stack does not lose on boot");
        for(int i=0;i<50;i++) {
            w.step(1.0f/60.0f);
            if(rule.hasFailed(w)) { check(false,"stable stack after simulation"); break; }
            if(i==49) check(true,"stable stack after simulation");
        }
        const int dropped=w.addBody(Body::box(1.6f,1.0f,1.2f,4.5f,2.4f));
        check(dropped==4 && !rule.hasFailed(w),
              "missed release above threshold still playing");
        w.body(dropped).position.y=rule.planeY()+0.02f;
        check(!rule.hasFailed(w),"block just above plane is safe");
        w.body(dropped).position.y=rule.planeY()-0.02f;
        check(rule.hasFailed(w),"dropped box crosses below plane: lose");
    }
    {
        World w=towerWorld();
        TowerFailureRule rule;
        rule.reset(w,1);
        w.body(2).position.y=rule.planeY()-0.15f;
        check(rule.hasFailed(w),"an upper stacked box falling also loses");
    }
    {
        World w=towerWorld();
        TowerFailureRule rule;
        rule.reset(w,1);
        w.body(1).position.y=-5.0f;
        check(!rule.hasFailed(w),"foundation itself is exempt");
        w.reset();
        rule.reset(w,1);
        check(!rule.hasFailed(w),"reset with missing foundation disables detector");
        w=towerWorld();
        rule.reset(w,1);
        check(!rule.hasFailed(w),"fresh run clears previous failure");
    }
    {
        World w=towerWorld();
        TowerFailureRule rule;
        rule.reset(w,1);
        // Simulate a real miss. The ground catches the box at y ~0.5;
        // checking y<0 would never notice it.
        const int dropped=w.addBody(Body::box(1.6f,1.0f,1.2f,5.0f,4.0f));
        bool lost=false;
        for(int i=0;i<600;i++) {
            w.step(1.0f/60.0f);
            if(rule.hasFailed(w)) { lost=true; break; }
        }
        check(dropped==4 && lost,"off-tower drop loses before/at landing on ground");
    }
    printf("Failure rule tests: %s\n",failures?"FAILED":"PASSED");
    return failures?1:0;
}
