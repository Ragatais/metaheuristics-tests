#pragma once
#include "cvrp.hpp"

// tylko funkcja split do losowego

inline CVRPSolution greedy_split(const CVRPInstance& I, const std::vector<int>& order) {
    CVRPSolution sol;
    sol.order = order;   // zapamiętaj permutację klientów
    sol.routes.clear();

    std::vector<int> current_route;
    int load = 0;

    for (int id : order) {
        int idx = I.index_of_id(id);
        int demand = I.nodes[idx].demand;

        if (load + demand <= I.capacity) {
            current_route.push_back(id);
            load += demand;
        } else {
            sol.routes.push_back(current_route);
            current_route.clear();
            load = demand;
            current_route.push_back(id);
        }
    }

    // dodaj ostatnią trasę
    if (!current_route.empty())
        sol.routes.push_back(current_route);

    return sol;
}
