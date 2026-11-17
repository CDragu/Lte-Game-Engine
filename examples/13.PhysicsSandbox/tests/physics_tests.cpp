#include "../Physics2D.h"
#include <math.h>
#include <stdio.h>

using namespace tower;
static int failed=0;
static void check(bool okay,const char* label)
{
    printf("%s %s\n", okay ? "PASS":"FAIL",label);
    if(!okay) failed++;
}
static float absolute(float a) { return a<0.0f?-a:a; }
static bool finiteAndBounded(const Body& b)
{
    return b.position.x==b.position.x && b.position.y==b.position.y &&
           absolute(b.position.x)<100 && absolute(b.position.y)<100 &&
           absolute(b.velocity.x)<100 && absolute(b.velocity.y)<100 &&
           absolute(b.angle)<100;
}
static World ground()
{
    World w;
    w.addBody(Body::box(20.0f,1.0f,0.0f,0.0f,-0.5f));
    return w;
}
int main()
{
    const float dt=1.0f/60.0f;
    {
        World w=ground();
        int a=w.addBody(Body::box(1,1,1,0,3));
        float previous=w.body(a).position.y;
        for(int i=0;i<15;i++) {
            w.step(dt);
            check(w.body(a).position.y<previous,"freefall descending");
            previous=w.body(a).position.y;
        }
    }
    {
        World w=ground();
        int a=w.addBody(Body::box(1,1,1,0,2));
        int sawContact=0;
        for(int i=0;i<300;i++) { w.step(dt); if(w.contactCount()) sawContact=1; }
        check(sawContact,"ground contact detected");
        check(absolute(w.body(a).position.y-0.5f)<0.15f,"box rests on floor");
        check(absolute(w.body(a).velocity.y)<0.4f,"resting box velocity bounded");
    }
    {
        World w=ground();
        for(int i=0;i<3;i++) w.addBody(Body::box(1,1,1,0.0f,0.52f+i*1.03f));
        for(int i=0;i<600;i++) w.step(dt);
        bool stable=true;
        for(int i=1;i<w.bodyCount();i++)
            if(!finiteAndBounded(w.body(i)) || w.body(i).position.y<-1.0f ||
               absolute(w.body(i).position.x)>1.5f) stable=false;
        check(stable,"three-block stack remains bounded 10 seconds");
        check(w.contactCount()>0,"stack maintains contacts");
    }
    {
        World w=ground();
        int bottom=w.addBody(Body::box(1,1,1,0,0.5f));
        int top=w.addBody(Body::box(1,1,1,0.35f,3.3f));
        float maximumAngle=0.0f;
        for(int i=0;i<180;i++) {
            w.step(dt);
            const float v=absolute(w.body(bottom).angle)+absolute(w.body(top).angle);
            if(v>maximumAngle) maximumAngle=v;
        }
        check(maximumAngle>0.02f,"off-center drop creates rotation");
    }
    {
        World a=ground(),b=ground();
        a.addBody(Body::box(1,1,1,0.2f,3.0f));
        b.addBody(Body::box(1,1,1,0.2f,3.0f));
        for(int i=0;i<120;i++) { a.step(dt);b.step(dt); }
        check(absolute(a.body(1).position.x-b.body(1).position.x)<0.000001f &&
              absolute(a.body(1).position.y-b.body(1).position.y)<0.000001f,
              "fixed-step repeatability");
    }
    printf("Physics tests: %s\n", failed?"FAILED":"PASSED");
    return failed?1:0;
}
