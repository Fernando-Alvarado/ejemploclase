#include "../include/graph_reader.hpp"
#include "../include/pso.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <vector>
#include <string>
#include <algorithm>

void print_usage(const char* prog_name) {
    std::cerr << "Uso:\n";
    std::cerr << "  Semilla única:        " << prog_name << " <file> <k> <swarm_size> <seed>\n";
    std::cerr << "  Conjunto de semillas: " << prog_name << " <file> <k> <swarm_size> <seed1> <seed2> ...\n";
    std::cerr << "  Intervalo de semillas:" << prog_name << " <file> <k> <swarm_size> <seed_inicio>-<seed_fin>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string input_path = argv[1];
    int k = std::atoi(argv[2]);
    int swarm_size = std::atoi(argv[3]);
    const int iterations = 10000;

    // --- Interpretar semillas ---
    std::vector<unsigned> seeds;
    if (argc == 4) {
        seeds.push_back(static_cast<unsigned>(std::time(nullptr)));
    } 
    else if (argc == 5 && std::string(argv[4]).find('-') != std::string::npos) {
        std::string range = argv[4];
        size_t dash_pos = range.find('-');
        unsigned start = std::stoi(range.substr(0, dash_pos));
        unsigned end   = std::stoi(range.substr(dash_pos + 1));
        for (unsigned s = start; s <= end; ++s) seeds.push_back(s);
    } 
    else {
        for (int i = 4; i < argc; ++i)
            seeds.push_back(std::stoi(argv[i]));
    }

    // --- Cargar grafo ---
    Graph g = GraphReader::from_file(input_path);
    std::cout << "Grafo cargado con " << g.num_vertices() << " vértices y "
              << g.num_edges() << " aristas.\n";

    g.calcula_Normalizador(k);
    g.complete(k);
    std::cout << "Normalizador: " << g.getNormalizador() << "\n";
    std::cout << "Diámetro: " << g.diameter() << "\n\n";

    std::cout << "Ejecutando " << seeds.size() << " corridas en paralelo...\n";

    // --- Variables compartidas para mejor global ---
    double global_best_value = std::numeric_limits<double>::infinity();
    unsigned global_best_seed = 0;
    std::vector<int> global_best_set;

    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < seeds.size(); ++i) {
        unsigned seed = seeds[i];
        int thread_id = omp_get_thread_num();

         #pragma omp critical
        std::cout << "[Hilo " << thread_id << "] ejecutando semilla " << seed << "\n";

        PSO solver(g, k, swarm_size, iterations, 0.6, 0.3, seed);
        solver.initialize();
        solver.run();

        double best_val = solver.best_value();
        std::vector<int> best_set = solver.best_set();

        // ✅ normalizar antes de guardar
        double normalized_val = best_val / g.getNormalizador();

        // Guardar resultado en archivo
        std::ofstream out("../kmst-" + std::to_string(seed) + ".mst");
        out << "# Resultados PSO - Semilla " << seed << "\n";
        out << "# Mejor conjunto (gbest): ";
        for (int v : best_set) out << "V" << v << " ";
        out << "\n# Peso total normalizado: " << normalized_val << "\n";  // ✅ normalizado
        out.close();

        {
            std::cout << "[Seed " << seed << "] terminado. Peso = "
                      << normalized_val << " → guardado en kmst-" << seed << ".mst\n";  // ✅ normalizado
            if (best_val < global_best_value) {
                global_best_value = best_val;
                global_best_seed = seed;
                global_best_set = best_set;
            }
        }
    }

    // --- Mostrar mejor global ---
    std::cout << "\n=== Mejor resultado global ===\n";
    std::cout << "Seed: " << global_best_seed << "\n";
    std::cout << "Conjunto: { ";
    for (int v : global_best_set) std::cout << "V" << v << " ";
    std::cout << "}\n";
    std::cout << "Peso total normalizado: " 
              << global_best_value / g.getNormalizador() << "\n";  // ✅ normalizado

    return 0;
}
