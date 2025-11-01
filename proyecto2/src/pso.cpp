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
    swarm_.particles.reserve(swarm_size);
    swarm_.gbest.reserve(k);
}

void PSO::initialize() {
    std::uniform_int_distribution<int> dist_vertex(0, n_ - 1);
    double best_global_value = std::numeric_limits<double>::infinity();
    std::vector<int> best_global_set;
    best_global_set.reserve(k_);

    for (auto& p : swarm_.particles) {
        std::unordered_set<int> used;
        used.reserve(k_);
        p.current.clear();
        p.current.reserve(k_);
        
        while ((int)p.current.size() < k_) {
            int v = dist_vertex(rng_);
            if (used.insert(v).second)
                p.current.push_back(v);
        }

        double cost = graph_.prim_subset(p.current);
        
        p.best = p.current;
        p.best.reserve(k_);
        p.best_value = cost;
        p.current_value = cost;
        if (cost < best_global_value) {
            best_global_value = cost;
            best_global_set = p.current;
        } 
    }

    swarm_.gbest = best_global_set;
    swarm_.gbest_value = best_global_value;
}

std::vector<int> PSO::transition(const Particle& p) const {
    thread_local std::unordered_set<int> current_set;
    thread_local std::vector<int> candidates;
    
    current_set.clear();
    current_set.insert(p.current.begin(), p.current.end());
    
    candidates.clear();
    
    double rand = dist01_(rng_);
    if (rand < alpha_g_) {
        for (int v : swarm_.gbest) {
            if (!current_set.count(v)) {
                candidates.push_back(v);
            }
        }
    } 
    else if (rand < alpha_g_ + alpha_p_) {
        for (int v : p.best) {
            if (!current_set.count(v)) {
                candidates.push_back(v);
            }
        }
    } 
    else {

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
    
    if (candidates.empty())
        return p.current;
    
    std::vector<int> new_current = p.current;
   
    std::uniform_int_distribution<int> dist_cand(0, candidates.size() - 1);
    std::uniform_int_distribution<int> dist_pos(0, new_current.size() - 1);
    
    new_current[dist_pos(rng_)] = candidates[dist_cand(rng_)];
    
    return new_current;
}

void PSO::run() {
    //int stagnation = 0;
    //const int MAX_STAGNATION = 5000;  
    for (int iter = 0; iter < iterations_; ++iter) {
        //bool improved = false;

        for (auto& p : swarm_.particles) {
            std::vector<int> new_pos = transition(p);
            double new_value = graph_.prim_subset(new_pos);

            p.current = std::move(new_pos);  
            p.current_value = new_value;

            if (new_value < p.best_value) {
                p.best = p.current;
                p.best_value = new_value;
                
                if (new_value < swarm_.gbest_value) {
                    swarm_.gbest = p.current;
                    swarm_.gbest_value = new_value;
                    //improved = true;
                    //stagnation = 0;  
                    iter = 0;      
                }
            }   
        }

    }

}


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

void PSO::sweep() {
    std::vector<int> best = swarm_.gbest;
    double best_val = swarm_.gbest_value;

    std::unordered_set<int> in_set(best.begin(), best.end());

    std::vector<int> out_set;
    out_set.reserve(n_ - k_);
    for (int v = 0; v < n_; ++v)
        if (!in_set.count(v)) out_set.push_back(v);

    bool improved = true;
    int iteration = 0;

    while (improved) {
        improved = false;
        iteration++;

        for (size_t i = 0; i < best.size(); ++i) {
            int in_v = best[i];
            for (size_t j = 0; j < out_set.size(); ++j) {
                int out_v = out_set[j];

                std::vector<int> candidate = best;
                candidate[i] = out_v;

                double val = graph_.prim_subset(candidate);
                if (val < best_val) {
                    best_val = val;             
                    improved = true;

                    out_set[j] = in_v;
                    best = std::move(candidate);
                    break; 
                }
            }
            if (improved) break; 
        }
    }

    if (best_val < swarm_.gbest_value) {
        swarm_.gbest = std::move(best);
        swarm_.gbest_value = best_val;
    }
}
