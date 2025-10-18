#pragma once
#include <string>
#include "cvrp.hpp"

// loader instancji problemu cvrp z pliku .vrp
class CVRPLoader {
public:
    static CVRPInstance load_from_file(const std::string& filepath);
};
