#include "graphy.hpp"
#include <fstream>
#include <algorithm>
#include <queue>
#include <iostream>

Graphy::Graphy(const Graph& graph) : graph_(graph) {}

std::string Graphy::dimensionaLienzo(int ancho, int alto) {
    return " width='" + std::to_string(ancho) + 
           "' height='" + std::to_string(alto) + "'>\n";
}

std::string Graphy::dibujaArista(int x1, int y1, int x2, int y2, 
                                  const std::string& color, int stroke_width) {
    return "\t<line x1='" + std::to_string(x1) + 
           "' y1='" + std::to_string(y1) + 
           "' x2='" + std::to_string(x2) + 
           "' y2='" + std::to_string(y2) + 
           "' stroke='" + color + 
           "' stroke-width='" + std::to_string(stroke_width) + "' />\n";
}

std::string Graphy::escribe(int x, int y, const std::string& texto, 
                             const std::string& color) {
    return "\t<text x='" + std::to_string(x) + 
           "' y='" + std::to_string(y) + 
           "' fill='" + color + 
           "' font-family='Arial' font-size='14' text-anchor='middle'>" + 
           texto + "</text>\n";
}

std::string Graphy::dibujaVertice(const std::string& nombre, int x, int y, 
                                   const std::string& color) {
    std::string s = "\t<circle cx='" + std::to_string(x) + 
                    "' cy='" + std::to_string(y) + 
                    "' r='20' fill='white' stroke='black' stroke-width='2'/>\n";
    s += escribe(x, y + 5, nombre, color);
    return s;
}

int Graphy::calcularAnchoSubarbol(const std::vector<Nodo>& nodos, int nodo_id) {
    const auto& nodo = nodos[nodo_id];
    
    if (nodo.hijos.empty()) {
        return 1;
    }
    
    int ancho_total = 0;
    for (int hijo_id : nodo.hijos) {
        ancho_total += calcularAnchoSubarbol(nodos, hijo_id);
    }
    
    return std::max(1, ancho_total);
}

void Graphy::calcularPosicionesArbol(std::vector<Nodo>& nodos, int raiz, 
                                     int x, int y, int nivel, int ancho_nivel) {
    nodos[raiz].x = x;
    nodos[raiz].y = y;
    
    if (nodos[raiz].hijos.empty()) {
        return;
    }
    
    // Calcular ancho de cada subárbol hijo
    std::vector<int> anchos_hijos;
    int ancho_total = 0;
    for (int hijo_id : nodos[raiz].hijos) {
        int ancho = calcularAnchoSubarbol(nodos, hijo_id);
        anchos_hijos.push_back(ancho);
        ancho_total += ancho;
    }
    
    // Calcular espaciado entre hijos
    int espaciado = ancho_nivel / std::max(1, ancho_total);
    int y_hijo = y + 80;
    
    // Posicionar hijos centrados bajo el padre
    int x_actual = x - (ancho_total * espaciado) / 2;
    
    for (size_t i = 0; i < nodos[raiz].hijos.size(); ++i) {
        int hijo_id = nodos[raiz].hijos[i];
        int ancho_hijo = anchos_hijos[i];
        int x_hijo = x_actual + (ancho_hijo * espaciado) / 2;
        
        calcularPosicionesArbol(nodos, hijo_id, x_hijo, y_hijo, nivel + 1, 
                                ancho_hijo * espaciado);
        
        x_actual += ancho_hijo * espaciado;
    }
}

void Graphy::dibujaArbol(const std::vector<int>& vertex_subset,
                         const std::vector<int>& parent,
                         const std::string& output_file) {
    int k = vertex_subset.size();
    if (k == 0) return;
    
    // Construir árbol de nodos
    std::vector<Nodo> nodos(k);
    int raiz = -1;
    
    for (int i = 0; i < k; ++i) {
        nodos[i].id = vertex_subset[i];
        nodos[i].padre = parent[i];
        
        if (parent[i] == -1) {
            raiz = i;
        } else {
            nodos[parent[i]].hijos.push_back(i);
        }
    }
    
    if (raiz == -1) raiz = 0; // Fallback
    
    // Calcular dimensiones del lienzo
    int altura_arbol = 0;
    std::queue<std::pair<int, int>> q;
    q.push({raiz, 0});
    
    while (!q.empty()) {
        auto [nodo_id, nivel] = q.front();
        q.pop();
        altura_arbol = std::max(altura_arbol, nivel);
        
        for (int hijo : nodos[nodo_id].hijos) {
            q.push({hijo, nivel + 1});
        }
    }
    
    int alto = (altura_arbol + 1) * 80 + 100;
    int ancho = k * 60 + 100;
    
    // Calcular posiciones
    calcularPosicionesArbol(nodos, raiz, ancho / 2, 50, 0, ancho - 100);
    
    // Iniciar SVG
    svg_ = INICIO;
    svg_ += dimensionaLienzo(ancho, alto);
    
    // Dibujar aristas primero (para que queden detrás)
    for (int i = 0; i < k; ++i) {
        if (nodos[i].padre != -1) {
            int padre_id = nodos[i].padre;
            svg_ += dibujaArista(nodos[padre_id].x, nodos[padre_id].y,
                                nodos[i].x, nodos[i].y, "#2563eb", 3);
        }
    }
    
    // Dibujar vértices
    for (int i = 0; i < k; ++i) {
        std::string nombre = graph_.get_vertex_name(nodos[i].id);
        svg_ += dibujaVertice(nombre, nodos[i].x, nodos[i].y, "black");
    }
    
    svg_ += FINAL;
    
    // Guardar archivo
    std::ofstream out(output_file);
    out << svg_;
    out.close();
    
    std::cout << "[Graphy] Árbol guardado en: " << output_file << "\n";
}

void Graphy::dibujaCircular(const std::vector<int>& vertex_subset,
                            const std::vector<int>& parent,
                            const std::string& output_file) {
    int k = vertex_subset.size();
    if (k == 0) return;
    
    // Calcular dimensiones
    int radio = (k * 800) / 100;
    int ancho = 2 * radio + 100;
    int altura = ancho;
    int centroX = ancho / 2;
    int centroY = altura / 2;
    
    // Calcular posiciones en círculo
    double angulo = 2 * M_PI / k;
    std::vector<int> xCoords(k);
    std::vector<int> yCoords(k);
    
    for (int i = 0; i < k; ++i) {
        xCoords[i] = static_cast<int>(centroX + radio * std::cos(i * angulo));
        yCoords[i] = static_cast<int>(centroY + radio * std::sin(i * angulo));
    }
    
    // Iniciar SVG
    svg_ = INICIO;
    svg_ += dimensionaLienzo(ancho, altura);
    
    // Dibujar aristas del MST
    for (int i = 0; i < k; ++i) {
        if (parent[i] != -1) {
            svg_ += dibujaArista(xCoords[parent[i]], yCoords[parent[i]],
                                xCoords[i], yCoords[i], "#2563eb", 3);
        }
    }
    
    // Dibujar vértices
    for (int i = 0; i < k; ++i) {
        std::string nombre = graph_.get_vertex_name(vertex_subset[i]);
        svg_ += dibujaVertice(nombre, xCoords[i], yCoords[i], "black");
    }
    
    svg_ += FINAL;
    
    // Guardar archivo
    std::ofstream out(output_file);
    out << svg_;
    out.close();
    
    std::cout << "[Graphy] Grafo circular guardado en: " << output_file << "\n";
}