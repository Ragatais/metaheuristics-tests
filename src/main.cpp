#include <windows.h>
#include <iostream>
#include "cvrp_loader.hpp"
#include "experiments.hpp"
#include "csv_logger.hpp"
#include "tabu.hpp"
#include "tabu_logger.hpp"


int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        //const std::string instance_path = "../data/toy.vrp";
        //const std::string instance_name = "toy";

        //INSTANCJE PODSTAWOWE

        //const std::string instance_path = "../data/A-n32-k5.vrp";  
        //const std::string instance_name = "A-n32-k5";

        //const std::string instance_path = "../data/A-n37-k6.vrp";  
        //const std::string instance_name = "A-n37-k6";      
        
        //const std::string instance_path = "../data/A-n39-k5.vrp";  
        //const std::string instance_name = "A-n39-k5"; 

        //const std::string instance_path = "../data/A-n45-k6.vrp";  
        //const std::string instance_name = "A-n45-k6"; 

        //const std::string instance_path = "../data/A-n48-k7.vrp";  
        //const std::string instance_name = "A-n48-k7"; 

        //INSTANCJE TRUDNE

        //const std::string instance_path = "../data/A-n54-k7.vrp";  
        //const std::string instance_name = "A-n54-k7";
        
        const std::string instance_path = "../data/A-n60-k9.vrp";  
        const std::string instance_name = "A-n60-k9"; 

        CVRPInstance inst = CVRPLoader::load_from_file(instance_path);

        //parametry EA
        EAParams P;
        P.pop_size = 100;
        P.generations = 300;
        P.Px = 0.7;
        P.Pm = 0.4;
        P.tournament = 5;
        P.elitism = 1;

        std::string log_path = "../results/" + instance_name + "_convergence.csv";
        CVRPSolution ea_best_convergence = run_EA(inst, P, log_path);

        //TEABU SEARCH
        TSParams T;
        T.max_iterations = 3;     // możesz zmieniać
        T.tabu_tenure    = 10;
        //T.neighborhood_cap = P.pop_size*(P.generations+1)/T.max_iterations;
        T.seed           = 123;     // możesz podmienić

        std::string tabu_log_path = "../results/" + instance_name + "_tabu_convergence.csv";
        tabu_init_log(tabu_log_path);

        std::cout << "Logowanie przebiegu Tabu Search do: " << tabu_log_path << "\n";

        CVRPSolution tabu_best_convergence = run_tabu(inst, T, [&](int it, double best, double curr){
             tabu_log_iteration(tabu_log_path, it, best, curr);
        });

        
        const int random_runs = P.pop_size * P.generations;
        const int ea_repeats  = 10;                         // ILE uruchomień ea

        Stats s_random = run_random_experiment(inst, random_runs);
        Stats s_greedy = run_greedy_all_starts(inst);
        EAExperimentResult ea_result = run_ea_repeated(inst, P, ea_repeats);
        Stats s_ea = ea_result.stats;  // statystyki do csv
        TSExperimentResult ts_result = run_tabu_repeated(inst, T, 10); // 10 runów TS
        Stats s_ts = ts_result.stats;


        //zapis do scv
        const std::string csv_path = "../results/" + instance_name + "_results.csv";
        csv_ensure_header(csv_path);

        RowSummary row;
        row.instance   = instance_name;
        row.random     = s_random;
        row.greedy     = s_greedy;
        row.ea         = s_ea;
        row.tabu       = s_ts;


        csv_append_row(csv_path, row);

        std::cout << "Random  [runs=" << s_random.runs << "] best=" << s_random.best
                  << " worst=" << s_random.worst << " avg=" << s_random.avg << " std=" << s_random.std << "\n";
        std::cout << "Greedy  [runs=" << s_greedy.runs << "] best=" << s_greedy.best
                  << " worst=" << s_greedy.worst << " avg=" << s_greedy.avg << " std=" << s_greedy.std << "\n";
        std::cout << "EA      [runs=" << s_ea.runs << "] best=" << s_ea.best
                  << " worst=" << s_ea.worst << " avg=" << s_ea.avg << " std=" << s_ea.std << "\n";
        std::cout << "TS      [runs=" << s_ts.runs << "] best=" << s_ts.best
                  << " worst=" << s_ts.worst << " avg=" << s_ts.avg << " std=" << s_ts.std << "\n";


        // wypiszanie najelpzej trasy ea
        std::cout << "\nEA – najlepsze rozwiązanie (run " << ea_result.best_run_index << "):\n";
        for (const auto& route : ea_result.best_solution.routes) {
            std::cout << "Route: ";
            for (int id : route) std::cout << id << " ";
            std::cout << "\n";
        }
        
        //wyspiasnie najlepiszej trasy TS
        std::cout << "\nTS – najlepsze rozwiązanie (run " << ts_result.best_run_index << "):\n";
        for (const auto& route : ts_result.best_solution.routes) {
            std::cout << "Route: ";
            for (int id : route) std::cout << id << " ";
            std::cout << "\n";
        }


    } catch (const std::exception& ex) {
        std::cerr << "Błąd: " << ex.what() << "\n";
    }
    return 0;
}
