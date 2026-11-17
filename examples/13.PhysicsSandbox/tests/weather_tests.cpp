#include "../Weather.h"
#include "../Physics2D.h"
#include "../Tuning.h"
#include <math.h>
#include <stdio.h>
using namespace tower;
static int failures=0;
static void check(bool v,const char* what) {
    printf("%s %s\n",v?"PASS":"FAIL",what);
    if(!v) failures++;
}
static float absolute(float v) { return v<0?-v:v; }
int main() {
    const float dt=1.0f/60.0f;
    {
        Weather a,b;
        a.configure(2.0f,0.4f,10.0f);
        b.configure(2.0f,0.4f,10.0f);
        bool same=true;
        bool seenLeft=false,seenRight=false;
        float highest=0;
        for(int i=0;i<1800;i++) {
            a.step(dt);b.step(dt);
            if(absolute(a.accelerationX()-b.accelerationX())>0.0001f) same=false;
            if(a.direction()<0) seenLeft=true;
            if(a.direction()>0) seenRight=true;
            const float magnitude=absolute(a.accelerationX());
            if(magnitude>highest) highest=magnitude;
            if(magnitude>2.001f) same=false;
        }
        check(same && highest>0.7f,"deterministic gusts stay within configured strength");
        check(seenLeft && seenRight,"wind direction reverses over time");
        Weather light,gale;
        light.configure(0.5f,0.0f,14.0f);
        gale.configure(6.0f,0.0f,14.0f);
        check(light.severity()==Weather::BREEZE &&
              gale.severity()==Weather::GALE,
              "weather severity uses absolute force rather than cap percentage");
        a.reset();b.reset();
        check(absolute(a.accelerationX()-b.accelerationX())<0.00001f,
              "restart resets deterministic weather phase");
        a.configure(0,1.0f,10.0f);
        check(a.direction()==0 && a.severity()==Weather::CALM && a.fraction()==0.0f,
              "zero-strength wind is calm");
    }
    {
        Weather a;
        a.configure(4.0f,0.4f,15.0f);
        const float first=a.accelerationX();
        a.step(dt);
        check(absolute(a.accelerationX()-first)>0.00001f &&
              absolute(a.changeRateX())>0.00001f,
              "wind changes smoothly as fixed time advances");
        const float frozen=a.accelerationX();
        a.step(0.0f);
        check(absolute(a.accelerationX()-frozen)<0.00001f,
              "paused simulation does not change weather");
        a.configure(0.0f,0.0f,15.0f);
        check(absolute(a.accelerationX())<0.0001f,
              "live tuning force to zero immediately calms wind");
    }
    {
        World still,windy;
        const int stillId=still.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,5.0f));
        const int windId=windy.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,5.0f));
        still.setDynamics(0,0,0); windy.setDynamics(0,0,0);
        windy.setWindAcceleration(2.0f);
        for(int i=0;i<30;i++){still.step(dt);windy.step(dt);}
        check(windy.body(windId).position.x > still.body(stillId).position.x+0.15f,
              "right wind pushes dynamic block to the right");
        check(windy.body(windId).angle < still.body(stillId).angle-0.005f,
              "wind acting high on face rotates dynamic cuboid");
        windy.setWindAcceleration(-2.0f);
        check(windy.windAcceleration()==-2.0f,"wind force direction updates immediately");
        const int base=windy.addBody(Body::box(10,1,0,0,-0.5f));
        const float baseX=windy.body(base).position.x;
        for(int i=0;i<50;i++) windy.step(dt);
        check(windy.body(base).position.x==baseX,
              "wind cannot move static ground/foundation");
        windy.reset();
        check(windy.windAcceleration()==0.0f,"reset clears applied wind");
    }
    printf("Weather tests: %s\n",failures?"FAILED":"PASSED");
    return failures?1:0;
}
