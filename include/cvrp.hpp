#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <algorithm>

// model cvrp

// ojedynczy node (klient lub depot)
struct VRPNode {
    int id{};       
    double x{};     
    double y{};     
    int demand{};   // 0 dla depotu
};

// Główna instancja problemu
struct CVRPInstance {
    std::string name;          // nazwa instancji (z pliku)
    int capacity{};            // pojemność pojazdu
    int depot_id{1};           // ID depotu zwykle 1
    std::vector<VRPNode> nodes; // lista wszystkich węzłów

    // znajdowanie indeks (w wektorze nodes) na podstawie ID
    int index_of_id(int id) const {
        for (size_t i = 0; i < nodes.size(); ++i)
            if (nodes[i].id == id) return static_cast<int>(i);
        return -1;
    }
};

// Struktura rozwiązania CVRP
struct CVRPSolution {
    std::vector<int> order;                  // permutacja klientów (bez depotu)
    std::vector<std::vector<int>> routes;    // trasy po splicie
};


// odl euklidesowa między dwoma węzłami
inline double euclidean(const VRPNode& a, const VRPNode& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

// dlugosc pojedynczej trasy (powrót do depotu)
inline double route_length(const CVRPInstance& I, const std::vector<int>& route_ids) {
    int depot_idx = I.index_of_id(I.depot_id);
    if (depot_idx < 0) throw std::runtime_error("Depot not found");

    const VRPNode& depot = I.nodes[depot_idx];
    double sum = 0.0;
    VRPNode prev = depot;

    for (int id : route_ids) {
        int idx = I.index_of_id(id);
        sum += euclidean(prev, I.nodes[idx]);
        prev = I.nodes[idx];
    }

    // Powrot do depotu
    sum += euclidean(prev, depot);
    return sum;
}

// Całkowity koszt rozwiązania funkcja fitness
inline double total_cost(const CVRPInstance& I, const CVRPSolution& S) {
    double sum = 0.0;
    for (const auto& r : S.routes) sum += route_length(I, r);
    return sum;
}
