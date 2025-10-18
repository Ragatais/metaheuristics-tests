#pragma once
#include <vector>
#include <random>
#include "cvrp.hpp"
#include "random_cvrp_solution.hpp"
#include "greedy_cvrp.hpp"
#include "fitness.hpp"
#include "stats.hpp"
#include "ea.hpp"

// random uruchomia sie N razy (N = generations * pop_size)
inline Stats run_random_experiment(const CVRPInstance& I, int runs, unsigned seed = std::random_device{}()) {
    std::mt19937 g(seed);
    std::vector<double> vals;
    vals.reserve(runs);

    for (int i = 0; i < runs; ++i) {
        CVRPSolution s = create_random_cvrp_solution(I);
        vals.push_back(fitness_cvrp(I, s));
    }
    return compute_stats(vals);
}

// greedy, tyle uruchomień co klinetów, kazdy klient jako punkt startowy
inline Stats run_greedy_all_starts(const CVRPInstance& I) {
    std::vector<double> vals;
    vals.reserve(I.nodes.size()-1);
    for (const auto& n : I.nodes) {
        if (n.id == I.depot_id) continue;
        CVRPSolution s = greedy_nearest_neighbor_from_start(I, n.id);
        vals.push_back(fitness_cvrp(I, s));
    }
    return compute_stats(vals);
}

// ea uruchamia sie domyslnie 10 razy, statystyki i najlpsze rozwiazanie
struct EAExperimentResult {
    Stats stats;
    CVRPSolution best_solution;
    int best_run_index = 1; // numeracja routes od 1
};

inline EAExperimentResult run_ea_repeated(const CVRPInstance& I, const EAParams& P, int repeats) {
    std::vector<double> vals;
    vals.reserve(repeats);

    CVRPSolution best_solution;
    double best_cost = std::numeric_limits<double>::infinity();
    int best_run = 1;

    for (int r = 0; r < repeats; ++r) {
        EAParams P2 = P;
        P2.seed = P.seed + r;  // różne seedy dla różnych runów
        CVRPSolution candidate = run_EA(I, P2);  // jedno uruchomienie EA
        double cost = fitness_cvrp(I, candidate);
        vals.push_back(cost);

        if (cost < best_cost) {
            best_cost = cost;
            best_solution = candidate;
            best_run = r + 1; // numeracja od 1
        }
    }

    EAExperimentResult result;
    result.stats = compute_stats(vals);
    result.best_solution = best_solution;
    result.best_run_index = best_run;
    return result;
}
