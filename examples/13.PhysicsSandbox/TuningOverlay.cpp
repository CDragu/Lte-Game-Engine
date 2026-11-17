#include "TuningOverlay.h"
#include "Tuning.h"
#include <engine.h>

namespace tower {
void TuningOverlay::draw(engine::video::IVideoDriver* driver,
                         engine::gui::IGUIFont* font,
                         const Tuning& tuning,const TuningEditor& editor) const
{
    if(!driver || !font) return;
    // Textures not needed; the underlying 3D simulation remains visible.
    const int left=27,right=390;
    driver->draw2DRectangle(engine::video::SColor(232,9,21,39),
        engine::core::rect<engine::s32>(left,12,right,260));
    driver->draw2DRectangle(engine::video::SColor(255,243,181,85),
        engine::core::rect<engine::s32>(left,12,right,16));
    font->draw(L"LIVE TUNING - HOLD L",
        engine::core::rect<engine::s32>(40,22,370,41),
        engine::video::SColor(255,255,215,152));

    for(int i=0;i<Tuning::FIELD_COUNT;i++) {
        const int y=44+i*21;
        const bool selected=i==editor.selected();
        if(selected)
            driver->draw2DRectangle(engine::video::SColor(255,241,180,90),
                engine::core::rect<engine::s32>(36,y-1,382,y+20));
        const engine::video::SColor color=selected ?
            engine::video::SColor(255,21,32,48):
            engine::video::SColor(255,232,240,250);
        engine::core::stringw label=selected?L"> ":L"  ";
        label+=tuning.entry(i).label;
        font->draw(label.c_str(),
            engine::core::rect<engine::s32>(43,y+3,245,y+20),color);
        engine::core::stringw value;
        const Tuning::Entry& field=tuning.entry(i);
        if(field.integer) {
            value+=(int)(field.value+0.5f);
        } else {
            int hundredths=(int)(field.value*100.0f+0.5f);
            value+=hundredths/100;
            value+=L".";
            if(hundredths%100<10) value+=L"0";
            value+=hundredths%100;
        }
        font->draw(value.c_str(),
            engine::core::rect<engine::s32>(271,y+3,374,y+20),color);
    }
    font->draw(L"UP/DOWN: ITEM   LEFT/RIGHT: VALUE",
        engine::core::rect<engine::s32>(40,236,386,248),
        engine::video::SColor(255,215,230,244));
    font->draw(L"RELEASE L TO SAVE",
        engine::core::rect<engine::s32>(40,248,385,261),
        engine::video::SColor(255,255,218,157));
}
}
