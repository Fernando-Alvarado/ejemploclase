#include "../include/graph_reader.hpp"
#include <sstream>
#include <algorithm>

Graph GraphReader::from_file(const std::string& filename) {
    std::ifstream file(filename);
    return from_stream(file);
}

Graph GraphReader::from_stream(std::istream& input) {
    std::string content((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());

    // eliminar espacios y saltos de línea
    content.erase(std::remove_if(content.begin(), content.end(),
                  [](unsigned char c){ return std::isspace(c); }), content.end());

    // separar por ';'
    std::stringstream ss(content);
    std::string edge_str;
    std::vector<std::tuple<int,int,double>> edges;

    int max_vertex = -1;

    while (std::getline(ss, edge_str, ';')) {
        if (edge_str.empty()) continue;

        std::stringstream edge(edge_str);
        std::string u_str, v_str, w_str;

        std::getline(edge, u_str, ',');
        std::getline(edge, v_str, ',');
        std::getline(edge, w_str, ',');

        int u = std::stoi(u_str);
        int v = std::stoi(v_str);
        double w = std::stod(w_str);

        edges.emplace_back(u, v, w);
        if (u > max_vertex) max_vertex = u;
        if (v > max_vertex) max_vertex = v;
    }

    int n = max_vertex + 1;
    Graph g(n);

    for (auto [u, v, w] : edges)
        g.add_edge(u, v, w);

    return g;
}
