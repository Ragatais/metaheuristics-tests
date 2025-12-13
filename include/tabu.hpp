#pragma once
#include <vector>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <functional>
#include <random>
#include <cstdint>
#include "cvrp.hpp"
#include "greedy.hpp"    // greedy_split (wykorzystywane w evaluate_order)
#include "fitness.hpp"
#include "random_cvrp_solution.hpp" // do startu losowego

// parametry Tabu Search
struct TSParams {
    int max_iterations = 500;   // ile iteracji lokalnego przeszukania
    int tabu_tenure    = 10;    // ile iteracji ruch (swap) pozostaje tabu
    int neighborhood_cap = 0;   // 0 = pełne sąsiedztwo; >0 = losowo ogranicz do tylu ruchow/iteracji
    unsigned seed      = std::random_device{}();
};

//hash klucza ruchu,para ID klientow, nie indeksów!!
// klucz zawsze uporządkowany (min,max), żeby (a,b) == (b,a)
inline std::uint64_t tabu_key_for_ids(int a, int b) {
    if (a > b) std::swap(a, b);
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a)) << 32)
         |  static_cast<std::uint32_t>(b);
}

//pobranie permutacji klientów z instancji (bez depotu)
inline std::vector<int> make_random_order(const CVRPInstance& I, std::mt19937& g) {
    std::vector<int> order;
    order.reserve(I.nodes.size() - 1);
    for (const auto& n : I.nodes) if (n.id != I.depot_id) order.push_back(n.id);
    std::shuffle(order.begin(), order.end(), g);
    return order;
}

// Główny Tabu Search (swap(i,j) na permutacji klientów)
inline CVRPSolution run_tabu(const CVRPInstance& I,
                             const TSParams& P,
                             std::function<void(int,double,double)> on_iter = nullptr)
{
    std::mt19937 rng(P.seed);

    //inicjalizacja: losowa permutacja + ocena
    std::vector<int> curr_order = make_random_order(I, rng);
    double curr_cost = evaluate_order(I, curr_order);

    std::vector<int> best_order = curr_order;
    double best_cost = curr_cost;

    //tabu list" mapowanie "ruch swap(IDa,IDb)" -> pozostały czas tabu
    std::unordered_map<std::uint64_t, int> tabu_tenure;
    tabu_tenure.reserve(1024);

    //logowanie generacji 0 (stan początkowy)
    if (on_iter) on_iter(0, best_cost, curr_cost);

    const int n = static_cast<int>(curr_order.size());
    if (n < 2) {
        return greedy_split(I, best_order);
    }

    //lista wszystkich par (i,j) dla pełnego sąsiedztwa
    std::vector<std::pair<int,int>> all_pairs;
    all_pairs.reserve((n*(n-1))/2);
    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            all_pairs.emplace_back(i, j);

    std::uniform_int_distribution<int> uni_index(0, static_cast<int>(all_pairs.size()) - 1);

    //Główna pętla Tabu Search
    for (int iter = 1; iter <= P.max_iterations; ++iter) {

        //wybór sąsiedztwa: pełne lub ograniczone do losowych ruchów
        std::vector<std::pair<int,int>> neighborhood;
        if (P.neighborhood_cap <= 0 || P.neighborhood_cap >= (int)all_pairs.size()) {
            neighborhood = all_pairs; // pełne sąsiedztwo
        } else {
            // losowo wybieramy P.neighborhood_cap różnych ruchów
            neighborhood.clear();
            neighborhood.reserve(P.neighborhood_cap);
            //sampling bez powtórzeń (może być z kolizjami i retry)
            std::unordered_map<std::uint64_t, char> seen;
            seen.reserve(P.neighborhood_cap*2);
            while ((int)neighborhood.size() < P.neighborhood_cap) {
                auto pr = all_pairs[uni_index(rng)];
                std::uint64_t key = (static_cast<std::uint64_t>(pr.first) << 32) | static_cast<std::uint32_t>(pr.second);
                if (!seen.count(key)) {
                    seen[key] = 1;
                    neighborhood.push_back(pr);
                }
            }
        }

        //przeszukwianei sąsiedztwa, najlepszy dopuszczalny ruch (z aspiracją)
        double best_neighbor_cost = std::numeric_limits<double>::infinity();
        int best_i = -1, best_j = -1;
        bool best_was_tabu = false;

        //bufor do testów swapów
        std::vector<int> candidate = curr_order;

        for (const auto& mv : neighborhood) {
            int i = mv.first;
            int j = mv.second;

            // ruch opiywany parami ID (nie indeksów)
            int ida = curr_order[i];
            int idb = curr_order[j];
            std::uint64_t move_key = tabu_key_for_ids(ida, idb);

            // wykonujemy ruch (swap) na kopii
            candidate[i] = curr_order[j];
            candidate[j] = curr_order[i];

            double cand_cost = evaluate_order(I, candidate);

            // Czy ruch jest tabu?
            bool is_tabu = false;
            auto it = tabu_tenure.find(move_key);
            if (it != tabu_tenure.end() && it->second > 0) {
                is_tabu = true;
            }

            // Aspiracja: jeśli poprawia global best, wolno go przyjąć mimo tabu
            bool aspiration = (cand_cost < best_cost);

            // Kryterium wyboru najlepszego sąsiada:
            // - jeśli ruch nie jest tabu, porównuj normalnie
            // - jeśli jest tabu, rozważ tylko jeśli działa aspiracja
            if ((!is_tabu && cand_cost < best_neighbor_cost) ||
                (is_tabu && aspiration && cand_cost < best_neighbor_cost)) {
                best_neighbor_cost = cand_cost;
                best_i = i;
                best_j = j;
                best_was_tabu = is_tabu;
            }

            // cofnij tymczasowy swap
            candidate[i] = curr_order[i];
            candidate[j] = curr_order[j];
        }

        //jesli nie znaleziono żadnego ruchu zakończ
        if (best_i < 0) {
            if (on_iter) on_iter(iter, best_cost, curr_cost);
            break;
        }

        //zastosowanie wybranego ruchu do bieżącego rozwiązania
        std::swap(curr_order[best_i], curr_order[best_j]);
        curr_cost = best_neighbor_cost;

        // Aktualizuj najlepsze globalne rozwiązanie
        if (curr_cost < best_cost) {
            best_cost = curr_cost;
            best_order = curr_order;
        }

        // aktualizuj listę tabu:
        //     - dekrementuj istniejące wpisy
        //     - ustaw tabu dla wykonanego ruchu
        //     - wyczyść te, które wygasły
        for (auto it = tabu_tenure.begin(); it != tabu_tenure.end(); ) {
            it->second -= 1;
            if (it->second <= 0) {
                it = tabu_tenure.erase(it);
            } else {
                ++it;
            }
        }
        // ustaw tabu dla ruchu (po ID)
        {
            int ida = curr_order[best_i];
            int idb = curr_order[best_j];
            std::uint64_t move_key = tabu_key_for_ids(ida, idb);
            tabu_tenure[move_key] = P.tabu_tenure;
        }

        // ts_convergence
        if (on_iter) on_iter(iter, best_cost, curr_cost);

    }

    // zwracanie najelpszego rozwiazania po splicie
    return greedy_split(I, best_order);
}
