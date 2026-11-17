#ifndef TOWER_MENU_FLOW_H
#define TOWER_MENU_FLOW_H

// Pure C++98 menu/navigation state; independent of LTE and PSP SDK.
namespace tower {
class MenuFlow {
public:
    enum Screen { MAIN_MENU, PLAYING, EXIT_REQUESTED };
    enum Selection { PLAY, EXIT };
    MenuFlow() : screen_(MAIN_MENU), selection_(PLAY) {}
    Screen screen() const { return screen_; }
    Selection selection() const { return selection_; }
    void up() { if (screen_ == MAIN_MENU) selection_ = PLAY; }
    void down() { if (screen_ == MAIN_MENU) selection_ = EXIT; }
    void activate() {
        if (screen_ != MAIN_MENU) return;
        screen_ = (selection_ == PLAY) ? PLAYING : EXIT_REQUESTED;
    }
    void backToMenu() {
        if (screen_ == PLAYING) {
            screen_ = MAIN_MENU;
            selection_ = PLAY;
        }
    }
private:
    Screen screen_;
    Selection selection_;
};
}
#endif
