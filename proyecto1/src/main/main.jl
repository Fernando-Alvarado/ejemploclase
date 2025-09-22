# src/main/main.jl - Punto de entrada principal del programa
using Random
using SQLite
using DataFrames
using StatsBase
using Base.Threads

# Incluir archivos locales
include("estructuras.jl")
include("recocido_simulado.jl")
include("database_reader.jl")
include("utilidades.jl")
include("vecinos.jl")
include("temperatura.jl")

const output_lock = ReentrantLock()

function main()
    if length(ARGS) == 0
        printUso()
        return
    end

    # Crear directorio de salida en la raíz del proyecto
    mkpath("results/soluciones")
    
    println("Cargando instancia TSP...")
    archivo_tsp = encontrar_archivo_tsp()
    tsp_input = leer_archivo_tsp(archivo_tsp)
    tsp = construir_tsp_desde_db("src/bd/tsp.db", tsp_input.path)
    solucion_base = ids_a_permutacion(tsp, tsp_input.path)
    
    println("Instancia cargada: $(length(tsp.ciudades)) ciudades")
    
    semillas = parse_argumentos(ARGS)
    ejecutar_semillas(semillas, tsp, solucion_base, archivo_tsp)
end

function ejecutar_semillas(semillas::Vector{Int}, tsp::TSP, solucion_base::Vector{Int}, archivo_tsp::String)
    println("Ejecutando $(length(semillas)) semillas en $(nthreads()) hilos...")
    
    resultados = Vector{Any}(undef, length(semillas))
    
    if length(semillas) == 1
        resultados[1] = ejecutar_semilla(semillas[1], tsp, solucion_base, archivo_tsp)
    else
        @sync begin
            for i in 1:length(semillas)
                @spawn begin
                    resultados[i] = ejecutar_semilla(semillas[i], tsp, copy(solucion_base), archivo_tsp)
                end
            end
        end
    end
    
    mostrar_resumen(resultados)
end

function ejecutar_semilla(semilla::Int, tsp::TSP, solucion_base::Vector{Int}, archivo_tsp::String)
    println("Hilo $(threadid()): Ejecutando semilla $semilla")
    Random.seed!(semilla)
    
    solucion_inicial = shuffle(copy(solucion_base))
    
    mejor_solucion, costo_normalizado = aceptacion_por_umbrales(
        tsp, 
        solucion_inicial,
        L=8000,
        φ=0.9,
        ε=0.001,
        max_iteraciones=100000
    )
    mejor_solucion, costo_normalizado = barrido(tsp, mejor_solucion, costo_normalizado)
    
    mejor_path_ids = permutacion_a_ids(tsp, mejor_solucion)
    factible = es_factible(tsp, mejor_solucion) ? "YES" : "NO"
    
    # Escribir a archivo específico de la semilla
    nombre_archivo = "results/soluciones/salida_$semilla.tsp"
    open(nombre_archivo, "w") do f
        println(f, "Filename: $archivo_tsp")
        println(f, "Path: $(join(mejor_path_ids, ","))")
        println(f, "Maximum: $(tsp.max_distancia)")
        println(f, "Normalizer: $(tsp.normalizador)")
        println(f, "Evaluation: $costo_normalizado")
        println(f, "Feasible: $factible")
        println(f, "Seed: $semilla")
    end
    
    # Mostrar progreso en pantalla
    lock(output_lock) do
        println("Semilla $semilla completada - Costo: $costo_normalizado - Archivo: $nombre_archivo")
    end
    
    return (semilla, costo_normalizado, factible, mejor_path_ids, archivo_tsp, tsp.max_distancia, tsp.normalizador)
end

function mostrar_resumen(resultados)
    if length(resultados) == 1
        return
    else
        println()
        println("=" ^ 60)
        println("RESUMEN DE EJECUCIÓN PARALELA")
        
        mejor_idx = argmin([r[2] for r in resultados])
        mejor = resultados[mejor_idx]
        
        costos = [r[2] for r in resultados]
        factibles = [r[3] == "YES" for r in resultados]
        
        println("Total ejecutadas: $(length(resultados))")
        println("Mejor costo: $(minimum(costos)) (Semilla $(mejor[1]))")
   
        println("\nMEJOR SOLUCIÓN ENCONTRADA:")
        println("Filename: $(mejor[5])")
        println("Path: $(join(mejor[4], ","))")
        println("Maximum: $(mejor[6])")
        println("Normalizer: $(mejor[7])")
        println("Evaluation: $(mejor[2])")
        println("Feasible: $(mejor[3])")
        println("Seed: $(mejor[1])")
        println("=" ^ 60)
    end
end

function parse_argumentos(args::Vector{String})
    if length(args) == 1
        return [parse(Int, args[1])]
    elseif length(args) == 3 && args[2] == "-"
        inicio = parse(Int, args[1])
        fin = parse(Int, args[3])
        return collect(inicio:fin)
    else
        return [parse(Int, arg) for arg in args]
    end
end

function printUso()
    println("Uso del programa:")
    println("  julia run.jl <semilla>")
    println("  julia --threads=auto run.jl <inicio> - <fin>")
    println("  julia --threads=auto run.jl <semilla1> <semilla2> ... <semillan>")
    println()
    println("Hilos disponibles: $(nthreads())")
end

# Ejecutar siempre que tenga argumentos, sin importar cómo se llame
if length(ARGS) > 0
    try
        main()
    catch e
        println("Error: $e")
        printUso()
    end
else
    printUso()
end