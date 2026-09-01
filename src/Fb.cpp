// Modulo FB - Fuerza Bruta
// Ataque por enumeracion exhaustiva contra un hash SHA-256 objetivo,
// y ataque por diccionario para comparacion (Seccion 8.1).
//
// La configuracion de hilos y la logica de paralelizacion viven en
// FbThreads.hpp (separado para mantener este archivo enfocado en el
// CLI y los modos de ataque).
//
// Compilar: g++ -std=c++17 -O2 -pthread -o fb src/Fb.cpp
//
// Uso (fuerza bruta):
//   ./fb bruteforce --hash <hex> --alphabet a1|a2 --n <len> [--threads N]
//
// Uso (diccionario):
//   ./fb dictionary --hash <hex> --dict resources/diccionario.txt
//
// Uso (batch, corre las instancias de referencia + equipo y escribe CSV):
//   ./fb batch --out results/fb_results.csv

#include "FbAlphabets.hpp"
#include "FbThreads.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// ---------- Modulo FB - Seccion 8.1: ataque por diccionario ----------
// No exhaustivo: solo prueba los candidatos listados en el diccionario.
// Puede ser mucho mas rapido, pero no garantiza encontrar la contraseña
// si esta no pertenece a la lista.

static ResultadoAtaque ataquePorDiccionario(const std::string& hashObjetivo,
                                             const std::string& rutaDiccionario) {
    ResultadoAtaque r;
    std::ifstream in(rutaDiccionario);
    if (!in) {
        std::cerr << "No se pudo abrir el diccionario: " << rutaDiccionario << "\n";
        return r;
    }
    std::string linea;
    auto t0 = std::chrono::high_resolution_clock::now();
    while (std::getline(in, linea)) {
        if (linea.empty()) continue;
        r.candidatosEvaluados++;
        if (sha256Hex(linea) == hashObjetivo) {
            r.encontrada = true;
            r.texto_plano = linea;
            break;
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    r.tiempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return r;
}

// ---------- Instancias fijas del enunciado (Seccion 9.1) ----------

struct Instancia {
    std::string nombre;
    std::string hash;
    std::string alfabeto; // referencia al string A1 o A2
    int n;
};

static void imprimirResultado(const Instancia& inst, const ResultadoAtaque& r, int hilos) {
    std::cout << "== " << inst.nombre << " ==\n";
    std::cout << "n=" << inst.n << " |Sigma|=" << inst.alfabeto.size()
              << " hilos=" << hilos << "\n";
    std::cout << "encontrada=" << (r.encontrada ? "si" : "no") << "\n";
    if (r.encontrada) std::cout << "texto_plano=" << r.texto_plano << "\n";
    std::cout << "candidatosEvaluados=" << r.candidatosEvaluados << "\n";
    std::cout << "tiempo_ms=" << r.tiempo_ms << "\n";
    std::cout << "CSV," << inst.nombre << "," << inst.n << "," << inst.alfabeto.size() << ","
              << hilos << "," << (r.encontrada ? 1 : 0) << "," << r.candidatosEvaluados << ","
              << r.tiempo_ms << "\n\n";
}

// ---------- main ----------

int fb_main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso:\n"
                  << "  " << argv[0] << " bruteforce --hash <hex> --alphabet a1|a2 --n <len> [--threads N]\n"
                  << "  " << argv[0] << " dictionary --hash <hex> --dict <ruta>\n"
                  << "  " << argv[0] << " batch --out <ruta_csv>\n";
        return 1;
    }

    std::string modo = argv[1];
    int numHilosPedidos = 0; // 0 = detectar automaticamente

    std::string hashObjetivo, alfabetoNombre, rutaDiccionario, rutaSalida;
    int n = 0;

    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--hash" && i + 1 < argc) hashObjetivo = argv[++i];
        else if (a == "--alphabet" && i + 1 < argc) alfabetoNombre = argv[++i];
        else if (a == "--n" && i + 1 < argc) n = std::atoi(argv[++i]);
        else if (a == "--threads" && i + 1 < argc) numHilosPedidos = std::atoi(argv[++i]);
        else if (a == "--dict" && i + 1 < argc) rutaDiccionario = argv[++i];
        else if (a == "--out" && i + 1 < argc) rutaSalida = argv[++i];
    }

    int numHilos = configurarNumHilos(numHilosPedidos);

    if (modo == "bruteforce") {
        if (hashObjetivo.empty() || alfabetoNombre.empty() || n <= 0) {
            std::cerr << "Faltan argumentos para bruteforce.\n";
            return 1;
        }
        std::string alfabeto;
        if (alfabetoNombre == "a1") {
            if (!esLongitudValidaA1(n)) std::cerr << "Advertencia: n fuera del rango recomendado para A1.\n";
            alfabeto = A1;
        } else if (alfabetoNombre == "a2") {
            if (!esLongitudValidaA2(n)) std::cerr << "Advertencia: n fuera del rango recomendado para A2.\n";
            alfabeto = A2;
        } else {
            std::cerr << "Alfabeto invalido: usa a1 o a2\n";
            return 1;
        }

        Instancia inst{"bruteforce", hashObjetivo, alfabeto, n};
        ResultadoAtaque r = fuerzaBrutaParalela(hashObjetivo, alfabeto, n, numHilos);
        imprimirResultado(inst, r, numHilos);

    } else if (modo == "dictionary") {
        if (hashObjetivo.empty() || rutaDiccionario.empty()) {
            std::cerr << "Faltan argumentos para dictionary.\n";
            return 1;
        }
        ResultadoAtaque r = ataquePorDiccionario(hashObjetivo, rutaDiccionario);
        std::cout << "== dictionary ==\n";
        std::cout << "encontrada=" << (r.encontrada ? "si" : "no") << "\n";
        if (r.encontrada) std::cout << "texto_plano=" << r.texto_plano << "\n";
        std::cout << "candidatosEvaluados=" << r.candidatosEvaluados << "\n";
        std::cout << "tiempo_ms=" << r.tiempo_ms << "\n";
        std::cout << "CSV,dictionary," << rutaDiccionario << "," << (r.encontrada ? 1 : 0) << ","
                  << r.candidatosEvaluados << "," << r.tiempo_ms << "\n";

    } else if (modo == "batch") {
        // Instancia de referencia comun a todo el curso, mas las 5 instancias
        // propias del equipo (semilla 4613, Seccion 9.1).
        std::vector<Instancia> instancias = {
            {"referencia_abc12_A2_n5",
             "8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773",
             A2, 5},
            // Alfabetos alternados A1,A2,A1,A2,A1 (n=6 solo es valido en A1)
            {"equipo_1_A1_n4",
             "22072ff7764a1f68cb9d05e287d907e5e52cd88a6b9960f28b97b905cdb4a5a2",
             A1, 4},
            {"equipo_2_A2_n4",
             "5b78a6425aedd17a1683c982597afc0968030da8b1106b09fa7aaabecb85e5e4",
             A2, 4},
            {"equipo_3_A1_n5",
             "8308f1219366f836697fa47cd231ae591e6ef7ea09a09a49a0130f7fe1c104cb",
             A1, 5},
            {"equipo_4_A2_n5",
             "81eb081297b8c50d7f27a7ea2ed3b9320fc253ef02338ade6ba727c50e37d5d3",
             A2, 5},
            {"equipo_5_A1_n6",
             "dd03c5067c571911c07bc6fcab16a480d85e5a2ec2fc9200b01303ae8ec1d9aa",
             A1, 6}
        };

        std::ofstream out;
        if (!rutaSalida.empty()) {
            out.open(rutaSalida);
            out << "instancia,n,alfabeto_size,hilos,encontrada,candidatos,tiempo_ms\n";
        }

        for (auto& inst : instancias) {
            ResultadoAtaque r = fuerzaBrutaParalela(inst.hash, inst.alfabeto, inst.n, numHilos);
            imprimirResultado(inst, r, numHilos);
            if (out.is_open()) {
                out << inst.nombre << "," << inst.n << "," << inst.alfabeto.size() << ","
                    << numHilos << "," << (r.encontrada ? 1 : 0) << "," << r.candidatosEvaluados
                    << "," << r.tiempo_ms << "\n";
            }
        }
    } else {
        std::cerr << "Modo desconocido: " << modo << "\n";
        return 1;
    }

    return 0;
}
