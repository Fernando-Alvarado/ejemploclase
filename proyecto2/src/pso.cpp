#include "../include/pso.hpp"
#include <numeric>
#include <unordered_set>
#include <algorithm>
#include <iostream>

PSO::PSO(const Graph& graph, int k, int swarm_size, int iterations,
         double alpha_g, double alpha_p, unsigned seed)
    : graph_(graph),
      k_(k),
      n_(graph.num_vertices()),
      swarm_size_(swarm_size),
      iterations_(iterations),
      //alpha_g_(alpha_g),
      //alpha_p_(alpha_p),
      swarm_(swarm_size),
      rng_(seed),
      dist01_(0.0, 1.0)
      
{
}


void PSO::initialize() {

    std::uniform_int_distribution<int> dist_vertex(0, n_ - 1);
    double best_global_value = std::numeric_limits<double>::infinity();
    std::vector<int> best_global_set;

    for (auto& p : swarm_.particles) {
        // Generar conjunto aleatorio único de k vértices
        std::unordered_set<int> used;
        p.current.clear();
        while ((int)p.current.size() < k_) {
            int v = dist_vertex(rng_);
            if (used.insert(v).second)
                p.current.push_back(v);
        }

        // Evaluar su MST
        double cost = evaluate(p.current) / graph_.getNormalizador();
        // std::cout << "[PSO] Partícula inicial: peso = " 
                //   << std::fixed << std::setprecision(15) 
                //   << cost << "\n";
// 
        // Inicializar mejor personal
        p.best = p.current;
        p.best_value = cost;
        p.current_value = cost;

        // Actualizar mejor global
        if (cost < best_global_value) {
            best_global_value = cost;
            best_global_set = p.current;
        } 
    }

    swarm_.gbest = best_global_set;
    swarm_.gbest_value = best_global_value;

    std::cout << "[PSO] Enjambre inicializado con " << swarm_size_ << " partículas.\n";
    std::cout << "[PSO] Mejor solución inicial: peso = " << best_global_value << "\n";
}

double PSO::evaluate(const std::vector<int>& vertices) const {
    if (vertices.empty())
        return std::numeric_limits<double>::infinity();

    auto [_, total] = graph_.prim_subset(vertices);
    return total;
}

/**
 * Transición discreta basada en 3 conjuntos:
 * A = gbest \ current  (vértices en mejor global que no están en current)
 * B = pbest \ current  (vértices en mejor personal que no están en current)
 * C = V \ (current ∪ gbest ∪ pbest)  (todos los demás vértices)
 * 
 * Con probabilidad alpha_g: tomar vértice de A y reemplazar uno aleatorio de current
 * Con probabilidad alpha_p: tomar vértice de B y reemplazar uno aleatorio de current
 * En caso contrario: tomar vértice de C y reemplazar uno aleatorio de current
 */
std::vector<int> PSO::transition(const Particle& p, double alpha_g_, double alpha_p_ ) const {
    // Crear conjunto actual como set para búsquedas rápidas
    std::unordered_set<int> current_set(p.current.begin(), p.current.end());
    
    // --- Calcular conjuntos A, B, C ---
    
    // A = gbest \ current
    std::vector<int> A;
    for (int v : swarm_.gbest) {
        if (current_set.find(v) == current_set.end())
            A.push_back(v);
    }
    
    // B = pbest \ current
    std::vector<int> B;
    for (int v : p.best) {
        if (current_set.find(v) == current_set.end())
            B.push_back(v);
    }
    
    // C = V \ (current ∪ gbest ∪ pbest)
    std::unordered_set<int> union_set = current_set;
    union_set.insert(swarm_.gbest.begin(), swarm_.gbest.end());
    union_set.insert(p.best.begin(), p.best.end());
    
    std::vector<int> C;
    for (int v = 0; v < n_; ++v) {
        if (union_set.find(v) == union_set.end())
            C.push_back(v);
    }
    
    // --- Aplicar transición ---
    std::vector<int> new_current = p.current;
    
    // Si no hay vértices para intercambiar, retornar current
    if (new_current.empty())
        return new_current;
    
    // Determinar de qué conjunto tomar el vértice
    double rand = dist01_(rng_);
    int vertex_to_add = -1;
    
    if (rand < alpha_g_ && !A.empty()) {
        // Tomar de A (influencia global)
        std::uniform_int_distribution<int> dist(0, A.size() - 1);
        vertex_to_add = A[dist(rng_)];
    } 
    else if (rand < alpha_g_ + alpha_p_ && !B.empty()) {
        // Tomar de B (influencia personal)
        std::uniform_int_distribution<int> dist(0, B.size() - 1);
        vertex_to_add = B[dist(rng_)];
    } 
    else if (!C.empty()) {
        // Tomar de C (exploración)
        std::uniform_int_distribution<int> dist(0, C.size() - 1);
        vertex_to_add = C[dist(rng_)];
    }
    
    // Si encontramos un vértice para agregar, reemplazar uno aleatorio
    if (vertex_to_add != -1) {
        std::uniform_int_distribution<int> dist_pos(0, new_current.size() - 1);
        int pos_to_replace = dist_pos(rng_);
        new_current[pos_to_replace] = vertex_to_add;
    }
    
    return new_current;
}

void PSO::run() {
    // parámetros iniciales y finales
    const double alpha_g_start = 0.2;
    const double alpha_g_end   = 0.6;
    const double alpha_p_start = 0.4;
    const double alpha_p_end   = 0.3;

    for (int iter = 0; iter < iterations_; ++iter) {
        double progress = static_cast<double>(iter) / iterations_;
        
        // α(t) lineal: de exploración → explotación
        double alpha_g_t = alpha_g_start + (alpha_g_end - alpha_g_start) * progress;
        double alpha_p_t = alpha_p_start + (alpha_p_end - alpha_p_start) * progress;

        bool improved = false;
        int improvements_count = 0;

        for (auto& p : swarm_.particles) {
            // Transición discreta con α(t)
            std::vector<int> new_pos = transition(p, alpha_g_t, alpha_p_t);
            double new_value = evaluate(new_pos) / graph_.getNormalizador();

            p.current = new_pos;
            p.current_value = new_value;

            if (new_value < p.best_value) {
                p.best = new_pos;
                p.best_value = new_value;
                improvements_count++;
                
                if (new_value < swarm_.gbest_value) {
                    swarm_.gbest = new_pos;
                    swarm_.gbest_value = new_value;
                    improved = true;
                }
            }
        }

        if (iter % 10 == 0 || improved) {
            std::cout << "[PSO] Iter " << std::setw(3) << iter 
                      << " | α_g=" << std::fixed << std::setprecision(2) << alpha_g_t
                      << " α_p=" << alpha_p_t
                      << " | Mejor: " << std::setprecision(10)
                      << swarm_.gbest_value
                      << " | Mejoras: " << improvements_count << "\n";
        }
    }

    std::cout << "\n[PSO] Optimización completada.\n";
    std::cout << "[PSO] Mejor solución: peso = " << swarm_.gbest_value << "\n";
}


std::vector<int> PSO::difference(const std::vector<int>& a, const std::vector<int>& b) {
    std::unordered_set<int> b_set(b.begin(), b.end());
    std::vector<int> result;
    
    for (int x : a)
        if (b_set.find(x) == b_set.end())
            result.push_back(x);
    
    return result;
}

std::vector<int> PSO::random_subset() const {
    std::uniform_int_distribution<int> dist_vertex(0, n_ - 1);
    std::unordered_set<int> used;
    std::vector<int> subset;
    
    while ((int)subset.size() < k_) {
        int v = dist_vertex(rng_);
        if (used.insert(v).second)
            subset.push_back(v);
    }
    
    return subset;
}