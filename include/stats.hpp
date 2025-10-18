#pragma once
#include <vector>
#include <cmath>
#include <limits>

struct Stats {
    double best = std::numeric_limits<double>::infinity();
    double worst = -std::numeric_limits<double>::infinity();
    double avg = 0.0;
    double std = 0.0;
    int runs = 0;
};

inline Stats compute_stats(const std::vector<double>& vals) {
    Stats s;
    if (vals.empty()) return s;
    s.runs = (int)vals.size();

    double sum = 0.0;
    s.best = vals[0];
    s.worst = vals[0];
    for (double v : vals) {
        if (v < s.best) s.best = v;
        if (v > s.worst) s.worst = v;
        sum += v;
    }
    s.avg = sum / vals.size();

    double sq = 0.0;
    for (double v : vals) {
        double d = v - s.avg;
        sq += d * d;
    }
    s.std = (vals.size() > 1) ? std::sqrt(sq / (vals.size() - 1)) : 0.0;
    return s;
}
