#pragma once
#include <vector>
#include <iostream>
#include <limits>
#include <tuple>
#include <string>
#include <utility>

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

public:
    // --- Constructores ---
    Graph();
    explicit Graph(int n);

    // --- Métodos de construcción ---
    void add_edge(int u, int v, double w);

    // --- Consultas ---
    double weight(int u, int v) const;
    bool has_edge(int u, int v) const;
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
    void complete();

    /**
     * Calcula el normalizador, definido como la suma de los  k - 1 mayores pesos
     */
    void calcula_Normalizador() const;

    /**
     * Algoritmo de Prim.
     * Calcula el árbol de expansión mínima (MST) usando la matriz de adyacencia.
     * 
     * @param start vértice inicial (por defecto 0).
     * @return par:
     *         - first: cadena con formato "v1,v2,p1;v2,v3,p2;..."
     *         - second: peso total del MST.
     */
    std::pair<std::string, double> prim(int start = 0) const;


};
