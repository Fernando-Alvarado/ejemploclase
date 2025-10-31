#pragma once
#include "graph.hpp"
#include <vector>
#include <random>
#include <limits>
#include <unordered_set>
#include <iomanip>

/**
 * PSO Discreto (D-PSO) para el problema del k-MST.
 * 
 * ALGORITMO:
 * ----------
 * Cada partícula representa un conjunto S de k vértices.
 * 
 * Para actualizar una partícula, se definen 3 conjuntos:
 *   A = gbest \ S     (vértices en mejor global que NO están en S)
 *   B = pbest \ S     (vértices en mejor personal que NO están en S)
 *   C = V \ (S ∪ gbest ∪ pbest)  (todos los demás vértices)
 * 
 * TRANSICIÓN:
 * -----------
 * Con probabilidad alpha_g: 
 *   - Seleccionar v ∈ A aleatoriamente
 *   - Reemplazar un vértice aleatorio de S con v
 * 
 * Con probabilidad alpha_p:
 *   - Seleccionar v ∈ B aleatoriamente
 *   - Reemplazar un vértice aleatorio de S con v
 * 
 * En caso contrario:
 *   - Seleccionar v ∈ C aleatoriamente
 *   - Reemplazar un vértice aleatorio de S con v
 * 
 * PARÁMETROS TÍPICOS:
 * -------------------
 * alpha_g ∈ [0.5, 0.7]  → Influencia del mejor global
 * alpha_p ∈ [0.2, 0.4]  → Influencia del mejor personal
 * alpha_g + alpha_p < 1 → El resto es exploración (conjunto C)
 */
class PSO {
public:
    /**
     * Estructura de una partícula.
     * Representa una solución candidata (conjunto de k vértices).
     */
    struct Particle {
        std::vector<int> current;        // Solución actual S
        std::vector<int> best;           // Mejor solución personal pbest
        double best_value;               // f(pbest) = peso del MST
        double current_value;            // f(S) = peso del MST actual

        Particle(int k = 0)
            : current(), best(), 
              best_value(std::numeric_limits<double>::infinity()),
              current_value(std::numeric_limits<double>::infinity()) {}
    };

    /**
     * Estructura del enjambre completo.
     */
    struct Swarm {
        std::vector<Particle> particles;   // Conjunto de partículas
        std::vector<int> gbest;            // Mejor solución global
        double gbest_value;                // f(gbest) = mejor peso encontrado

        Swarm(int size = 0)
            : particles(size),
              gbest_value(std::numeric_limits<double>::infinity()) {}
    };

private:
    // --- Problema ---
    const Graph& graph_;     // Grafica completa
    int k_;                  // Número de vértices en el MST
    int n_;                  // Total de vértices de la grafica

    // --- Parámetros PSO ---
    int swarm_size_;         // Tamaño del enjambre
    int iterations_;         // Iteraciones máximas
    double alpha_g_;         // P(seleccionar de A) - influencia global
    double alpha_p_;         // P(seleccionar de B) - influencia personal
                             // P(seleccionar de C) = 1 - alpha_g - alpha_p

    // --- Estado ---
    Swarm swarm_;

    // --- Aleatoriedad ---
    mutable std::mt19937 rng_;
    mutable std::uniform_real_distribution<double> dist01_;

public:
    /**
     * Constructor del optimizador PSO.
     * 
     * @param graph Grafo completo sobre el cual buscar el k-MST
     * @param k Número de vértices a seleccionar
     * @param swarm_size Número de partículas en el enjambre
     * @param iterations Número máximo de iteraciones
     * @param alpha_g Probabilidad de seleccionar de A (influencia global)
     * @param alpha_p Probabilidad de seleccionar de B (influencia personal)
     * @param seed Semilla para generador aleatorio
     */
    PSO(const Graph& graph, int k, int swarm_size, int iterations,
        double alpha_g = 0.6, double alpha_p = 0.3, 
        unsigned seed = std::random_device{}());

    /**
     * Inicializa el enjambre con soluciones aleatorias.
     */
    void initialize();

    /**
     * Ejecuta el algoritmo PSO por el número especificado de iteraciones.
     */
    void run();

    /**
     * Obtiene el mejor conjunto de vértices encontrado.
     * @return Vector de IDs de vértices de la mejor solución
     */
    const std::vector<int>& best_set() const { return swarm_.gbest; }

    /**
     * Obtiene el peso del MST de la mejor solución.
     * @return Peso total del k-MST óptimo encontrado
     */
    double best_value() const { return swarm_.gbest_value; }

private:
    /**
     * Evalúa un conjunto de vértices calculando el peso de su MST.
     * @param vertices Conjunto de k vértices
     * @return Peso total del MST sobre ese subconjunto
     */
    double evaluate(const std::vector<int>& vertices) const;

    /**
     * Realiza la transición discreta de una partícula.
     * Calcula los conjuntos A, B, C y selecciona un vértice para intercambio.
     * 
     * @param p Partícula a transicionar
     * @return Nueva posición (conjunto de k vértices)
     */
    std::vector<int> transition(const Particle& p, double alpha_g_t, double alpha_p_t) const;


    /**
     * Calcula la diferencia entre dos conjuntos: a \ b
     * @return Elementos en 'a' que no están en 'b'
     */
    static std::vector<int> difference(const std::vector<int>& a, 
                                       const std::vector<int>& b);

    /**
     * Genera un subconjunto aleatorio de k vértices.
     * @return Vector con k vértices únicos seleccionados aleatoriamente
     */
    std::vector<int> random_subset() const;
};