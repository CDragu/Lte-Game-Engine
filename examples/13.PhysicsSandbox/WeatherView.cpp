#include "WeatherView.h"
#include "Weather.h"
#include <engine.h>

namespace tower {
namespace {
void setTint(engine::scene::ISceneNode* node,
             const engine::video::SColor& tint)
{
    if(!node) return;
    node->getMaterial(0).AmbientColor=tint;
    node->getMaterial(0).DiffuseColor=tint;
}
engine::scene::ISceneNode* bar(engine::scene::ISceneManager* smgr)
{
    engine::scene::ISceneNode* node=smgr->addTestSceneNode(1.0f);
    if(!node) return 0;
    node->setMaterialFlag(engine::video::EMF_LIGHTING,false);
    node->setMaterialFlag(engine::video::EMF_BACK_FACE_CULLING,false);
    return node;
}
engine::video::SColor windColor(Weather::Severity s)
{
    switch(s) {
    case Weather::GALE: return engine::video::SColor(255,240,89,87);
    case Weather::STRONG: return engine::video::SColor(255,245,151,79);
    case Weather::WINDY: return engine::video::SColor(255,245,204,115);
    case Weather::BREEZE: return engine::video::SColor(255,128,213,234);
    default: return engine::video::SColor(255,158,171,189);
    }
}
} // anonymous

WeatherView::WeatherView()
 : shaft_(0),headUp_(0),headDown_(0),visible_(false) {}

void WeatherView::initialize(engine::scene::ISceneManager* smgr)
{
    if(!smgr) return;
    shaft_=bar(smgr);
    headUp_=bar(smgr);
    headDown_=bar(smgr);
    setVisible(false);
}
void WeatherView::setVisible(bool visible)
{
    visible_=visible;
    if(!visible_) {
        if(shaft_) shaft_->setVisible(false);
        if(headUp_) headUp_->setVisible(false);
        if(headDown_) headDown_->setVisible(false);
    }
}
void WeatherView::update(const Weather& wind)
{
    if(!shaft_ || !headUp_ || !headDown_) return;
    const int direction=wind.direction();
    const bool show=visible_ && direction!=0;
    shaft_->setVisible(show);
    headUp_->setVisible(show);
    headDown_->setVisible(show);
    if(!show) return;
    // Actual volumetric arrow made of THREE 3D cuboids, facing left/right
    // along the X axis in the same plane as the tower.
    const float length=0.70f+wind.fraction()*0.95f;
    const float cx=-4.3f,cy=4.0f,z=1.0f;
    const float tip=cx+direction*length*0.5f;
    shaft_->setScale(engine::core::vector3df(length,0.11f,0.24f));
    shaft_->setPosition(engine::core::vector3df(cx,cy,z));
    shaft_->setRotation(engine::core::vector3df(0,0,0));
    const float armLength=0.37f;
    headUp_->setScale(engine::core::vector3df(armLength,0.11f,0.24f));
    headDown_->setScale(engine::core::vector3df(armLength,0.11f,0.24f));
    headUp_->setPosition(engine::core::vector3df(tip-direction*0.13f,cy+0.105f,z));
    headDown_->setPosition(engine::core::vector3df(tip-direction*0.13f,cy-0.105f,z));
    headUp_->setRotation(engine::core::vector3df(0,0,-direction*38.0f));
    headDown_->setRotation(engine::core::vector3df(0,0,direction*38.0f));
    const engine::video::SColor tint=windColor(wind.severity());
    setTint(shaft_,tint);
    setTint(headUp_,tint);
    setTint(headDown_,tint);
    shaft_->updateAbsolutePosition();
    headUp_->updateAbsolutePosition();
    headDown_->updateAbsolutePosition();
}

void WeatherView::drawHud(engine::video::IVideoDriver* driver,
                          engine::gui::IGUIFont* font,
                          const Weather& wind) const
{
    if(!driver || !font) return;
    // Fixed 480x272 layout; lower-left, separated from the score at upper-right.
    const int x=8,y=199;
    driver->draw2DRectangle(engine::video::SColor(223,11,25,43),
                            engine::core::rect<engine::s32>(x,y,x+130,y+63));
    driver->draw2DRectangle(windColor(wind.severity()),
                            engine::core::rect<engine::s32>(x,y,x+3,y+63));
    font->draw(L"WIND",
               engine::core::rect<engine::s32>(x+9,y+6,x+74,y+22),
               engine::video::SColor(255,234,241,250));
    font->draw(Weather::severityLabel(wind.severity()),
               engine::core::rect<engine::s32>(x+9,y+23,x+118,y+39),
               windColor(wind.severity()));
    engine::core::stringw direction=L"";
    if(wind.direction()>0) direction=L">> ";
    else if(wind.direction()<0) direction=L"<< ";
    else direction=L"-- ";
    direction+=(int)(wind.fraction()*100.0f+0.5f);
    direction+=L"%";
    font->draw(direction.c_str(),
               engine::core::rect<engine::s32>(x+9,y+43,x+120,y+58),
               engine::video::SColor(255,236,242,250));

    // Three flowing streaks form a visible wind icon. The strength increases
    // their length; all share the physical direction.
    const int d=wind.direction()==0?1:wind.direction();
    const int len=9+(int)(13.0f*wind.fraction());
    for(int k=0;k<3;k++) {
        const int iy=y+12+k*9;
        const int sx=x+96-d*len/2;
        driver->draw2DLine(engine::core::position2d<engine::s32>(sx,iy),
                           engine::core::position2d<engine::s32>(sx+d*len,iy),
                           windColor(wind.severity()));
    }
}
}
