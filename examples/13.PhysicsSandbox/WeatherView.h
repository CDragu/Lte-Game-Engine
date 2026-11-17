#ifndef TOWER_WEATHER_VIEW_H
#define TOWER_WEATHER_VIEW_H

namespace engine {
namespace scene { class ISceneManager; class ISceneNode; }
namespace video { class IVideoDriver; }
namespace gui { class IGUIFont; }
}
namespace tower {
class Weather;

// View only: no PSP controls, physics state, or tuning file dependencies.
// Draws a small always-on 2D weather indicator and an actual 3D scene arrow.
class WeatherView {
public:
    WeatherView();
    void initialize(engine::scene::ISceneManager* manager);
    void setVisible(bool visible);
    void update(const Weather& weather);
    void drawHud(engine::video::IVideoDriver* driver,
                 engine::gui::IGUIFont* font,const Weather& weather) const;
private:
    engine::scene::ISceneNode* shaft_;
    engine::scene::ISceneNode* headUp_;
    engine::scene::ISceneNode* headDown_;
    bool visible_;
};
}
#endif
