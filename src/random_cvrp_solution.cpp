#include "random_cvrp_solution.hpp"
#include "greedy.hpp"  // split
#include <algorithm>   // std::shuffle
#include <random>

CVRPSolution create_random_cvrp_solution(const CVRPInstance& instance) {
    CVRPSolution sol;

    //lista klientów (bez depotu)
    for (const auto& node : instance.nodes) {
        if (node.id != instance.depot_id) {
            sol.order.push_back(node.id);
        }
    }

    //losowa permutacja klientów
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(sol.order.begin(), sol.order.end(), gen);

    //split trasy wg pojemnosci
    sol = greedy_split(instance, sol.order);

    return sol;
}
