#include "../include/graph.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <charconv>

Graph::Graph() : n(0), m(0), diameter_(0.0), normalizador_(0.0) {}

Graph::Graph(int n) : n(n), m(0), diameter_(0.0), normalizador_(0.0) {
    const double INF = std::numeric_limits<double>::infinity();
    adj.assign(n, std::vector<double>(n, INF));
    for (int i = 0; i < n; ++i)
        adj[i][i] = 0.0;
    id_to_vertex.resize(n);
}

int Graph::get_or_create_vertex(const std::string& name) {
    auto it = vertex_to_id.find(name);
    if (it != vertex_to_id.end()) {
        return it->second;
    }
    
    int id = n++;
    vertex_to_id[name] = id;
    id_to_vertex.push_back(name);
    
    const double INF = std::numeric_limits<double>::infinity();
    for (auto& row : adj) {
        row.push_back(INF);
    }
    adj.push_back(std::vector<double>(n, INF));
    adj[id][id] = 0.0;
    
    return id;
}

std::string Graph::get_vertex_name(int id) const {
    if (id >= 0 && id < n)
        return id_to_vertex[id];
    return "";
}

int Graph::get_vertex_id(const std::string& name) const {
    auto it = vertex_to_id.find(name);
    if (it != vertex_to_id.end())
        return it->second;
    return -1;
}

void Graph::add_edge(const std::string& u, const std::string& v, double w) {
    int uid = get_or_create_vertex(u);
    int vid = get_or_create_vertex(v);
    
    adj[uid][vid] = w;
    adj[vid][uid] = w;
    ++m;
}

double Graph::weight(const std::string& u, const std::string& v) const {
    int uid = get_vertex_id(u);
    int vid = get_vertex_id(v);
    if (uid == -1 || vid == -1)
        return std::numeric_limits<double>::infinity();
    return adj[uid][vid];
}

bool Graph::has_edge(const std::string& u, const std::string& v) const {
    int uid = get_vertex_id(u);
    int vid = get_vertex_id(v);
    if (uid == -1 || vid == -1)
        return false;
    return adj[uid][vid] != std::numeric_limits<double>::infinity();
}

void Graph::print() const {
    std::cout << "Grafo con " << n << " vértices y " << m << " aristas\n";
    std::cout << std::fixed << std::setprecision(2);
    for (int i = 0; i < n; ++i) {
        std::cout << id_to_vertex[i] << ": ";
        for (int j = 0; j < n; ++j)
            if (adj[i][j] != std::numeric_limits<double>::infinity() && i != j)
                std::cout << "(" << id_to_vertex[j] << "," << adj[i][j] << ") ";
        std::cout << "\n";
    }
}

std::pair<std::string, double> Graph::prim(const std::string& start) const {
    const double INF = std::numeric_limits<double>::infinity();

    int start_id = 0;
    if (!start.empty()) {
        start_id = get_vertex_id(start);
        if (start_id == -1) start_id = 0;
    }

    using Edge = std::pair<double, int>;
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;

    std::vector<char> in_mst(n, 0);
    std::vector<int> parent(n, -1);
    std::vector<double> min_edge(n, INF);

    min_edge[start_id] = 0.0;
    pq.push({0.0, start_id});

    double total = 0.0;

    while (!pq.empty()) {
        auto [cost, u] = pq.top();
        pq.pop();

        if (in_mst[u]) continue;

        in_mst[u] = 1;
        total += cost;

        const auto& row = adj[u];
        for (int v = 0; v < n; ++v) {
            double w = row[v];
            if (!in_mst[v] && w < min_edge[v]) {
                min_edge[v] = w;
                parent[v] = u;
                pq.push({w, v});
            }
        }
    }

    return {"", total};  
}

Graph::Matrix Graph::floyd_warshall() {
    const double INF = std::numeric_limits<double>::infinity();
    Matrix dist = adj;

    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (dist[i][k] < INF) {  
                for (int j = 0; j < n; ++j) {
                    if (dist[k][j] < INF) {
                        double new_dist = dist[i][k] + dist[k][j];
                        if (new_dist < dist[i][j])
                            dist[i][j] = new_dist;
                    }
                }
            }
        }
    }


    diameter_ = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)  
            if (dist[i][j] < INF && dist[i][j] > diameter_)
                diameter_ = dist[i][j];

    return dist;
}

void Graph::complete(int k) {
    const double INF = std::numeric_limits<double>::infinity();
    distances_ = floyd_warshall();
    
    const double factor = diameter_ * k;

    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) { 
            if (adj[u][v] == INF) {
                double new_weight = distances_[u][v] * factor;
                adj[u][v] = new_weight;
                adj[v][u] = new_weight;  
            }
        }
    }
}

void Graph::calcula_Normalizador(int k) {
    const double INF = std::numeric_limits<double>::infinity();
    
    std::vector<double> pesos;
    pesos.reserve((n * (n - 1)) / 2); 
    
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (adj[i][j] < INF)
                pesos.push_back(adj[i][j]);
    
    if (pesos.empty()) {
        normalizador_ = 1.0;
        return;
    }
    
    int limit = std::min(k - 1, static_cast<int>(pesos.size()));
    
    if (limit <= 0) {
        normalizador_ = 0.0;
        return;
    }
    
    std::nth_element(pesos.begin(), 
                     pesos.end() - limit, 
                     pesos.end());

    double sum = 0.0;
    for (size_t i = pesos.size() - limit; i < pesos.size(); ++i)
        sum += pesos[i];
    
    std::cout << "Arista mas pesada original: " 
              << *std::max_element(pesos.end() - limit, pesos.end()) << "\n";
    
    normalizador_ = sum;
}


std::pair<std::vector<int>, double> Graph::prim_subset_full(const std::vector<int>& vertex_subset) const {
    const double INF = std::numeric_limits<double>::infinity();
    
    if (vertex_subset.empty())
        return {{}, 0.0};

    int k = vertex_subset.size();
    
    using Edge = std::pair<double, int>;
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;

    std::vector<char> in_mst(k, 0);
    std::vector<int> parent(k, -1);
    std::vector<double> min_edge(k, INF);

    min_edge[0] = 0.0;
    pq.push({0.0, 0});

    double total = 0.0;
    int added = 0;

    while (!pq.empty() && added < k) {
        auto [cost, u_local] = pq.top();
        pq.pop();

        if (in_mst[u_local]) continue;

        in_mst[u_local] = 1;
        total += cost;
        added++;

        int u_global = vertex_subset[u_local];
        for (int v_local = 0; v_local < k; ++v_local) {
            if (!in_mst[v_local]) {
                int v_global = vertex_subset[v_local];
                double w = adj[u_global][v_global];
                if (w < min_edge[v_local]) {
                    min_edge[v_local] = w;
                    parent[v_local] = u_local;
                    pq.push({w, v_local});
                }
            }
        }
    }

    return {parent, total};
}

std::string Graph::mst_to_string(const std::vector<int>& vertex_subset, 
                                  const std::vector<int>& parent) const {
    std::string result;
    int k = vertex_subset.size();
    result.reserve(k * 40);
    
    char buffer[32];
    
    for (int i = 0; i < k; ++i) {
        if (parent[i] != -1) {
            int parent_global = vertex_subset[parent[i]];
            int child_global = vertex_subset[i];
            
            result += id_to_vertex[parent_global];
            result += ',';
            result += id_to_vertex[child_global];
            result += ',';
            
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), 
                                          adj[parent_global][child_global]);
            result.append(buffer, ptr - buffer);
            result += ';';
        }
    }
    
    return result;
}
