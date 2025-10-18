#pragma once
#include "cvrp.hpp"
#include <vector>
#include <limits>
#include <algorithm>

// greedy (Nearest Neighbor) dla CVRP
// Zwraca CVRPSolution: zestaw tras budowanych zachłannie od depotu
CVRPSolution greedy_nearest_neighbor(const CVRPInstance& I);

inline CVRPSolution greedy_nearest_neighbor(const CVRPInstance& I) {
    CVRPSolution S;
    S.routes.clear();
    S.order.clear(); // globalna kolejność odwiedzonych klientów

    const int depot_idx = I.index_of_id(I.depot_id);
    if (depot_idx < 0) throw std::runtime_error("Depot not found");

    const int N = static_cast<int>(I.nodes.size());
    if (N <= 1) return S; // tylko depot

    // czy klient byl odwiedzony
    std::vector<bool> visited(N, false);
    visited[depot_idx] = true;

    int unvisited_clients = 0;
    for (int i = 0; i < N; ++i) {
        if (i != depot_idx) ++unvisited_clients;
    }

    // Główna pętla budowanie tras
    while (unvisited_clients > 0) {
        std::vector<int> route;
        int load = 0;
        int current_idx = depot_idx;

        while (true) {
            double best_dist = std::numeric_limits<double>::infinity();
            int best_idx = -1;

            for (int i = 0; i < N; ++i) {
                if (i == depot_idx) continue;
                if (visited[i]) continue;
                int demand = I.nodes[i].demand;
                if (load + demand > I.capacity) continue;

                double d = euclidean(I.nodes[current_idx], I.nodes[i]);
                if (d < best_dist) {
                    best_dist = d;
                    best_idx = i;
                }
            }

            // Jeśli nic nie znaleziono koniec trasy
            if (best_idx == -1) break;

            route.push_back(I.nodes[best_idx].id);
            S.order.push_back(I.nodes[best_idx].id);
            visited[best_idx] = true;
            load += I.nodes[best_idx].demand;
            --unvisited_clients;

            current_idx = best_idx;
        }

        if (!route.empty()) {
            S.routes.push_back(std::move(route));
        }
    }

    return S;
}

inline CVRPSolution greedy_nearest_neighbor_from_start(const CVRPInstance& I, int start_client_id) {
    CVRPSolution S;
    const int depot_idx = I.index_of_id(I.depot_id);
    const int start_idx = I.index_of_id(start_client_id);
    if (depot_idx < 0 || start_idx < 0) throw std::runtime_error("Depot/start not found");

    const int N = (int)I.nodes.size();
    std::vector<bool> visited(N, false);
    visited[depot_idx] = true;

    int unvisited = 0;
    for (int i = 0; i < N; ++i) if (i != depot_idx) ++unvisited;

    while (unvisited > 0) {
        std::vector<int> route;
        int load = 0;
        int current_idx = depot_idx;

        //eśli start_client_id jeszcze nie był użyty i mieści się — wstaw go jako pierwszy
        if (!visited[start_idx] && (load + I.nodes[start_idx].demand <= I.capacity)) {
            route.push_back(I.nodes[start_idx].id);
            visited[start_idx] = true;
            load += I.nodes[start_idx].demand;
            --unvisited;
            current_idx = start_idx;
        }

        //NN
        while (true) {
            double best_dist = std::numeric_limits<double>::infinity();
            int best_idx = -1;
            for (int i = 0; i < N; ++i) {
                if (i == depot_idx || visited[i]) continue;
                if (load + I.nodes[i].demand > I.capacity) continue;
                double d = euclidean(I.nodes[current_idx], I.nodes[i]);
                if (d < best_dist) { best_dist = d; best_idx = i; }
            }
            if (best_idx == -1) break;
            route.push_back(I.nodes[best_idx].id);
            visited[best_idx] = true;
            load += I.nodes[best_idx].demand;
            --unvisited;
            current_idx = best_idx;
        }

        if (!route.empty()) S.routes.push_back(std::move(route));
        else break; // awaryjnie
    }
    return S;
}
