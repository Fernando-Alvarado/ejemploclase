#pragma once
#include "graph.hpp"
#include <string>
#include <iostream>
#include <fstream>

/**
 * Clase GraphReader
 * Lee una gráfica a partir de un archivo o cadena de texto
 * en formato "u,v,w;u,v,w;..." sin indicar tamaño explícito.
 */
class GraphReader {
public:
    /**
     * Construye un grafo a partir del contenido de un archivo.
     * @param filename Ruta del archivo.
     * @return Objeto Graph construido a partir del contenido.
     */
    static Graph from_file(const std::string& filename);

    /**
     * Construye un grafo a partir de una cadena o flujo.
     * @param input Flujo de texto en formato "u,v,w;..."
     * @return Objeto Graph correspondiente.
     */
    static Graph from_stream(std::istream& input);
};
