#include "../GameRules.h"
#include "../Weather.h"
#include <limits>
#include <math.h>
#include <stdio.h>

using namespace tower;
static int failures=0;
static void check(bool okay,const char* name) {
    printf("%s %s\n",okay?"PASS":"FAIL",name);
    if(!okay) ++failures;
}
static bool same(float a,float b) { return fabsf(a-b)<0.000001f; }

int main()
{
    {
        World world;
        const int body=world.addBody(Body::box(1,1,1,0,2));
        const Vec2 start=world.body(body).position;
        world.step(0.0f);
        world.step(-1.0f);
        world.step(0.051f);
        world.step(std::numeric_limits<float>::quiet_NaN());
        check(same(world.body(body).position.x,start.x) &&
              same(world.body(body).position.y,start.y),
              "invalid and non-finite timesteps leave physics unchanged");
    }
    {
        World world;
        for(int i=0;i<World::MAX_BODIES;++i)
            check(world.addBody(Body::box(1,1,1,0,(float)i))==i,
                  "world accepts each fixed-capacity slot");
        check(world.addBody(Body::box(1,1,1,0,99))==-1 &&
              world.bodyCount()==World::MAX_BODIES,
              "world rejects overflow without corrupting its count");
    }
    {
        World world;
        world.addBody(Body::box(10,1,0,0,-0.5f));
        const int body=world.addBody(Body::box(1,1,1,0,0.5f));
        world.step(1.0f/60.0f);
        check(world.hasContact(body),"contact exists before reset");
        world.reset();
        check(world.bodyCount()==0 && world.contactCount()==0 &&
              !world.hasContact(body),"reset clears bodies and contact state");
    }
    {
        Weather weather;
        const float before=weather.accelerationX();
        weather.step(std::numeric_limits<float>::quiet_NaN());
        check(same(weather.accelerationX(),before),
              "non-finite weather timestep is ignored");
    }
    printf("Robustness tests: %s\n",failures?"FAILED":"PASSED");
    return failures?1:0;
}
