#pragma once
#include <vector>
#include <iostream>
#include <limits>
#include <tuple>
#include <string>
#include <utility>
#include <unordered_map>

/**
 * Clase Graph
 * Representa un grafo no dirigido y ponderado mediante matriz de adyacencia.
 * adj[i][j] almacena el peso de la arista entre i y j, o +inf si no hay arista.
 */
class Graph {
public:
    using Matrix = std::vector<std::vector<double>>;

private:
    int n;              // número de vértices
    int m;              // número de aristas
    Matrix adj;       
    double diameter_;   
    Matrix distances_;
    double normalizador_;  
    double dist_normalizador_;

    std::unordered_map<std::string, int> vertex_to_id;
    std::vector<std::string> id_to_vertex;

public:
    // --- Constructores ---
    Graph();
    explicit Graph(int n);


    // --- Métodos de construcción ---
    /**
     * Agrega una arista entre dos vértices identificados por string.
     * Si los vértices no existen, los crea automáticamente.
     */
    void add_edge(const std::string& u, const std::string& v, double w);
    
    /**
     * Obtiene o crea el ID interno de un vértice.
     * @return índice del vértice
     */
    int get_or_create_vertex(const std::string& name);
    
    /**
     * Obtiene el nombre de un vértice dado su ID.
     * @param id índice interno del vértice
     * @return nombre del vértice
     */
    std::string get_vertex_name(int id) const;
    
    /**
     * Obtiene el ID de un vértice dado su nombre.
     * @param name nombre del vértice
     * @return índice interno, o -1 si no existe
     */
    int get_vertex_id(const std::string& name) const;

    // --- Consultas ---
    double weight(const std::string& u, const std::string& v) const;
    bool has_edge(const std::string& u, const std::string& v) const;
    double getNormalizador() const { return normalizador_; }
    const Matrix& adjacency() const { return adj; }
    const Matrix& distances() const { return distances_; }

    // --- Información general ---
    /** @return número de vértices del grafo. */
    int num_vertices() const { return n; }

    /** @return número de aristas del grafo. */
    int num_edges() const { return m; }

    /** @return el diámetro del grafo (longitud máxima de los caminos mínimos). */
    double diameter() const { return diameter_; }

    // --- Utilidades ---
    /** Imprime una representación de la gráfica. */
    void print() const;

    // --- Algoritmos ---
    /**
     * Algoritmo de Floyd–Warshall.
     * Calcula la matriz de distancias mínimas entre todos los pares de vértices.
     * 
     * @return matriz de distancias mínimas.
     */
    Matrix floyd_warshall();

    /**
     * Completa la matriz de adyacencias usando la función de costo:
     * 
     *      f(u,v) = w(u,v)             si u y v son adyacentes
     *             = d(u,v) * diameter  en otro caso
     * 
     * donde d(u,v) es la distancia mínima entre u y v obtenida
     * por Floyd–Warshall. Debe ejecutarse después de floyd_warshall().
     */
    void complete(int k);

    /**
     * Calcula el normalizador, definido como la suma de los k - 1 mayores pesos
     * @param k tamaño del conjunto a considerar
     */
    void calcula_Normalizador(int k);

    /**
     * Algoritmo de Prim.
     * Calcula el árbol de expansión mínima (MST) usando la matriz de adyacencia.
     * 
     * @param start nombre del vértice inicial (vacío = usar el primero).
     * @return par:
     *         - first: cadena con formato "v1,v2,p1;v2,v3,p2;..."
     *         - second: peso total del MST.
     */
    std::pair<std::string, double> prim(const std::string& start = "") const;

    /**
     * Algoritmo de Prim sobre SUBCONJUNTO de vértices.
     * Calcula el MST solo usando los vértices especificados.
     * 
     * @param vertex_subset Vector de IDs internos de vértices a incluir.
     * @return par (string con aristas, peso total del MST).
     */
    std::pair<std::string, double> prim_subset(const std::vector<int>& vertex_subset) const;

};

