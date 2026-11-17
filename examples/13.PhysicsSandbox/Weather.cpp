#include "Weather.h"
#include <math.h>

namespace tower {
static float absWind(float x) { return x<0?-x:x; }
static float bound(float v,float a,float b) { return v<a?a:(v>b?b:v); }
Weather::Weather() : time_(0), x_(0), derivative_(0),
                     strength_(1.3f), gust_(0.40f), period_(14.0f) {
    reset();
}
void Weather::reset()
{
    time_=0.0f;
    derivative_=0.0f;
    recalculate(0.0f);
}
void Weather::configure(float strength,float gust,float period)
{
    if(strength>=0 && strength<=6.0f) strength_=strength;
    if(gust>=0 && gust<=1.0f) gust_=gust;
    if(period>=4.0f && period<=40.0f) period_=period;
    recalculate(0.0f); // changes in live tuning take effect without a time jump
}
void Weather::step(float dt)
{
    if(!(dt>0.0f && dt<=0.05f)) return;
    time_+=dt;
    if(time_>period_*100.0f) time_-=period_*100.0f;
    recalculate(dt);
}
void Weather::recalculate(float dt)
{
    const float old=x_;
    const float cycle=6.28318530718f*time_/period_;
    // Slowly reverse direction with an independent quicker gust ripple.
    // Multipliers sum to <= 1 so wind never exceeds the tunable maximum.
    const float wave=0.67f*sinf(cycle+0.8f)+
                     (0.33f*gust_)*sinf(cycle*3.1f+1.4f);
    x_=strength_*bound(wave,-1.0f,1.0f);
    derivative_=dt>0.0f ? (x_-old)/dt : 0.0f;
}
float Weather::fraction() const
{
    if(strength_<=0.0001f) return 0.0f;
    return bound(absWind(x_)/strength_,0.0f,1.0f);
}
int Weather::direction() const
{
    if(absWind(x_)<0.08f) return 0;
    return x_>0.0f?1:-1;
}
Weather::Severity Weather::severity() const
{
    if(direction()==0) return CALM;
    const float value=fraction();
    if(value<0.28f) return BREEZE;
    if(value<0.50f) return WINDY;
    if(value<0.74f) return STRONG;
    return GALE;
}
const wchar_t* Weather::severityLabel(Severity s)
{
    switch(s) {
    case BREEZE: return L"BREEZE";
    case WINDY: return L"WINDY";
    case STRONG: return L"STRONG";
    case GALE: return L"GALE";
    default: return L"CALM";
    }
}
}
