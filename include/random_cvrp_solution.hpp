#pragma once
#include "cvrp.hpp"
#include <random>

// losowy osobnik cVRP


CVRPSolution create_random_cvrp_solution(const CVRPInstance& instance);
