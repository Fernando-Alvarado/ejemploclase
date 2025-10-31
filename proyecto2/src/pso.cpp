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
      alpha_g_(alpha_g),
      alpha_p_(alpha_p),
      swarm_(swarm_size),
      rng_(seed),
      dist01_(0.0, 1.0)
{
    // Pre-alocar memoria para evitar realocaciones
    swarm_.particles.reserve(swarm_size);
    swarm_.gbest.reserve(k);
}

void PSO::initialize() {
    std::uniform_int_distribution<int> dist_vertex(0, n_ - 1);
    double best_global_value = std::numeric_limits<double>::infinity();
    std::vector<int> best_global_set;
    best_global_set.reserve(k_);

    for (auto& p : swarm_.particles) {
        // Generar conjunto aleatorio único de k vértices
        std::unordered_set<int> used;
        used.reserve(k_);
        p.current.clear();
        p.current.reserve(k_);
        
        while ((int)p.current.size() < k_) {
            int v = dist_vertex(rng_);
            if (used.insert(v).second)
                p.current.push_back(v);
        }

        // Evaluar su MST (sin normalizar todavía)
        double cost = evaluate(p.current);
        
        // Inicializar mejor personal
        p.best = p.current;
        p.best.reserve(k_);
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
    std::cout << "[PSO] Mejor solución inicial: peso = " 
              << best_global_value / graph_.getNormalizador() << "\n";
}

// ============================================================================
// OPTIMIZACIÓN 1: Evaluar sin construir strings - SOLO PESO
// ============================================================================
double PSO::evaluate(const std::vector<int>& vertices) const {
    if (vertices.empty())
        return std::numeric_limits<double>::infinity();

    return graph_.prim_subset(vertices);
}

// ============================================================================
// OPTIMIZACIÓN 2: Transition con cache thread_local y early exits
// ============================================================================
std::vector<int> PSO::transition(const Particle& p) const {
    // OPTIMIZACIÓN: Cache thread-local para evitar allocaciones repetidas
    thread_local std::unordered_set<int> current_set;
    thread_local std::vector<int> candidates;
    
    current_set.clear();
    current_set.insert(p.current.begin(), p.current.end());
    
    candidates.clear();
    
    // Determinar de qué conjunto tomar el vértice
    double rand = dist01_(rng_);
    
    if (rand < alpha_g_) {
        // Tomar de A: gbest \ current
        for (int v : swarm_.gbest) {
            if (!current_set.count(v)) {
                candidates.push_back(v);
            }
        }
    } 
    else if (rand < alpha_g_ + alpha_p_) {
        // Tomar de B: pbest \ current
        for (int v : p.best) {
            if (!current_set.count(v)) {
                candidates.push_back(v);
            }
        }
    } 
    else {
        // Tomar de C: exploración limitada (no todos los vértices)
        // OPTIMIZACIÓN: Solo samplear algunos aleatorios en lugar de iterar todos
        candidates.reserve(std::min(10, n_ - k_));
        std::uniform_int_distribution<int> dist(0, n_ - 1);
        
        int attempts = 0;
        const int max_attempts = std::min(20, n_);
        
        while ((int)candidates.size() < 10 && attempts < max_attempts) {
            int v = dist(rng_);
            if (!current_set.count(v) && 
                std::find(candidates.begin(), candidates.end(), v) == candidates.end()) {
                candidates.push_back(v);
            }
            attempts++;
        }
    }
    
    // Si no hay candidatos, retornar current sin cambios
    if (candidates.empty())
        return p.current;
    
    // Crear nueva posición
    std::vector<int> new_current = p.current;
    
    // Seleccionar candidato y posición aleatoria para reemplazar
    std::uniform_int_distribution<int> dist_cand(0, candidates.size() - 1);
    std::uniform_int_distribution<int> dist_pos(0, new_current.size() - 1);
    
    new_current[dist_pos(rng_)] = candidates[dist_cand(rng_)];
    
    return new_current;
}

// ============================================================================
// OPTIMIZACIÓN 3: Run con criterio de convergencia y sin reseteo de iteraciones
// ============================================================================
void PSO::run() {
    int stagnation = 0;
    const int MAX_STAGNATION = 5000;  // Ajustar según necesidad
    
    for (int iter = 0; iter < iterations_; ++iter) {
        bool improved = false;

        for (auto& p : swarm_.particles) {
            // Generar nueva posición mediante transición
            std::vector<int> new_pos = transition(p);
            double new_value = evaluate(new_pos);

            // Actualizar posición actual
            p.current = std::move(new_pos);  // OPTIMIZACIÓN: move en lugar de copy
            p.current_value = new_value;

            // Actualizar mejor personal si mejoró
            if (new_value < p.best_value) {
                p.best = p.current;
                p.best_value = new_value;
                
                // Actualizar mejor global si mejoró
                if (new_value < swarm_.gbest_value) {
                    swarm_.gbest = p.current;
                    swarm_.gbest_value = new_value;
                    improved = true;
                    stagnation = 0;  // Reset stagnation counter
                    iter = 0;      // Reset iteration counter
                }
            }   
        }

    }

    std::cout << "[PSO] Mejor solución final: peso = " 
              << swarm_.gbest_value / graph_.getNormalizador() << "\n";
}

// ============================================================================
// Funciones auxiliares (sin cambios necesarios)
// ============================================================================
std::vector<int> PSO::difference(const std::vector<int>& a, const std::vector<int>& b) {
    std::unordered_set<int> b_set(b.begin(), b.end());
    std::vector<int> result;
    result.reserve(a.size());
    
    for (int x : a)
        if (b_set.find(x) == b_set.end())
            result.push_back(x);
    
    return result;
}

std::vector<int> PSO::random_subset() const {
    std::uniform_int_distribution<int> dist_vertex(0, n_ - 1);
    std::unordered_set<int> used;
    used.reserve(k_);
    std::vector<int> subset;
    subset.reserve(k_);
    
    while ((int)subset.size() < k_) {
        int v = dist_vertex(rng_);
        if (used.insert(v).second)
            subset.push_back(v);
    }
    
    return subset;
}