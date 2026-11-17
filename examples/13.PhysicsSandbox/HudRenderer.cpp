#include "HudRenderer.h"
#include "HudStats.h"
#include <engine.h>

namespace tower {
void HudRenderer::draw(engine::video::IVideoDriver* driver,
                       engine::gui::IGUIFont* font,
                       const HudStats& stats) const
{
    if (!driver || !font) return;
    // Place the gameplay counters at the upper right, away from the debug
    // telemetry at upper left. PSP screen is 480x272.
    const int x1=336, x2=474, rowHeight=47, yStart=36;
    for (int i=0;i<stats.metricCount();++i) {
        const int y=yStart+i*(rowHeight+5);
        if (y+rowHeight>244) break; // don't cover the bottom gameplay hint
        driver->draw2DRectangle(engine::video::SColor(212,14,25,43),
                                engine::core::rect<engine::s32>(x1,y,x2,y+rowHeight));
        driver->draw2DRectangle(engine::video::SColor(255,245,180,83),
                                engine::core::rect<engine::s32>(x1,y,x1+3,y+rowHeight));
        const HudStats::Metric& metric=stats.metric(i);
        font->draw(metric.label,engine::core::rect<engine::s32>(x1+10,y+6,x2-5,y+22),
                   engine::video::SColor(255,219,230,244));
        engine::core::stringw value;
        value+=metric.value;
        font->draw(value.c_str(),
                   engine::core::rect<engine::s32>(x1+10,y+25,x2-5,y+44),
                   engine::video::SColor(255,255,203,119));
    }
}
} // namespace tower
