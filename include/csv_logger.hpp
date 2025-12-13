#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include "stats.hpp"

struct RowSummary {
    std::string instance;
    Stats random;       // [best worst avg std runs]
    Stats greedy;
    Stats ea;
    Stats tabu;
};

inline void csv_ensure_header(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::ofstream out(path, std::ios::out);
        out << "instance,"
            << "rand_best,rand_worst,rand_avg,rand_std,rand_runs,"
            << "greedy_best,greedy_worst,greedy_avg,greedy_std,greedy_runs,"
            << "ea_best,ea_worst,ea_avg,ea_std,ea_runs,"
            << "ts_best,ts_worst,ts_avg,ts_std,ts_runs\n";
    }
}

inline void csv_append_row(const std::string& path, const RowSummary& r) {
    std::ofstream out(path, std::ios::app);
out << r.instance << "," 
    << r.random.best  << "," << r.random.worst  << "," << r.random.avg  << "," << r.random.std  << "," << r.random.runs << ","
    << r.greedy.best  << "," << r.greedy.worst  << "," << r.greedy.avg  << "," << r.greedy.std  << "," << r.greedy.runs << ","
    << r.ea.best      << "," << r.ea.worst      << "," << r.ea.avg      << "," << r.ea.std      << "," << r.ea.runs << ","
    << r.tabu.best    << "," << r.tabu.worst    << "," << r.tabu.avg    << "," << r.tabu.std    << "," << r.tabu.runs << "\n";

}
