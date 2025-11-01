#ifndef GRAPHY_HPP
#define GRAPHY_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include "graph.hpp"

class Graphy {
private:
    const Graph& graph_;
    std::string svg_;
    
    static constexpr const char* INICIO = "<?xml version='1.0' encoding='UTF-8' ?>\n<svg xmlns='http://www.w3.org/2000/svg'";
    static constexpr const char* FINAL = "</svg>";
    
    struct Nodo {
        int id;
        int x, y;
        std::vector<int> hijos;
        int padre;
    };
    
    std::string dimensionaLienzo(int ancho, int alto);
    std::string dibujaArista(int x1, int y1, int x2, int y2, const std::string& color = "black", int stroke_width = 2);
    std::string dibujaVertice(const std::string& nombre, int x, int y, const std::string& color = "black");
    std::string escribe(int x, int y, const std::string& texto, const std::string& color);
    
    void calcularPosicionesArbol(std::vector<Nodo>& nodos, int raiz, int x, int y, int nivel, int ancho_nivel);
    int calcularAnchoSubarbol(const std::vector<Nodo>& nodos, int nodo_id);
    
public:
    Graphy(const Graph& graph);
    
    // Dibuja el MST como árbol (requiere vector de padres y conjunto de vértices)
    void dibujaArbol(const std::vector<int>& vertex_subset, 
                     const std::vector<int>& parent,
                     const std::string& output_file);
    
    // Dibuja el MST como grafo circular
    void dibujaCircular(const std::vector<int>& vertex_subset,
                        const std::vector<int>& parent,
                        const std::string& output_file);
};

#endif 