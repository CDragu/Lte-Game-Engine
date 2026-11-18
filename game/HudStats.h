#ifndef TOWER_HUD_STATS_H
#define TOWER_HUD_STATS_H

// Game-facing HUD data model: C++98, no PSP or LTE dependency.
// Add counters by registering a label and update them by returned index.
// Labels must outlive HudStats (wide string literals are ideal).
namespace tower {
class HudStats {
public:
    enum { MAX_METRICS = 4 };
    struct Metric {
        const wchar_t* label;
        int value;
    };
    HudStats() : count_(0) {}
    int addCounter(const wchar_t* label, int initialValue) {
        if (!label || count_ >= MAX_METRICS) return -1;
        metrics_[count_].label = label;
        metrics_[count_].value = initialValue;
        return count_++;
    }
    bool setValue(int id, int value) {
        if (id < 0 || id >= count_) return false;
        metrics_[id].value = value;
        return true;
    }
    int metricCount() const { return count_; }
    const Metric& metric(int id) const { return metrics_[id]; } // valid id only
private:
    Metric metrics_[MAX_METRICS];
    int count_;
};
}
#endif
