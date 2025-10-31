#include "../include/graph_reader.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

Graph GraphReader::from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }
    return from_stream(file);
}

Graph GraphReader::from_stream(std::istream& input) {
    std::string content((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());

    // eliminar saltos de línea y retornos
    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

    Graph g;

    size_t pos = 0;
    while (true) {
        // buscar primer vértice (V...)
        size_t start = content.find('V', pos);
        if (start == std::string::npos) break;

        size_t comma1 = content.find(',', start);
        if (comma1 == std::string::npos) break;

        size_t v_start = content.find('V', comma1 + 1);
        if (v_start == std::string::npos) break;

        size_t comma2 = content.find(',', v_start);
        if (comma2 == std::string::npos) break;

        std::string u = content.substr(start, comma1 - start);
        std::string v = content.substr(v_start, comma2 - v_start);

        // buscar inicio del siguiente V (para cortar el peso)
        size_t next_v = content.find('V', comma2 + 1);
        std::string w_str = (next_v == std::string::npos)
                            ? content.substr(comma2 + 1)
                            : content.substr(comma2 + 1, next_v - comma2 - 1);

        // limpiar
        auto trim = [](std::string &s) {
            s.erase(0, s.find_first_not_of(" \t"));
            s.erase(s.find_last_not_of(" \t") + 1);
        };
        trim(u); trim(v); trim(w_str);

        try {
            double w = std::stod(w_str);
            g.add_edge(u, v, w);
        } catch (...) {
            std::cerr << "[WARN] Error parseando arista: " << u << "," << v << "," << w_str << "\n";
        }

        pos = (next_v == std::string::npos) ? content.size() : next_v;
    }

    return g;
}
