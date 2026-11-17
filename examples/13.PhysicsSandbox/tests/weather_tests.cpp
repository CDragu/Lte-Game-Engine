#include "../Weather.h"
#include "../Physics2D.h"
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
        a.configureDrops(3);b.configureDrops(3);
        bool same=true,keptDirection=true;
        const int firstDirection=a.direction();
        const float firstForce=a.accelerationX();
        bool gustChanged=false;
        for(int i=0;i<1800;i++) {
            a.step(dt);b.step(dt);
            if(absolute(a.accelerationX()-b.accelerationX())>0.0001f) same=false;
            if(a.direction()!=firstDirection) keptDirection=false;
            if(absolute(a.accelerationX()-firstForce)>0.05f) gustChanged=true;
            if(absolute(a.accelerationX())>2.001f) same=false;
        }
        check(same && keptDirection && gustChanged,
              "gusts vary while underlying direction persists without drops");
        check(a.weatherIndex()==0 && a.totalDrops()==0,
              "time alone never changes the weather profile");
        check(!a.onBlockDropped() && a.dropsUntilChange()==2,
              "first drop counts toward three-drop interval");
        check(!a.onBlockDropped() && a.dropsUntilChange()==1,
              "second drop leaves one until change");
        check(a.onBlockDropped() && a.dropsUntilChange()==3 &&
              a.weatherIndex()==1 && a.totalDrops()==3,
              "third drop changes profile and resets interval");
        for(int i=0;i<90;i++) {a.step(dt);b.step(dt);}
        check(a.direction()<0,"new profile smoothly changes wind direction");
        check(a.onBlockDropped()==false && a.dropsUntilChange()==2,
              "fourth drop begins the next three-drop group");
        a.reset();
        check(a.totalDrops()==0 && a.weatherIndex()==0 &&
              a.dropsUntilChange()==3 && a.direction()>0,
              "restart resets weather and drop progress");
        a.configure(0.0f,1.0f,10.0f);
        check(a.direction()==0 && a.severity()==Weather::CALM,
              "zero strength is always calm");
    }
    {
        Weather a;
        a.configureDrops(1);
        check(a.onBlockDropped() && a.weatherIndex()==1,
              "one-drop interval changes weather with first successful release");
        a.configure(4.0f,0.0f,15.0f);
        for(int i=0;i<70;i++) a.step(dt);
        const float settled=a.accelerationX();
        const int direction=a.direction();
        for(int i=0;i<900;i++) a.step(dt);
        check(a.direction()==direction && absolute(a.accelerationX()-settled)<0.001f,
              "no weather changes or gusts when no drops and gustiness zero");
        a.configure(4.0f,0.4f,15.0f);
        const float previous=a.accelerationX();
        a.step(dt);
        const float recorded=a.changeRateX();
        check(absolute(recorded)>0.00001f &&
              absolute(a.accelerationX()-previous)>0.00001f,
              "wind velocity is updated by 60Hz step");
        a.configure(4.0f,0.4f,15.0f);
        check(absolute(a.changeRateX()-recorded)<0.00001f,
              "render-time configure retains derivative");
        const float frozen=a.accelerationX();
        a.step(0.0f);
        check(absolute(a.accelerationX()-frozen)<0.00001f,
              "pause does not advance wind");
        a.configure(0.0f,0.0f,15.0f);
        check(a.direction()==0 && a.fraction()==0.0f,
              "live tuning to zero immediately calms wind");
    }
    {
        Weather w;
        w.configureDrops(5);
        w.onBlockDropped();w.onBlockDropped();w.onBlockDropped();
        w.configureDrops(2);
        check(w.dropsUntilChange()==1,
              "shortening interval waits for one more actual drop");
        check(w.onBlockDropped() && w.weatherIndex()==1,
              "next drop applies newly shortened interval");
        w.configureDrops(30);
        check(w.dropsPerChange()==30,"maximum supported interval accepted");
        w.configureDrops(0); w.configureDrops(31);
        check(w.dropsPerChange()==30,"invalid intervals rejected");
    }
    {
        Weather light,gale;
        light.configure(0.5f,0.0f,14.0f);
        gale.configure(6.0f,0.0f,14.0f);
        check(light.severity()==Weather::BREEZE &&
              gale.severity()==Weather::GALE,
              "severity depends on physical wind force");
    }
    {
        World still,windy;
        const int a=still.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,5.0f));
        const int b=windy.addBody(Body::box(1.6f,1.0f,1.2f,0.0f,5.0f));
        still.setDynamics(0,0,0);windy.setDynamics(0,0,0);
        windy.setWindAcceleration(2.0f);
        for(int i=0;i<30;i++){still.step(dt);windy.step(dt);}
        check(windy.body(b).position.x>still.body(a).position.x+0.15f,
              "rightward wind pushes a dynamic box");
        check(windy.body(b).angle<still.body(a).angle-0.005f,
              "wind also creates angular impulse");
        const int ground=windy.addBody(Body::box(10,1,0,0,-0.5f));
        const float x=windy.body(ground).position.x;
        for(int i=0;i<30;i++) windy.step(dt);
        check(windy.body(ground).position.x==x,"static ground resists wind");
    }
    printf("Weather tests: %s\n",failures?"FAILED":"PASSED");
    return failures?1:0;
}
