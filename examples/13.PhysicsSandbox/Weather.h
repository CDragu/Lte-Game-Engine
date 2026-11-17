#ifndef TOWER_WEATHER_H
#define TOWER_WEATHER_H

// Deterministic 2.5D wind model. Units are gameplay acceleration (X),
// not real-world mph. Positive means blowing to the right.
namespace tower {
class Weather {
public:
    enum Severity { CALM=0, BREEZE, WINDY, STRONG, GALE };
    Weather();
    void reset();
    void configure(float maxAcceleration,float gustiness,float cycleSeconds);
    void step(float dt); // advance ONLY on the fixed 1/60 s physics tick
    float accelerationX() const { return x_; }
    float changeRateX() const { return derivative_; }
    float maxAcceleration() const { return strength_; }
    int direction() const; // -1 left, 0 calm, +1 right
    float fraction() const; // normalized 0..1 strength for drawing
    Severity severity() const;
    static const wchar_t* severityLabel(Severity severity);
private:
    float time_,x_,derivative_;
    float strength_,gust_,period_;
    void recalculate(float dt);
};
}
#endif
