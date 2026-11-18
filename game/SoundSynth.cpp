#include "SoundSynth.h"
#include <math.h>

namespace tower {
int SoundSynth::build(Effect effect,short* dst,int capacity)
{
    if(!dst || effect<0 || effect>=EFFECT_COUNT || capacity<=0) return 0;
    const float tau=6.28318530718f;
    int durationMs=90;
    float startHz=550.0f,endHz=700.0f,level=0.52f,noise=0.0f;
    switch(effect) {
    case MENU_MOVE: durationMs=75; startHz=620; endHz=780; level=0.28f; break;
    case MENU_SELECT: durationMs=175; startHz=520; endHz=960; level=0.45f; break;
    case MENU_BACK: durationMs=125; startHz=650; endHz=320; level=0.36f; break;
    case DROP: durationMs=180; startHz=780; endHz=280; level=0.43f; break;
    case IMPACT: durationMs=230; startHz=175; endHz=75; level=0.55f; noise=0.43f; break;
    case STACK: durationMs=260; startHz=490; endHz=980; level=0.49f; break;
    case GAME_OVER: durationMs=440; startHz=410; endHz=115; level=0.60f; noise=0.18f; break;
    case PAUSE: durationMs=100; startHz=560; endHz=400; level=0.32f; break;
    case TUNE_CHANGE: durationMs=48; startHz=850; endHz=920; level=0.24f; break;
    case TUNE_SAVED: durationMs=210; startHz=690; endHz=1120; level=0.40f; break;
    default: return 0;
    }
    int frames=(SAMPLE_RATE*durationMs)/1000;
    if(frames>capacity) frames=capacity;
    float phase=0.0f;
    unsigned int seed=0x9e3779b9u+(unsigned int)effect*1234567u;
    for(int i=0;i<frames;++i) {
        const float p=(float)i/(float)frames;
        float hz=startHz+(endHz-startHz)*p;
        // Pleasant ascending two-tone confirmation without an external sample.
        if(effect==MENU_SELECT && p>=0.48f) hz+=180.0f;
        if(effect==STACK && p>=0.52f) hz+=110.0f;
        phase+=tau*hz/(float)SAMPLE_RATE;
        if(phase>tau) phase-=tau;
        seed=1664525u*seed+1013904223u;
        const float random=(float)((seed>>16)&0x7fff)/16383.5f-1.0f;
        const float wave=0.73f*sinf(phase)+0.16f*sinf(2.0f*phase)+noise*random;
        // 3ms attack, quadratic decay, 6ms release avoids clicks.
        float env=(1.0f-p)*(1.0f-p);
        const float attack=(float)i/(0.003f*(float)SAMPLE_RATE);
        if(attack<1.0f) env*=attack;
        const float release=(float)(frames-i)/(0.006f*(float)SAMPLE_RATE);
        if(release<1.0f) env*=release;
        float output=wave*level*env*23000.0f;
        if(output>30000.0f) output=30000.0f;
        if(output< -30000.0f) output=-30000.0f;
        dst[i]=(short)output;
    }
    return frames;
}
}
