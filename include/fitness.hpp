#pragma once
#include "cvrp.hpp"

//   f(x) fitness, łączny ksozt tras


inline double fitness_cvrp(const CVRPInstance& instance, const CVRPSolution& solution) {
    return total_cost(instance, solution);
}
