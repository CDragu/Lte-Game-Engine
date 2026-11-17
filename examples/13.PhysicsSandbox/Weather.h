#ifndef TOWER_WEATHER_H
#define TOWER_WEATHER_H

// Deterministic 2.5D crosswind. Positive means blowing right.
// The wind's underlying direction/strength changes only when the configured
// number of actual block releases has occurred. Small gusts still fluctuate.
namespace tower {
class Weather {
public:
    enum Severity { CALM=0, BREEZE, WINDY, STRONG, GALE };
    Weather();
    void reset();
    void configure(float maxAcceleration,float gustiness,float gustPeriodSeconds);
    void configureDrops(int dropsPerChange);
    bool onBlockDropped(); // call once, only after successfully spawning a released body
    void step(float dt); // fixed 1/60 s physics step
    float accelerationX() const { return x_; }
    float changeRateX() const { return derivative_; }
    float maxAcceleration() const { return strength_; }
    int direction() const; // -1 left, 0 calm, +1 right
    float fraction() const; // normalized 0..1 strength for drawing
    Severity severity() const;
    int dropsPerChange() const { return dropsPerChange_; }
    int dropsUntilChange() const { return dropsPerChange_-dropsSinceChange_; }
    int totalDrops() const { return totalDrops_; }
    int weatherIndex() const { return weatherIndex_; }
    static const wchar_t* severityLabel(Severity severity);
private:
    float gustTime_,x_,derivative_;
    float strength_,gust_,period_;
    float fromBase_,toBase_,base_,transition_;
    int dropsPerChange_,dropsSinceChange_,totalDrops_,weatherIndex_;
    void recalculate(float dt);
};
}
#endif
