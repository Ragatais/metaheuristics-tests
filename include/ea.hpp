#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <functional>
#include "cvrp.hpp"
#include "greedy.hpp"   // greedy_split (split permutacji w trasy)
#include "fitness.hpp"  
#include "ea_logger.hpp"


struct EAParams {
    int pop_size    = 100;
    int generations = 100;
    double Px       = 0.7;   // prawd. krzyżowania
    double Pm       = 0.1;   // prawd. mutacji (na osobnika)
    int tournament  = 5;     // rozmiar turnieju
    int elitism     = 1;     // ilu najlepszych przenosimy bez zmian
    unsigned seed   = std::random_device{}();
};

// lista ID klientów (bez depotu)
inline std::vector<int> client_ids(const CVRPInstance& I) {
    std::vector<int> ids;
    ids.reserve(I.nodes.size() - 1);
    for (const auto& n : I.nodes) if (n.id != I.depot_id) ids.push_back(n.id);
    return ids;
}

// ocenianie permutacji: split -> fitness
inline double evaluate_order(const CVRPInstance& I, const std::vector<int>& order) {
    CVRPSolution s = greedy_split(I, order);
    return fitness_cvrp(I, s);
}

// inicjalizacja populacji
inline std::vector<std::vector<int>> init_population(const CVRPInstance& I, const EAParams& P, std::mt19937& g) {
    auto base = client_ids(I);
    std::vector<std::vector<int>> pop(P.pop_size, base);
    for (int i = 0; i < P.pop_size; ++i) {
        std::shuffle(pop[i].begin(), pop[i].end(), g);
    }
    return pop;
}

// selekcja: turniej (minimalizacja)
inline int tournament_select(const std::vector<double>& fit, int k, std::mt19937& g) {
    std::uniform_int_distribution<int> dist(0, (int)fit.size()-1);
    int best = dist(g);
    for (int i = 1; i < k; ++i) {
        int c = dist(g);
        if (fit[c] < fit[best]) best = c;
    }
    return best;
}

// krzyżowanie: OX (Ordered Crossover) -> 1 potomek
inline std::vector<int> crossover_OX(const std::vector<int>& A, const std::vector<int>& B, std::mt19937& g) {
    int n = (int)A.size();
    if (n == 0) return {};
    std::uniform_int_distribution<int> dist(0, n-1);
    int l = dist(g), r = dist(g);
    if (l > r) std::swap(l, r);

    std::vector<int> child(n, -1);
    for (int i = l; i <= r; ++i) child[i] = A[i];
    int pos = (r + 1) % n;
    for (int k = 0; k < n; ++k) { //sprawdzenie potomka
        int b = B[(r + 1 + k) % n];
        if (std::find(child.begin(), child.end(), b) == child.end()) {
            child[pos] = b;
            pos = (pos + 1) % n;
        }
    }
    return child;
}

// mutacja: inversion
inline void mutate_inversion(std::vector<int>& ord, std::mt19937& g) {
    if (ord.size() < 2) return;
    std::uniform_int_distribution<int> d(0, (int)ord.size()-1);
    int i = d(g), j = d(g);
    if (i > j) std::swap(i, j);
    std::reverse(ord.begin()+i, ord.begin()+j+1);
}

// główna pętla EA — zwraca najlepsze rozwiązanie podzielone na traasy
inline CVRPSolution run_EA(const CVRPInstance& I, const EAParams& P, const std::string& log_path = "") {
    std::mt19937 g(P.seed);
    std::uniform_real_distribution<double> ur(0.0, 1.0);

    // inicjalizacja logowania 
    if (!log_path.empty()) {
        ea_init_log(log_path);
    }

    auto pop = init_population(I, P, g);
    std::vector<double> fit(P.pop_size);

    auto eval_all = [&]() {
        for (int i = 0; i < P.pop_size; ++i) {
            fit[i] = evaluate_order(I, pop[i]);
        }
    };
    eval_all();

    // logowanie gen 0
    if (!log_path.empty()) {
        double best = fit[0], worst = fit[0], sum = 0;
        for (double v : fit) {
            if (v < best) best = v;
            if (v > worst) worst = v;
            sum += v;
        }
        double avg = sum / fit.size();
        ea_log_generation(log_path, 0, best, avg, worst);
    }

    // Glowna pętla EA
    for (int gen = 1; gen <= P.generations; ++gen) {
        // szukanie najlepszego soobnika
        int best_idx = 0;
        for (int i = 1; i < P.pop_size; ++i) {
            if (fit[i] < fit[best_idx]) best_idx = i;
        }

        // Następna populacja z elityzmem
        std::vector<std::vector<int>> next;
        next.reserve(P.pop_size);
        for (int e = 0; e < P.elitism && (int)next.size() < P.pop_size; ++e)
            next.push_back(pop[best_idx]);

        // Tworzenie potomków (selekcja, krzyżowanie, mutacja)
        while ((int)next.size() < P.pop_size) {
            int p1 = tournament_select(fit, std::max(1, P.tournament), g);
            int p2 = tournament_select(fit, std::max(1, P.tournament), g);

            std::vector<int> child = (ur(g) < P.Px)
                ? crossover_OX(pop[p1], pop[p2], g)
                : pop[p1];

            if (ur(g) < P.Pm) {
                mutate_inversion(child, g);
            }

            next.push_back(std::move(child));
        }

        pop.swap(next);
        eval_all();

        // ocena popoulacja i logowanie postępów
        if (!log_path.empty()) {
            double best = fit[0], worst = fit[0], sum = 0;
            for (double v : fit) {
                if (v < best) best = v;
                if (v > worst) worst = v;
                sum += v;
            }
            double avg = sum / fit.size();
            ea_log_generation(log_path, gen, best, avg, worst);
        }
    }

    // Najlepszy osobnik
    int best_idx = 0;
    for (int i = 1; i < P.pop_size; ++i) {
        if (fit[i] < fit[best_idx]) best_idx = i;
    }
    return greedy_split(I, pop[best_idx]);
}
