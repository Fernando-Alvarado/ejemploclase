#include "../include/graph_reader.hpp"

int main() {
    Graph g = GraphReader::from_file("./data/input.txt");

    auto [edges, total] = g.prim();
    std::cout << "MST: " << edges << "\n";
    std::cout << "Peso total: " << total << "\n";
}
