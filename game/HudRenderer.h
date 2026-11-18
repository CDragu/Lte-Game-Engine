#ifndef TOWER_HUD_RENDERER_H
#define TOWER_HUD_RENDERER_H

namespace engine {
namespace video { class IVideoDriver; }
namespace gui { class IGUIFont; }
}
namespace tower {
class HudStats;

// Pure presentation: the game/physics never depends on widget positions.
// New metrics registered in HudStats automatically get a tile.
class HudRenderer {
public:
    void draw(engine::video::IVideoDriver* driver,
              engine::gui::IGUIFont* font, const HudStats& stats) const;
};
}
#endif
