#ifndef TOWER_TUNING_OVERLAY_H
#define TOWER_TUNING_OVERLAY_H

namespace engine {
namespace video { class IVideoDriver; }
namespace gui { class IGUIFont; }
}
namespace tower {
class Tuning;
class TuningEditor;

// LTE-only visualization of the PSP developer editor.
class TuningOverlay {
public:
    void draw(engine::video::IVideoDriver* driver,
              engine::gui::IGUIFont* font,
              const Tuning& values,const TuningEditor& editor) const;
};
}
#endif
