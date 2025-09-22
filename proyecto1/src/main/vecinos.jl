# vecinos.jl - Funciones relacionadas con la generación de soluciones vecinas
using Random
using StatsBase
include("estructuras.jl")

"""
    vecino_aleatorio(permutacion::Vector{Int}) -> Vector{Int}

Genera una permutación vecina intercambiando dos elementos aleatorios.
"""
function vecino_aleatorio(permutacion::Vector{Int})
    nueva_perm = copy(permutacion)
    n = length(nueva_perm)
    
    # Seleccionar dos índices diferentes aleatoriamente
    s, t = sample(1:n, 2, replace=false)
    
    # Intercambiar elementos
    nueva_perm[s], nueva_perm[t] = nueva_perm[t], nueva_perm[s]
    
    return s, t
end

"""
    vecino_aleatorio!(permutacion::Vector{Int})

Modifica in-place la permutación dada, intercambiando dos elementos aleatorios.
"""
@inline function vecino_aleatorio!(permutacion::Vector{Int})
    n = length(permutacion)
    i = rand(1:n)
    j = rand(1:n)
    while i == j
        j = rand(1:n)
    end
    permutacion[i], permutacion[j] = permutacion[j], permutacion[i]
    return nothing
end

"""
    VecinoPool(n::Int, pool_size::Int = 4) -> VecinoPool

Estructura para gestionar un pool de vectores temporales y evitar asignaciones repetidas.
"""
mutable struct VecinoPool
    temp_vectors::Vector{Vector{Int}}
    current_index::Int
end

function VecinoPool(n::Int, pool_size::Int = 4)
    temp_vectors = [Vector{Int}(undef, n) for _ in 1:pool_size]
    return VecinoPool(temp_vectors, 1)
end

"""
    get_temp_vector!(pool::VecinoPool, source::Vector{Int}) -> Vector{Int}

Obtiene un vector temporal del pool y copia el contenido de `source` en él.
"""
@inline function get_temp_vector!(pool::VecinoPool, source::Vector{Int})
    temp_vec = pool.temp_vectors[pool.current_index]
    copyto!(temp_vec, source)
    pool.current_index = (pool.current_index % length(pool.temp_vectors)) + 1
    return temp_vec
end