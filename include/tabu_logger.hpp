#pragma once
#include <fstream>
#include <string>
#include <filesystem>

// tworzenie TS_convergence
inline void tabu_init_log(const std::string& path) {
    std::ofstream out(path, std::ios::out); // nadpisanie
    out << "iteration;best;current\n";       // nowy nagłówek
}

// logowanie
inline void tabu_log_iteration(const std::string& path, int iteration, double best_cost, double current_cost) {
    std::ofstream out(path, std::ios::app);
    out << iteration << ";" << best_cost << ";" << current_cost << "\n";
}
