#include "../include/graph.hpp"
#include <sstream>
#include <iomanip>

Graph::Graph() : n(0), m(0), diameter_(0.0), normalizador_(0.0) {}

Graph::Graph(int n) : n(n), m(0), diameter_(0.0), normalizador_(0.0) {
    const double INF = std::numeric_limits<double>::infinity();
    adj.assign(n, std::vector<double>(n, INF));
    for (int i = 0; i < n; ++i)
        adj[i][i] = 0.0;
}

void Graph::add_edge(int u, int v, double w) {
    adj[u][v] = w;
    adj[v][u] = w;
    ++m;
}

double Graph::weight(int u, int v) const {
    return adj[u][v];
}

bool Graph::has_edge(int u, int v) const {
    return adj[u][v] != std::numeric_limits<double>::infinity();
}

void Graph::print() const {
    std::cout << "Grafo con " << n << " vértices y " << m << " aristas\n";
    std::cout << std::fixed << std::setprecision(2);
    for (int i = 0; i < n; ++i) {
        std::cout << i << ": ";
        for (int j = 0; j < n; ++j)
            if (adj[i][j] != std::numeric_limits<double>::infinity())
                std::cout << "(" << j << "," << adj[i][j] << ") ";
        std::cout << "\n";
    }
}

std::pair<std::string, double> Graph::prim(int start) const {
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<char> in_mst(n, 0);
    std::vector<double> min_edge(n, INF);
    std::vector<int> parent(n, -1);

    min_edge[start] = 0.0;
    double total = 0.0;
    std::string result;
    result.reserve(n * 10);

    for (int i = 0; i < n; ++i) {
        int u = -1;
        double best = INF;
        for (int v = 0; v < n; ++v)
            if (!in_mst[v] && min_edge[v] < best)
                best = min_edge[v], u = v;

        in_mst[u] = 1;
        total += min_edge[u];

        if (parent[u] != -1) {
            result += std::to_string(parent[u]) + "," +
                      std::to_string(u) + "," +
                      std::to_string(adj[parent[u]][u]) + ";";
        }

        const auto &row = adj[u];
        for (int v = 0; v < n; ++v) {
            double w = row[v];
            if (!in_mst[v] && w < min_edge[v]) {
                min_edge[v] = w;
                parent[v] = u;
            }
        }
    }

    return {result, total};
}

Graph::Matrix Graph::floyd_warshall() {
    const double INF = std::numeric_limits<double>::infinity();
    Matrix dist = adj;

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] < INF && dist[k][j] < INF) {
                    double new_dist = dist[i][k] + dist[k][j];
                    if (new_dist < dist[i][j]){
                        dist[i][j] = new_dist;
                        if (new_dist < dist[i][j])
                        dist[i][j] = new_dist;
                    }
                }

}

void Graph::complete() {
    const double INF = std::numeric_limits<double>::infinity();
    distances_ = floyd_warshall();

    for (int u = 0; u < n; ++u)
        for (int v = 0; v < n; ++v)
            if (u != v && adj[u][v] == INF)
                adj[u][v] = distances_[u][v] * diameter_;
}
