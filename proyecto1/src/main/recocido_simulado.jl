# aceptacion_umbrales.jl - Implementación del algoritmo de Aceptación por Umbrales

include("estructuras.jl")
include("utilidades.jl")
include("vecinos.jl")
include("temperatura.jl")

using Printf

"""
    calcula_lote(tsp::TSP, L::Int, T::Float64, s::Vector{Int}, mejor_costo::Float64, max_intentos::Int = 100 * L; debug::Bool=false)

Realiza un lote de iteraciones para el algoritmo de aceptación por umbrales, devolviendo el costo promedio, la solución y el mejor costo.
"""
function calcula_lote(tsp::TSP, L::Int, T::Float64, s::Vector{Int}, mejor_costo::Float64, max_intentos::Int = 100 * L; debug::Bool=false)
    c = 0
    r = 0.0
    intentos = 0
    s_actual = copy(s)
    mejor = mejor_costo
    n = length(s)
    while c < L && intentos < max_intentos
        s, t = sample(1:n, 2, replace=false)
        costo_prima = calcula_swap(tsp, s_actual, mejor, s, t)
        if costo_prima <= mejor + T
            s_actual[s], s_actual[t] = s_actual[t], s_actual[s]
            mejor = costo_prima
            #println(@sprintf("Costo: , %.11f", mejor))
            c += 1
            r += costo_prima
        end
        intentos += 1
    end
    if c == 0
        return Inf, s_actual
    end
    return r / c, s_actual, mejor
end

"""
    aceptacion_por_umbrales(tsp::TSP, s::Vector{Int}; L::Int, φ::Float64, ε::Float64, T_inicial::Float64 = 8.0, P::Float64 = 0.9, max_iteraciones::Int)

Implementa el algoritmo de aceptación por umbrales para el TSP, devolviendo la mejor solución y su costo.
"""
function aceptacion_por_umbrales(tsp::TSP, s::Vector{Int}; 
                                L::Int,
                                φ::Float64,
                                ε::Float64 ,
                                T_inicial::Float64 = 8.0,
                                P::Float64 = 0.9,
                                max_iteraciones::Int )
    T = temperatura_inicial(tsp, s, T_inicial, P)
    mejor_solucion = copy(s)
    mejor_costo = funcion_costo(tsp, s)
    costo_actual = mejor_costo
    iteracion = 0
    while T > ε && iteracion < max_iteraciones
        iteracion += 1
        p = 0.0
        q = Inf
        intentos_equilibrio = 0
        max_intentos_equilibrio = 5
        while p <= q && intentos_equilibrio < max_intentos_equilibrio
            q = p
            p, s, costo_actual = calcula_lote(tsp, L, T, s, costo_actual)
            intentos_equilibrio += 1
            if p == Inf
                break
            end
            if costo_actual < mejor_costo
                mejor_solucion = copy(s)
                mejor_costo = costo_actual
            end
            if abs(p - q) < 0.001
                break
            end
        end
        T *= φ
    end
    return mejor_solucion, mejor_costo
end

"""
    calcula_2opt_delta(tsp::TSP, solucion::Vector{Int}, i::Int, j::Int)

Calcula el cambio de costo al aplicar una inversión 2-opt entre las posiciones i+1 y j.
"""
function calcula_2opt_delta(tsp::TSP, solucion::Vector{Int}, i::Int, j::Int)
    n = length(solucion)
    delta = 0.0
    if i >= 1
        elem_before = solucion[i]
        elem_start = solucion[i+1]
        elem_end = solucion[j]
        if i >= 1
            delta -= tsp.distancias[elem_before, elem_start]
            delta += tsp.distancias[elem_before, elem_end]
        end
        if j < n
            elem_after = solucion[j+1]
            delta -= tsp.distancias[elem_end, elem_after]
            delta += tsp.distancias[elem_start, elem_after]
        end
    end
    return delta / tsp.normalizador
end

"""
    barrido(tsp::TSP, solucion::Vector{Int}, mejor::Float64)

Realiza un barrido de swaps para mejorar la solución dada.
"""
function barrido(tsp::TSP, solucion::Vector{Int}, mejor::Float64 )
    mejor_solucion = copy(solucion)
    mejor_costo = mejor
    n = length(solucion)
    mejorado = true
    while mejorado
        mejorado = false
        for i in 1:n-1
            for j in i+1:n
                costo_vecino = calcula_swap(tsp, mejor_solucion, mejor_costo, i, j)
                if costo_vecino < mejor_costo
                    mejor_solucion[i], mejor_solucion[j] = mejor_solucion[j], mejor_solucion[i]
                    mejor_costo = costo_vecino
                    #println(@sprintf("Nuevo mejor costo normalizado: %.6f (swap %d-%d)", mejor_costo, i, j))
                    mejorado = true
                end
            end
        end
    end
    return mejor_solucion, mejor_costo
end

"""
    barrido_20pt(tsp::TSP, solucion::Vector{Int}, costo::Float64)

Realiza un barrido 2-opt clásico sobre la solución dada.
"""
function barrido_2Opt(tsp::TSP, solucion::Vector{Int}, costo::Float64) 
    mejor_solucion = copy(solucion)
    mejor_costo = costo
    n = length(solucion)
    mejorado = true
    while mejorado
        mejorado = false
        for i in 1:n-1
            for j in i+2:n
                delta = calcula_2opt_delta(tsp, mejor_solucion, i, j)
                if delta < 0
                    reverse!(mejor_solucion, i+1, j)
                    mejor_costo += delta
                    #println(@sprintf("Costo: %.11f", mejor_costo))
                    mejorado = true
                end
            end
        end
    end
    return mejor_solucion, mejor_costo
end

# La función calcula_swap mantiene comentarios de una línea para explicar casos específicos
function calcula_swap(tsp::TSP, solucion::Vector{Int}, mejor_costo::Float64, i::Int, j::Int)
    if i > j
        i, j = j, i
    end

    n = length(solucion)
    delta = 0.0

    if j == i + 1
        # --- Caso adyacente ---
        if i == 1
            # Cambia (s1,s2) y (s2,s3) por (s2,s1) y (s1,s3)
            delta = tsp.distancias[solucion[2], solucion[1]] +
                    tsp.distancias[solucion[1], solucion[3]] -
                    (tsp.distancias[solucion[1], solucion[2]] +
                     tsp.distancias[solucion[2], solucion[3]])
        elseif j == n
            # Cambia (s_{n-2},s_{n-1}) y (s_{n-1},s_n) por (s_{n-2},s_n) y (s_n,s_{n-1})
            delta = tsp.distancias[solucion[n-2], solucion[n]] +
                    tsp.distancias[solucion[n], solucion[n-1]] -
                    (tsp.distancias[solucion[n-2], solucion[n-1]] +
                     tsp.distancias[solucion[n-1], solucion[n]])
        else
            # Caso general adyacente
            delta = tsp.distancias[solucion[i-1], solucion[j]] +
                    tsp.distancias[solucion[i], solucion[j+1]] -
                    (tsp.distancias[solucion[i-1], solucion[i]] +
                     tsp.distancias[solucion[j], solucion[j+1]])
        end
    else
        # --- Caso no adyacente ---
        if i == 1 && j == n
            # Invertir extremos: afecta solo a (s1,s2) y (s_{n-1},s_n)
            delta = tsp.distancias[solucion[n-1], solucion[1]] +
                    tsp.distancias[solucion[n], solucion[2]] -
                    (tsp.distancias[solucion[1], solucion[2]] +
                     tsp.distancias[solucion[n-1], solucion[n]])
        elseif i == 1
            # Intercambiar s1 y sj
            delta = tsp.distancias[solucion[j], solucion[2]] +
                    tsp.distancias[solucion[j-1], solucion[1]] +
                    tsp.distancias[solucion[1], solucion[j+1]] -
                    (tsp.distancias[solucion[1], solucion[2]] +
                     tsp.distancias[solucion[j-1], solucion[j]] +
                     tsp.distancias[solucion[j], solucion[j+1]])
        elseif j == n
            # Intercambiar si y sn
            delta = tsp.distancias[solucion[i-1], solucion[j]] +
                    tsp.distancias[solucion[j], solucion[i+1]] +
                    tsp.distancias[solucion[j-1], solucion[i]] -
                    (tsp.distancias[solucion[i-1], solucion[i]] +
                     tsp.distancias[solucion[i], solucion[i+1]] +
                     tsp.distancias[solucion[j-1], solucion[j]])
        else
            # Caso general no adyacente
            delta = tsp.distancias[solucion[i-1], solucion[j]] +
                    tsp.distancias[solucion[j], solucion[i+1]] +
                    tsp.distancias[solucion[j-1], solucion[i]] +
                    tsp.distancias[solucion[i], solucion[j+1]] -
                    (tsp.distancias[solucion[i-1], solucion[i]] +
                     tsp.distancias[solucion[i], solucion[i+1]] +
                     tsp.distancias[solucion[j-1], solucion[j]] +
                     tsp.distancias[solucion[j], solucion[j+1]])
        end
    end

    return mejor_costo + (delta / tsp.normalizador)
end
