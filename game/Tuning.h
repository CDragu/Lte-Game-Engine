#ifndef TOWER_TUNING_H
#define TOWER_TUNING_H

// PSP-independent, fixed-capacity gameplay tuning model.
// A UTF-8/ASCII text file is the same read/write source at runtime.
namespace tower {

class Tuning {
public:
    enum Field {
        GRAVITY=0, FRICTION, RESTITUTION, LINEAR_DAMPING, ANGULAR_DAMPING,
        SWING_AMPLITUDE, SWING_SPEED, SETTLING_TIME, SCORE_PER_BLOCK,
        WIND_STRENGTH, WIND_GUSTINESS, WIND_CYCLE, WIND_DROPS, FIELD_COUNT
    };
    struct Entry {
        const char* key;
        const wchar_t* label;
        float value, minimum, maximum, step;
        bool integer;
    };

    Tuning();
    const Entry& entry(int i) const { return items_[i]; } // i in [0, FIELD_COUNT)
    float get(Field field) const { return items_[(int)field].value; }
    int getInt(Field field) const { return (int)(items_[(int)field].value+0.5f); }
    bool set(Field field, float value);
    bool adjust(int index, int direction);
    bool load(const char* path);
    bool save(const char* path) const;

private:
    Entry items_[FIELD_COUNT];
    void add(Field field, const char* key, const wchar_t* label,
             float initial, float minValue, float maxValue, float step, bool integer);
};

class TuningEditor {
public:
    TuningEditor() : selected_(0) {}
    int selected() const { return selected_; }
    void up() { if (selected_>0) --selected_; }
    void down() { if (selected_<Tuning::FIELD_COUNT-1) ++selected_; }
    bool change(Tuning& tuning,int direction) {
        return tuning.adjust(selected_,direction);
    }
private:
    int selected_;
};
}
#endif
