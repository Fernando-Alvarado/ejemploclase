# database_reader.jl - Manejo de entrada/salida y base de datos

using SQLite
using DataFrames
using DBInterface
include("estructuras.jl")
include("utilidades.jl")

"""
    leer_archivo_tsp(archivo::String) -> TSPInput

Lee un archivo .tsp que contiene la secuencia de IDs y retorna un TSPInput.
"""
function leer_archivo_tsp(archivo::String)
    content = strip(read(archivo, String))
    path = [parse(Int, x) for x in split(content, ",")]
    return TSPInput(path)
end

"""
    cargar_ciudades_desde_db(db_path::String, city_ids::Vector{Int}) -> Tuple{Vector{Ciudad}, Dict{Int,Int}}

Carga las ciudades desde la base de datos y retorna el vector de ciudades y un diccionario de id a índice.
"""
function cargar_ciudades_desde_db(db_path::String, city_ids::Vector{Int})
    db = SQLite.DB(db_path)
    ids_str = join(city_ids, ",")
    query = "SELECT id, name, country, population, latitude, longitude FROM cities WHERE id IN ($ids_str)"
    result = DBInterface.execute(db, query) |> DataFrame
    ciudades = Ciudad[]
    for row in eachrow(result)
        push!(ciudades, Ciudad(
            row.id,
            row.name,
            row.country,
            row.population,
            row.latitude,
            row.longitude
        ))
    end
    city_id_to_index = Dict{Int, Int}()
    for (i, ciudad) in enumerate(ciudades)
        city_id_to_index[ciudad.id] = i
    end
    SQLite.close(db)
    return ciudades, city_id_to_index
end

"""
    cargar_conexiones_desde_db(db_path::String, city_ids::Vector{Int}) -> DataFrame

Carga las conexiones entre ciudades desde la base de datos.
"""
function cargar_conexiones_desde_db(db_path::String, city_ids::Vector{Int})
    db = SQLite.DB(db_path)
    ids_str = join(city_ids, ",")
    query = """
    SELECT id_city_1, id_city_2, distance 
    FROM connections 
    WHERE id_city_1 IN ($ids_str) AND id_city_2 IN ($ids_str)
    """
    result = DBInterface.execute(db, query) |> DataFrame
    SQLite.close(db)
    return result
end

"""
    construir_tsp_desde_db(db_path::String, city_ids::Vector{Int}) -> TSP

Construye el problema TSP a partir de la base de datos y los IDs de las ciudades.
"""
function construir_tsp_desde_db(db_path::String, city_ids::Vector{Int})
    ciudades, city_id_to_index = cargar_ciudades_desde_db(db_path, city_ids)
    n = length(ciudades)
    grafica = falses(n, n)
    distancias = zeros(Float64, n, n)
    conexiones = cargar_conexiones_desde_db(db_path, city_ids)
    for row in eachrow(conexiones)
        id1 = row.id_city_1
        id2 = row.id_city_2
        if haskey(city_id_to_index, id1) && haskey(city_id_to_index, id2)
            i = city_id_to_index[id1]
            j = city_id_to_index[id2]
            grafica[i, j] = true
            grafica[j, i] = true
            if !ismissing(row.distance) && row.distance > 0
                distancias[i, j] = row.distance
                distancias[j, i] = row.distance
            end
        end
    end

    # Calcula la distancia máxima de las conexiones existentes
    max_dist = 0.0
    for i in 1:n
        for j in 1:n
            if grafica[i, j] && distancias[i, j] > max_dist
                max_dist = distancias[i, j]
            end
        end
    end

    # Calcula distancias naturales penalizadas para pares sin conexión
    for i in 1:n
        for j in 1:n
            if i != j && !grafica[i, j]
                dist_natural = distancia_natural(ciudades[i], ciudades[j])
                distancias[i, j] = dist_natural * max_dist
            end
        end
    end

    # Calcula el normalizador según la definición 4.3.1
    distancias_aristas_existentes = Float64[]
    for i in 1:n
        for j in i+1:n
            if grafica[i, j]
                push!(distancias_aristas_existentes, distancias[i, j])
            end
        end
    end
    sort!(distancias_aristas_existentes, rev=true)
    k = length(ciudades)
    if length(distancias_aristas_existentes) <= k - 1
        L_prima = distancias_aristas_existentes
    else
        L_prima = distancias_aristas_existentes[1:(k-1)]
    end
    normalizador = length(L_prima) > 0 ? sum(L_prima) : 1.0

    return TSP(ciudades, grafica, distancias, max_dist, normalizador, city_id_to_index)
end

"""
    encontrar_archivo_tsp() -> String

Devuelve la ruta del archivo .tsp de entrada.
"""
function encontrar_archivo_tsp()
    "src/inputs/input-150.tsp"
end