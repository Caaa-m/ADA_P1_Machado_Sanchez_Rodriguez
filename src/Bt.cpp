// Modulo BT - Backtracking (version con hilos)
// Construccion incremental de contraseñas que cumplen una politica de
// composicion (Seccion 5, 6.2, 9.2 del enunciado), con poda por
// factibilidad, y una version sin poda para la comparacion de la
// Seccion 8.2. Ambas versiones estan paralelizadas: la configuracion de
// hilos y la recursion viven en BtThreads.hpp (separado, mismo patron
// usado en el Modulo FB con FbThreads.hpp).
//
// Compilar: g++ -std=c++17 -O2 -pthread -o bt src/Bt.cpp
//
// Uso:
//   ./bt variante --id i|ii|iii|iv|v [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]
//   ./bt referencia [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]
//   ./bt custom --minLower L --minUpper U --minDigit D --minSymbol S --n N [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]
//
// --threads N    : numero de hilos a usar. Si se omite, se detecta
//                  automaticamente con std::thread::hardware_concurrency().
// --maxnodes N   : aborta la busqueda tras visitar N nodos en total (sumado
//                  entre todos los hilos). Util para reportar progreso
//                  parcial en instancias intratables; marca completo=NO.
// --stopfirst    : detiene TODA la busqueda (todos los hilos) apenas se
//                  junten --examples K soluciones validas entre todos los
//                  hilos. Util para validar rapido que el algoritmo
//                  funciona, sin contar todas las soluciones.
//
// ADVERTENCIA: --sinpoda recorre el arbol completo hasta las hojas. Es
// intratable para n grande o politicas poco restrictivas (69^n crece muy
// rapido). Usar solo en instancias pequeñas, o con --maxnodes para acotar.
//
// ADVERTENCIA 2: incluso CON poda, si los minimos de la politica son bajos
// respecto de n (ej. la instancia de referencia: minimos suman 5 de n=6),
// la poda por factibilidad actua tarde y el arbol visitado puede seguir
// siendo del orden de |Sigma|^n. Para VALIDAR la implementacion rapido,
// usar --examples K --stopfirst en vez de esperar el conteo completo.

#include "BtAlphabet.hpp"
#include "BtThreads.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// ---------- Ejecucion y reporte de una instancia ----------

static void correrInstancia(const std::string& nombre, const PoliticaBT& pol, int n,
                             bool sinPoda, int ejemplosMax, uint64_t maxNodes, bool stopFirst,
                             int numHilos) {
    ResultadoBTHilos r = sinPoda
        ? backtrackSinPodaParalela(n, pol, numHilos, ejemplosMax, maxNodes, stopFirst)
        : backtrackConPodaParalela(n, pol, numHilos, ejemplosMax, maxNodes, stopFirst);

    std::cout << "== " << nombre << (sinPoda ? " (SIN poda)" : " (CON poda)") << " ==\n";
    std::cout << "n=" << n << " |Sigma|=" << BT_ALPHABET.size() << " hilos=" << numHilos
              << " minLower=" << pol.minLower << " minUpper=" << pol.minUpper
              << " minDigit=" << pol.minDigit << " minSymbol=" << pol.minSymbol
              << " sinRepetidos=" << (pol.sinRepetidosConsecutivos ? "si" : "no") << "\n";
    std::cout << "completo=" << (r.completo ? "si" : "NO (abortado por limite)") << "\n";
    std::cout << "nodesVisited=" << r.nodesVisited << "\n";
    std::cout << "leavesChecked=" << r.leavesChecked << "\n";
    std::cout << "validFound=" << r.validFound << "\n";
    std::cout << "prunedBranches=" << r.prunedBranches << "\n";
    std::cout << "time_ms=" << r.tiempo_ms << "\n";
    std::cout << "CSV," << nombre << "," << (sinPoda ? "sinpoda" : "conpoda") << "," << n << ","
              << BT_ALPHABET.size() << "," << numHilos << "," << r.nodesVisited << ","
              << r.leavesChecked << "," << r.validFound << "," << r.prunedBranches << ","
              << r.tiempo_ms << "," << (r.completo ? 1 : 0) << "\n";

    if (!r.ejemplos.empty()) {
        std::cout << "Ejemplos:\n";
        for (auto& s : r.ejemplos) std::cout << "  " << s << "\n";
    }
    std::cout << "\n";
}

// ---------- main ----------

int bt_main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso:\n"
                  << "  " << argv[0] << " variante --id i|ii|iii|iv|v [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]\n"
                  << "  " << argv[0] << " referencia [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]\n"
                  << "  " << argv[0] << " custom --minLower L --minUpper U --minDigit D --minSymbol S --n N [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]\n";
        return 1;
    }

    std::string modo = argv[1];
    int ejemplosMax = 0;
    bool sinPoda = false;
    bool stopFirst = false;
    uint64_t maxNodes = 0;
    int numHilosPedidos = 0; // 0 = detectar automaticamente
    std::string idVariante;
    int minLower = -1, minUpper = -1, minDigit = -1, minSymbol = -1, nCustom = -1;

    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--examples" && i + 1 < argc) ejemplosMax = std::atoi(argv[++i]);
        else if (a == "--sinpoda") sinPoda = true;
        else if (a == "--stopfirst") stopFirst = true;
        else if (a == "--maxnodes" && i + 1 < argc) maxNodes = std::strtoull(argv[++i], nullptr, 10);
        else if (a == "--threads" && i + 1 < argc) numHilosPedidos = std::atoi(argv[++i]);
        else if (a == "--id" && i + 1 < argc) idVariante = argv[++i];
        else if (a == "--minLower" && i + 1 < argc) minLower = std::atoi(argv[++i]);
        else if (a == "--minUpper" && i + 1 < argc) minUpper = std::atoi(argv[++i]);
        else if (a == "--minDigit" && i + 1 < argc) minDigit = std::atoi(argv[++i]);
        else if (a == "--minSymbol" && i + 1 < argc) minSymbol = std::atoi(argv[++i]);
        else if (a == "--n" && i + 1 < argc) nCustom = std::atoi(argv[++i]);
    }

    int numHilos = configurarNumHilosBT(numHilosPedidos);
    PoliticaBT politicaEquipo = politicaDelEquipo(SEMILLA_EQUIPO);

    if (modo == "referencia") {
        correrInstancia("referencia_n6", politicaReferencia(), 6, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);

    } else if (modo == "variante") {
        if (idVariante == "i") {
            correrInstancia("equipo_i_n8", politicaEquipo, 8, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);
        } else if (idVariante == "ii") {
            correrInstancia("equipo_ii_n6", politicaEquipo, 6, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);
        } else if (idVariante == "iii") {
            correrInstancia("equipo_iii_n10", politicaEquipo, 10, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);
        } else if (idVariante == "iv") {
            correrInstancia("equipo_iv_relajada_n8", politicaRelajada(), 8, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);
        } else if (idVariante == "v") {
            correrInstancia("equipo_v_podanula_n6", politicaVacia(), 6, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);
        } else {
            std::cerr << "Variante invalida: usa i, ii, iii, iv o v\n";
            return 1;
        }

    } else if (modo == "custom") {
        if (minLower < 0 || minUpper < 0 || minDigit < 0 || minSymbol < 0 || nCustom < 0) {
            std::cerr << "Faltan argumentos para custom.\n";
            return 1;
        }
        PoliticaBT pol{minLower, minUpper, minDigit, minSymbol, true};
        correrInstancia("custom", pol, nCustom, sinPoda, ejemplosMax, maxNodes, stopFirst, numHilos);

    } else {
        std::cerr << "Modo desconocido: " << modo << "\n";
        return 1;
    }

    return 0;
}
