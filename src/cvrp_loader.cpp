#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <cctype>
#include "cvrp_loader.hpp"

// trim z lewej/prawej
static inline void ltrim(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}
static inline void rtrim(std::string& s) {
    if (s.empty()) return;
    size_t i = s.size() - 1;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        if (i == 0) { s.clear(); return; }
        --i;
    }
    s.erase(i + 1);
}
static inline void trim(std::string& s) { rtrim(s); ltrim(s); }

CVRPInstance CVRPLoader::load_from_file(const std::string& filepath) {
    CVRPInstance instance;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filepath);
    }

    std::string line;

    //parsowanie nagłówka
    // przechodzimy do NODE_COORD_SECTION
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;

        if (line == "NODE_COORD_SECTION") {
            break;
        }

        if (line.rfind("NAME", 0) == 0) {
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                instance.name = line.substr(pos + 1);
                trim(instance.name);
            }
            continue;
        }
        if (line.rfind("CAPACITY", 0) == 0) {
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                std::string cap = line.substr(pos + 1);
                trim(cap);
                instance.capacity = std::stoi(cap);
            }
            continue;
        }


    }

    //id x, y
    // przechodzimy do DEMAND_SECTION
    instance.nodes.clear();
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line == "DEMAND_SECTION") {
            break;
        }

        std::istringstream iss(line);
        int id; double x, y;
        if (!(iss >> id >> x >> y)) {
            throw std::runtime_error("Bledny wiersz w NODE_COORD_SECTION: " + line);
        }
        VRPNode node;
        node.id = id;
        node.x = x;
        node.y = y;
        node.demand = 0; // uzupełnimy w DEMAND_SECTION
        instance.nodes.push_back(node);
    }

    // mapowanie id->index dla szybszego ustawiania popytów
    std::unordered_map<int, int> id2idx;
    id2idx.reserve(instance.nodes.size());
    for (int i = 0; i < (int)instance.nodes.size(); ++i) {
        id2idx[instance.nodes[i].id] = i;
    }

    // do DEPOT_SECTION
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line == "DEPOT_SECTION") {
            break;
        }

        std::istringstream iss(line);
        int id, d;
        if (!(iss >> id >> d)) {
            throw std::runtime_error("Bledny wiersz w DEMAND_SECTION: " + line);
        }
        auto it = id2idx.find(id);
        if (it == id2idx.end()) {
            throw std::runtime_error("DEMAND_SECTION odwoluje sie do nieznanego ID: " + std::to_string(id));
        }
        instance.nodes[it->second].demand = d;
    }

    //DEPOT_SECTION (id), pierwsze id jako depot

    bool depot_set = false;
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line == "EOF") break;

        int val = 0;
        {
            std::istringstream iss(line);
            if (!(iss >> val)) {
                throw std::runtime_error("Bledny wiersz w DEPOT_SECTION: " + line);
            }
        }
        if (val == -1) break;
        if (!depot_set) {
            instance.depot_id = val;
            depot_set = true;
        }
    }

    // walidacjae
    if (instance.nodes.empty()) {
        throw std::runtime_error("Brak wierzcholkow w NODE_COORD_SECTION.");
    }
    if (instance.capacity <= 0) {
        throw std::runtime_error("CAPACITY niepoprawne lub brak CAPACITY w naglowku.");
    }
    if (!depot_set) {
        // Jesli w DEPOT_SECTION nie ustawiono — ustaw 1
        instance.depot_id = 1;
    }

    // czy depot istnieje w nodes
    if (instance.index_of_id(instance.depot_id) < 0) {
        throw std::runtime_error("DEPOT_SECTION wskazuje ID, ktore nie istnieje w NODE_COORD_SECTION.");
    }

    return instance;
}
