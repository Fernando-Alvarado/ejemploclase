#include "../include/graph.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

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
    
    // Crear nuevo vértice
    int id = n++;
    vertex_to_id[name] = id;
    id_to_vertex.push_back(name);
    
    // Expandir matriz de adyacencia
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

    std::vector<char> in_mst(n, 0);
    std::vector<double> min_edge(n, INF);
    std::vector<int> parent(n, -1);

    min_edge[start_id] = 0.0;
    double total = 0.0;
    std::string result;
    result.reserve(n * 30);

    for (int i = 0; i < n; ++i) {
        int u = -1;
        double best = INF;
        for (int v = 0; v < n; ++v)
            if (!in_mst[v] && min_edge[v] < best)
                best = min_edge[v], u = v;

        in_mst[u] = 1;
        total += min_edge[u];

        if (parent[u] != -1) {
            result += id_to_vertex[parent[u]] + "," +
                      id_to_vertex[u] + "," +
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

std::pair<std::string, double> Graph::prim_subset(const std::vector<int>& vertex_subset) const {
    const double INF = std::numeric_limits<double>::infinity();
    
    if (vertex_subset.empty())
        return {"", 0.0};

    int k = vertex_subset.size();
    
    // Crear mapeo de ID global -> índice local
    std::unordered_map<int, int> global_to_local;
    for (int i = 0; i < k; ++i)
        global_to_local[vertex_subset[i]] = i;

    // Estructuras de Prim sobre el subconjunto
    std::vector<char> in_mst(k, 0);
    std::vector<double> min_edge(k, INF);
    std::vector<int> parent(k, -1);

    min_edge[0] = 0.0;
    double total = 0.0;
    std::string result;
    result.reserve(k * 30);

    for (int i = 0; i < k; ++i) {
        // Encontrar vértice no visitado con menor costo
        int u_local = -1;
        double best = INF;
        for (int v_local = 0; v_local < k; ++v_local) {
            if (!in_mst[v_local] && min_edge[v_local] < best) {
                best = min_edge[v_local];
                u_local = v_local;
            }
        }

        if (u_local == -1) break; // subgrafo desconectado

        in_mst[u_local] = 1;
        total += min_edge[u_local];

        // Agregar arista al resultado
        if (parent[u_local] != -1) {
            int parent_global = vertex_subset[parent[u_local]];
            int u_global = vertex_subset[u_local];
            
            result += id_to_vertex[parent_global] + "," +
                      id_to_vertex[u_global] + "," +
                      std::to_string(adj[parent_global][u_global]) + ";";
        }

        // Actualizar costos mínimos
        int u_global = vertex_subset[u_local];
        for (int v_local = 0; v_local < k; ++v_local) {
            if (!in_mst[v_local]) {
                int v_global = vertex_subset[v_local];
                double w = adj[u_global][v_global];
                if (w < min_edge[v_local]) {
                    min_edge[v_local] = w;
                    parent[v_local] = u_local;
                }
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
                    if (new_dist < dist[i][j])
                        dist[i][j] = new_dist;
                }

    // Calcular el diámetro (máxima distancia entre todos los pares)
    diameter_ = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (dist[i][j] < INF && dist[i][j] > diameter_)
                diameter_ = dist[i][j];

    return dist;
}

void Graph::complete(int k) {
    const double INF = std::numeric_limits<double>::infinity();
    distances_ = floyd_warshall();
    // double mas_pesado = 0.0;
    //double menos_pesado = std::numeric_limits<double>::infinity();

    for (int u = 0; u < n; ++u)
        for (int v = 0; v < n; ++v)
            if (u != v && adj[u][v] == INF){
                adj[u][v] = distances_[u][v] * diameter_ * k;
                //std::cout << "arista agregada: " << adj[u][v] << "\n";
                //if (adj[u][v] > mas_pesado)
                //    mas_pesado = adj[u][v];
                //else if(adj[u][v] < menos_pesado)
                //    menos_pesado = adj[u][v];

            
            }

    //std::cout << "arista mas pesada agregada: " << mas_pesado << "\n";
    //std::cout << "arista menos pesada agregada: " << menos_pesado << "\n";
}

void Graph::calcula_Normalizador(int k) {
    std::vector<double> pesos;
    pesos.reserve(n * n);
    
    const double INF = std::numeric_limits<double>::infinity();
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (adj[i][j] < INF)
                pesos.push_back(adj[i][j]);
    
    // Ordenar descendentemente
    std::sort(pesos.rbegin(), pesos.rend());
    
    // Sumar los k-1 mayores (asumiendo k = n para MST)
    double sum = 0.0;
    //int limit = std::min(n - 1, static_cast<int>(pesos.size()));
    for (int i = 0; i < k - 1; ++i){
        sum += pesos[i];
        //std::cout << "peso " << i << ": " << pesos[i] << "\n";
    }
    std::cout << "Arista mas pesada original: " << pesos[0] << "\n";
    
    normalizador_ = sum;
}