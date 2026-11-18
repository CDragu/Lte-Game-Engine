#ifndef TOWER_BLOCK_COLORS_H
#define TOWER_BLOCK_COLORS_H
#include <math.h>

// Deterministic unique colours in C++98, independent of LTE/PSP.
namespace tower {
struct BlockRgb {
    unsigned char r,g,b;
    BlockRgb(unsigned char red=0,unsigned char green=0,unsigned char blue=0)
        :r(red),g(green),b(blue){}
    bool operator==(const BlockRgb& v) const {return r==v.r && g==v.g && b==v.b;}
    bool operator!=(const BlockRgb& v) const {return !(*this==v);}
};
inline unsigned char rgbByte(float v) {
    if(v<0) v=0;
    if(v>255) v=255;
    return (unsigned char)(v+0.5f);
}
inline BlockRgb groundColor() {return BlockRgb(105,116,133);}
inline BlockRgb blockColor(int index) {
    if(index<0) index=0;
    // Golden-angle hue spacing: each consecutive block gets a distinct hue.
    const float h=fmodf((float)index*137.50776f+18.0f,360.0f)/60.0f;
    const float c=0.96f*0.70f;
    const float x=c*(1.0f-fabsf(fmodf(h,2.0f)-1.0f));
    const float m=0.96f-c;
    float r=0,g=0,b=0;
    if(h<1) {r=c;g=x;}
    else if(h<2) {r=x;g=c;}
    else if(h<3) {g=c;b=x;}
    else if(h<4) {g=x;b=c;}
    else if(h<5) {r=x;b=c;}
    else {r=c;b=x;}
    return BlockRgb(rgbByte((r+m)*255),rgbByte((g+m)*255),rgbByte((b+m)*255));
}
inline BlockRgb blockFaceColor(const BlockRgb& base,int face) {
    // Faces are front, back, right, left, top, bottom.
    static const float shade[6]={1.0f,0.66f,0.75f,0.87f,1.12f,0.58f};
    const float v=(face>=0 && face<6)?shade[face]:1.0f;
    return BlockRgb(rgbByte(base.r*v),rgbByte(base.g*v),rgbByte(base.b*v));
}
}
#endif
