#include "Tuning.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

namespace tower {

void Tuning::add(Field field,const char* key,const wchar_t* label,
                 float initial,float minValue,float maxValue,float step,bool integer)
{
    Entry& e=items_[(int)field];
    e.key=key; e.label=label; e.value=initial;
    e.minimum=minValue; e.maximum=maxValue; e.step=step; e.integer=integer;
}

Tuning::Tuning()
{
    add(GRAVITY,"gravity",L"GRAVITY",12.0f,0.0f,35.0f,0.50f,false);
    add(FRICTION,"friction",L"FRICTION",0.78f,0.0f,2.0f,0.05f,false);
    add(RESTITUTION,"restitution",L"BOUNCE",0.03f,0.0f,0.95f,0.01f,false);
    add(LINEAR_DAMPING,"linear_damping",L"LINEAR DAMP",0.04f,0.0f,3.0f,0.02f,false);
    add(ANGULAR_DAMPING,"angular_damping",L"ANGULAR DAMP",0.06f,0.0f,3.0f,0.02f,false);
    add(SWING_AMPLITUDE,"swing_amplitude",L"SWING ARC",0.60f,0.05f,1.25f,0.05f,false);
    add(SWING_SPEED,"swing_speed",L"SWING SPEED",2.05f,0.20f,6.00f,0.05f,false);
    add(SETTLING_TIME,"settling_time",L"SETTLE SEC",0.35f,0.05f,3.00f,0.05f,false);
    add(SCORE_PER_BLOCK,"score_per_block",L"POINTS/BOX",100.0f,0.0f,1000.0f,10.0f,true);
}

bool Tuning::set(Field field,float v)
{
    const int index=(int)field;
    if(index<0 || index>=FIELD_COUNT) return false;
    Entry& e=items_[index];
    if(!(v>=e.minimum && v<=e.maximum)) return false; // catches NaN too
    if(e.integer) v=floorf(v+0.5f);
    e.value=v;
    return true;
}

bool Tuning::adjust(int index,int direction)
{
    if(index<0 || index>=FIELD_COUNT || direction==0) return false;
    Entry& e=items_[index];
    float next=e.value+(direction>0?e.step:-e.step);
    if(next<e.minimum) next=e.minimum;
    if(next>e.maximum) next=e.maximum;
    // Keep three decimal places without snapping existing file values to
    // a step grid anchored at zero (e.g. friction 0.78 + 0.05 = 0.83).
    next=floorf(next*1000.0f+0.50001f)/1000.0f;
    if(next>e.maximum) next=e.maximum;
    if(e.integer) next=floorf(next+0.5f);
    if(fabsf(next-e.value)<0.00001f) return false;
    e.value=next;
    return true;
}

static char* strip(char* text)
{
    while(*text && isspace((unsigned char)*text)) ++text;
    char* end=text+strlen(text);
    while(end>text && isspace((unsigned char)end[-1])) *--end='\0';
    return text;
}

bool Tuning::load(const char* path)
{
    if(!path || !*path) return false;
    FILE* file=fopen(path,"r");
    if(!file) return false;
    char line[160];
    while(fgets(line,sizeof(line),file)) {
        // Ignore comments, empty lines and unfamiliar keys.
        char* raw=strip(line);
        if(!*raw || *raw=='#' || *raw==';') continue;
        char* equal=strchr(raw,'=');
        if(!equal) continue;
        *equal='\0';
        const char* key=strip(raw);
        char* value=strip(equal+1);
        char* end=0;
        const double parsed=strtod(value,&end);
        if(end==value) continue;
        end=strip(end);
        if(*end && *end!='#' && *end!=';') continue;
        for(int i=0;i<FIELD_COUNT;i++)
            if(strcmp(key,items_[i].key)==0) {
                set((Field)i,(float)parsed);
                break;
            }
    }
    const bool okay=ferror(file)==0;
    fclose(file);
    return okay;
}

bool Tuning::save(const char* path) const
{
    if(!path || !*path) return false;
    // The same path is used on boot and on left-trigger release.
    // Keep it a plain text file so tuned values can be committed to Git.
    FILE* file=fopen(path,"w");
    if(!file) return false;
    bool okay=fprintf(file,"# Tower Physics live tuning (edited in-game)\n")>=0;
    okay=(fprintf(file,"# Hold L; D-pad up/down select, left/right adjust; release L to save.\n")>=0) && okay;
    for(int i=0;i<FIELD_COUNT;i++) {
        const Entry& e=items_[i];
        const int wrote=e.integer ?
            fprintf(file,"%s=%d\n",e.key,(int)(e.value+0.5f)) :
            fprintf(file,"%s=%.3f\n",e.key,(double)e.value);
        if(wrote<0) okay=false;
    }
    if(fflush(file)!=0) okay=false;
    if(fclose(file)!=0) okay=false;
    return okay;
}
}
