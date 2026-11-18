#include "Weather.h"
#include <math.h>

namespace tower {
static float absWind(float x) { return x<0?-x:x; }
static float bound(float v,float a,float b) { return v<a?a:(v>b?b:v); }

// Deliberately authored profiles, not random; every new round has exactly the
// same conditions. Alternating signs and strengths produce meaningful shifts.
static float profile(int index)
{
    const float sequence[]={0.50f,-0.75f,0.82f,-0.38f,0.95f,-0.58f};
    return sequence[index%6];
}
Weather::Weather()
 : gustTime_(0),x_(0),derivative_(0),strength_(1.3f),gust_(0.4f),
   period_(14.0f),fromBase_(0),toBase_(0),base_(0),transition_(1.0f),
   dropsPerChange_(1),dropsSinceChange_(0),totalDrops_(0),weatherIndex_(0)
{
    reset();
}
void Weather::reset()
{
    gustTime_=0.0f;
    derivative_=0.0f;
    weatherIndex_=0;
    dropsSinceChange_=totalDrops_=0;
    fromBase_=toBase_=base_=profile(0);
    transition_=1.0f;
    recalculate(0.0f);
}
void Weather::configure(float strength,float gust,float period)
{
    float newStrength=strength_, newGust=gust_, newPeriod=period_;
    if(strength>=0 && strength<=6.0f) newStrength=strength;
    if(gust>=0 && gust<=1.0f) newGust=gust;
    if(period>=4.0f && period<=40.0f) newPeriod=period;
    if(newStrength!=strength_ || newGust!=gust_ || newPeriod!=period_) {
        strength_=newStrength;
        gust_=newGust;
        period_=newPeriod;
        // A render-frame configure() call must not erase derivative_ or
        // accidentally advance the weather change counter.
        recalculate(0.0f);
    }
}
void Weather::configureDrops(int drops)
{
    if(drops>=1 && drops<=30) dropsPerChange_=drops;
}
bool Weather::onBlockDropped()
{
    if(dropsSinceChange_<dropsPerChange_) ++dropsSinceChange_;
    ++totalDrops_;
    if(dropsSinceChange_<dropsPerChange_) return false;
    dropsSinceChange_=0;
    ++weatherIndex_;
    fromBase_=base_;
    toBase_=profile(weatherIndex_);
    transition_=0.0f;
    return true;
}
void Weather::step(float dt)
{
    if(!(dt>0.0f && dt<=0.05f)) return;
    gustTime_+=dt;
    if(gustTime_>period_*100.0f) gustTime_-=period_*100.0f;
    if(transition_<1.0f) {
        transition_=bound(transition_+dt/0.85f,0.0f,1.0f);
        const float smooth=transition_*transition_*(3.0f-2.0f*transition_);
        base_=fromBase_+(toBase_-fromBase_)*smooth;
    }
    recalculate(dt);
}
void Weather::recalculate(float dt)
{
    const float old=x_;
    const float cycle=6.28318530718f*gustTime_/period_;
    // Gusts change magnitude (not direction) between drops. The underlying
    // crosswind switches only after the configured number of block releases.
    const float multiplier=1.0f+gust_*0.18f*sinf(cycle);
    x_=strength_*bound(base_*multiplier,-1.0f,1.0f);
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
    const float force=absWind(x_);
    if(force<0.65f) return BREEZE;
    if(force<1.35f) return WINDY;
    if(force<2.50f) return STRONG;
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
