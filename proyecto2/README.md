# Proyecto k-MST Heurístico con PSO

Este proyecto implementa una heurística basada en **Particle Swarm Optimization (PSO)** para resolver el **problema del k-árbol generador de peso mínimo (k-MST)**.  
El sistema está desarrollado en **C++20** utilizando **Meson** como sistema de construcción y soporta **paralelización con OpenMP** para ejecutar múltiples semillas simultáneamente.

---

## 📘 Descripción general

El problema del **k-MST** consiste en encontrar un subárbol de exactamente `k` vértices dentro de un grafo ponderado, de forma que el peso total de las aristas sea mínimo.  
Este problema es **NP-hard**, por lo que se recurre a métodos heurísticos.

La heurística desarrollada utiliza un **PSO discreto**, donde cada partícula representa un conjunto de vértices del grafo.  
El movimiento de las partículas se define mediante tres conjuntos:

- **A** → vértices presentes en la mejor global (`gbest`) pero no en la partícula actual.  
- **B** → vértices presentes en la mejor personal (`pbest`) pero no en la actual.  
- **C** → vértices restantes del grafo no usados en A ni B.  

El algoritmo combina **exploración estocástica** con una **búsqueda local determinista (`sweep`)** aplicada al final para alcanzar un mínimo local estable.

---

## ⚙️ Compilación

### Requisitos

- **Meson** ≥ 1.0  
- **Ninja**  
- **g++** con soporte para C++20 y OpenMP  

### Instrucciones

```bash
# Configurar el proyecto
meson setup build

# Compilar
meson compile -C build
````

El ejecutable se genera en `./build/kmst`.

---

## 🚀 Uso

```bash
./build/kmst <file> <k> <swarm_size> <seed> [--viz]
./build/kmst <file> <k> <swarm_size> <seed1> <seed2> ... [--viz]
./build/kmst <file> <k> <swarm_size> <seed_inicio>-<seed_fin> [--viz]
```

### Argumentos

| Argumento      | Descripción                                             |
| -------------- | ------------------------------------------------------- |
| `<file>`       | Ruta al archivo de entrada con la definición del grafo. |
| `<k>`          | Número de vértices del árbol k-MST.                     |
| `<swarm_size>` | Número de partículas del enjambre.                      |
| `<seed>`       | Semilla aleatoria (o rango de semillas).                |

### Opciones

| Opción         | Descripción                                        |
| -------------- | -------------------------------------------------- |
| `--viz`        | Genera una visualización SVG de la mejor solución. |
| `--viz-tree`   | Visualización en forma de árbol.                   |
| `--viz-circle` | Visualización circular (predeterminada).           |

---

## 📄 Formato del archivo de entrada

El archivo de entrada debe contener una lista de aristas separadas por punto y coma (`;`):

```
u1,v1,w1;
u2,v2,w2;
...
```

Donde:

* `u` y `v` son los vértices (pueden ser etiquetas alfanuméricas).
* `w` es el peso asociado a la arista (valor real positivo).

Ejemplo:

```
A,B,1.5;
A,C,2.0;
B,C,1.2;
C,D,3.5;
```

---

## 🧩 Salida

Por cada semilla ejecutada, se genera un archivo:

```
kmst-<seed>.mst
```

con el siguiente formato:

```
# Resultados PSO - Semilla <seed>
# Mejor conjunto (gbest): V23 V314 V685 ...
# Peso total: 0.784321
```

Además, si se activa la opción `--viz`, se genera un archivo SVG correspondiente a la visualización de la mejor solución.

---

## 🧠 Componentes principales

| Archivo                               | Descripción                                                                     |
| ------------------------------------- | ------------------------------------------------------------------------------- |
| `graph.hpp / graph.cpp`               | Implementa la representación del grafo y algoritmos como Floyd-Warshall y Prim. |
| `graph_reader.hpp / graph_reader.cpp` | Lector de grafos desde archivo.                                                 |
| `pso.hpp / pso.cpp`                   | Implementación del PSO discreto y la búsqueda local `sweep()`.                  |
| `main.cpp`                            | Punto de entrada, manejo de semillas y paralelización con OpenMP.               |

---

## 🧪 Ejemplo de ejecución

```bash
./build/kmst ./data/input.txt 40 50 1-16 --viz
```

Esto ejecuta el PSO sobre el grafo `input.txt`, con `k = 40`, un enjambre de 50 partículas y semillas del 1 al 16, generando un archivo de resultados por semilla y una visualización SVG del mejor resultado global.

---

## 📈 Paralelización

El sistema usa OpenMP para ejecutar distintas semillas de forma concurrente:

```cpp
#pragma omp parallel for schedule(dynamic)
```

Cada hilo:

* ejecuta su propia heurística PSO independiente,
* guarda los resultados en archivos `kmst-<seed>.mst`,
* y participa en la selección del mejor global.

---


## 👤 Autor

**Diego Iain Ortíz Montiel**
Facultad de Ciencias, UNAM — 2025

```

