#pragma once
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

inline void ea_init_log(const std::string& path) {
    std::ofstream out(path, std::ios::out);  // bez sprawdzania istnienia
    out << "generation;best;avg;worst\n";
}


inline void ea_log_generation(const std::string& path, int generation, double best, double avg, double worst) {
    std::ofstream out(path, std::ios::app);
    out << generation << ";" << best << ";" << avg << ";" << worst << "\n";
}
