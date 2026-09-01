#pragma once
// Configuracion y logica de paralelizacion del Modulo BT.
//
// Contiene:
//  - deteccion/config del numero de hilos a usar
//  - factibilidad, verificacion de politica (comparten logica con la
//    version secuencial, pero trabajan sobre vector<string> de tokens
//    para soportar la ñ/Ñ de 2 bytes UTF-8, ver BtAlphabet.hpp)
//  - la recursion de backtracking (con poda y sin poda), preparada para
//    correr dentro de un hilo
//  - la orquestacion: lanzar hilos, esperar, consolidar resultados
//
// Paralelizacion: se particiona el arbol por el PRIMER simbolo (la raiz).
// Cada hilo recorre los indices del alfabeto tales que indice % totalHilos
// == idHilo, y bajo esa rama corre la recursion normal (con o sin poda).
// La union de las particiones cubre el arbol completo sin solapamiento.
//
// Los contadores (nodesVisited, leavesChecked, validFound, prunedBranches)
// son atomicos porque los escriben todos los hilos concurrentemente. La
// lista de ejemplos esta protegida por un mutex porque insertar en un
// vector no es seguro entre hilos.

#include "BtAlphabet.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// ---------- Configuracion de hilos ----------

inline int configurarNumHilosBT(int numHilosPedidos) {
    if (numHilosPedidos > 0) return numHilosPedidos;
    int detectados = (int)std::thread::hardware_concurrency();
    if (detectados <= 0) detectados = 4; // valor de respaldo
    return detectados;
}

// ---------- Estado parcial y factibilidad (poda) ----------

struct EstadoParcialBT {
    int nLower = 0, nUpper = 0, nDigit = 0, nSymbol = 0;
    std::string ultimo = "";
};

inline bool esFactibleBT(const EstadoParcialBT& e, int posicionesRestantes, const PoliticaBT& pol) {
    int faltanLower  = std::max(0, pol.minLower  - e.nLower);
    int faltanUpper  = std::max(0, pol.minUpper  - e.nUpper);
    int faltanDigit  = std::max(0, pol.minDigit  - e.nDigit);
    int faltanSymbol = std::max(0, pol.minSymbol - e.nSymbol);
    return (faltanLower + faltanUpper + faltanDigit + faltanSymbol) <= posicionesRestantes;
}

inline std::string unirTokensBT(const std::vector<std::string>& tokens) {
    std::string s;
    for (auto& t : tokens) s += t;
    return s;
}

inline bool cumplePoliticaBT(const std::vector<std::string>& s, const PoliticaBT& pol) {
    int nLower = 0, nUpper = 0, nDigit = 0, nSymbol = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        switch (claseDe(s[i])) {
            case ClaseChar::MINUSCULA: nLower++; break;
            case ClaseChar::MAYUSCULA: nUpper++; break;
            case ClaseChar::DIGITO:    nDigit++; break;
            case ClaseChar::SIMBOLO:   nSymbol++; break;
        }
        if (pol.sinRepetidosConsecutivos && i > 0 && s[i] == s[i - 1]) return false;
    }
    return nLower >= pol.minLower && nUpper >= pol.minUpper &&
           nDigit >= pol.minDigit && nSymbol >= pol.minSymbol;
}

// ---------- Estadisticas compartidas entre hilos (atomicas) ----------

struct StatsAtomicBT {
    std::atomic<uint64_t> nodesVisited{0};
    std::atomic<uint64_t> leavesChecked{0};
    std::atomic<uint64_t> validFound{0};
    std::atomic<uint64_t> prunedBranches{0};
    std::atomic<bool> abortar{false};
    std::atomic<bool> completo{true};
};

struct ResultadoBTHilos {
    uint64_t nodesVisited = 0;
    uint64_t leavesChecked = 0;
    uint64_t validFound = 0;
    uint64_t prunedBranches = 0;
    bool completo = true;
    double tiempo_ms = 0.0;
    std::vector<std::string> ejemplos;
};

// ---------- Recursion CON poda (corre dentro de un hilo) ----------
//
// El primer nivel (actual.empty(), es decir la raiz) se particiona entre
// hilos: cada hilo solo itera los indices del alfabeto que le corresponden
// (idx % totalHilos == idHilo). En los niveles siguientes se itera el
// alfabeto completo normalmente, como en la version secuencial.

inline void backtrackConPodaRec(std::vector<std::string>& actual, EstadoParcialBT estado, int n,
                                 const PoliticaBT& pol, StatsAtomicBT& st,
                                 int ejemplosMax, std::vector<std::string>& ejemplosCompartidos,
                                 std::mutex& mtxEjemplos, uint64_t maxNodes, bool stopFirst,
                                 int idHilo, int totalHilos) {
    if (st.abortar.load(std::memory_order_relaxed)) return;

    bool esRaiz = actual.empty();

    // El nodo raiz (prefijo vacio) es UNO solo conceptualmente, aunque los
    // "totalHilos" hilos entren a la recursion desde ahi; solo se cuenta
    // una vez (lo hace el orquestador antes de lanzar los hilos), para no
    // inflar nodesVisited en (totalHilos - 1).
    if (!esRaiz) {
        uint64_t nv = st.nodesVisited.fetch_add(1, std::memory_order_relaxed) + 1;
        if (maxNodes > 0 && nv >= maxNodes) {
            st.completo.store(false, std::memory_order_relaxed);
            st.abortar.store(true, std::memory_order_relaxed);
            return;
        }
    }

    if ((int)actual.size() == n) {
        st.leavesChecked.fetch_add(1, std::memory_order_relaxed);
        if (cumplePoliticaBT(actual, pol)) {
            uint64_t vf = st.validFound.fetch_add(1, std::memory_order_relaxed) + 1;
            if (ejemplosMax > 0) {
                std::lock_guard<std::mutex> lock(mtxEjemplos);
                if ((int)ejemplosCompartidos.size() < ejemplosMax)
                    ejemplosCompartidos.push_back(unirTokensBT(actual));
            }
            if (stopFirst && ejemplosMax > 0 && vf >= (uint64_t)ejemplosMax) {
                st.completo.store(false, std::memory_order_relaxed);
                st.abortar.store(true, std::memory_order_relaxed);
            }
        }
        return;
    }

    int base = (int)BT_ALPHABET.size();
    for (int idx = (esRaiz ? idHilo : 0); idx < base; idx += (esRaiz ? totalHilos : 1)) {
        if (st.abortar.load(std::memory_order_relaxed)) return;
        const std::string& tok = BT_ALPHABET[idx];

        // Poda 1: no repetir el simbolo inmediatamente anterior
        if (pol.sinRepetidosConsecutivos && !actual.empty() && tok == actual.back()) {
            st.prunedBranches.fetch_add(1, std::memory_order_relaxed);
            continue;
        }

        EstadoParcialBT siguiente = estado;
        switch (claseDe(tok)) {
            case ClaseChar::MINUSCULA: siguiente.nLower++; break;
            case ClaseChar::MAYUSCULA: siguiente.nUpper++; break;
            case ClaseChar::DIGITO:    siguiente.nDigit++; break;
            case ClaseChar::SIMBOLO:   siguiente.nSymbol++; break;
        }
        siguiente.ultimo = tok;

        int restantes = n - (int)actual.size() - 1;

        // Poda 2: factibilidad de completar los minimos de la politica
        if (!esFactibleBT(siguiente, restantes, pol)) {
            st.prunedBranches.fetch_add(1, std::memory_order_relaxed);
            continue;
        }

        actual.push_back(tok);
        backtrackConPodaRec(actual, siguiente, n, pol, st, ejemplosMax, ejemplosCompartidos,
                             mtxEjemplos, maxNodes, stopFirst, idHilo, totalHilos);
        actual.pop_back();
    }
}

// ---------- Recursion SIN poda (Seccion 8.2), corre dentro de un hilo ----------

inline void backtrackSinPodaRec(std::vector<std::string>& actual, int n, const PoliticaBT& pol,
                                 StatsAtomicBT& st, int ejemplosMax,
                                 std::vector<std::string>& ejemplosCompartidos, std::mutex& mtxEjemplos,
                                 uint64_t maxNodes, bool stopFirst, int idHilo, int totalHilos) {
    if (st.abortar.load(std::memory_order_relaxed)) return;

    bool esRaiz = actual.empty();

    // Igual que en backtrackConPodaRec: la raiz se cuenta una sola vez
    // (la cuenta el orquestador antes de lanzar los hilos), no una vez
    // por hilo.
    if (!esRaiz) {
        uint64_t nv = st.nodesVisited.fetch_add(1, std::memory_order_relaxed) + 1;
        if (maxNodes > 0 && nv >= maxNodes) {
            st.completo.store(false, std::memory_order_relaxed);
            st.abortar.store(true, std::memory_order_relaxed);
            return;
        }
    }

    if ((int)actual.size() == n) {
        st.leavesChecked.fetch_add(1, std::memory_order_relaxed);
        if (cumplePoliticaBT(actual, pol)) {
            uint64_t vf = st.validFound.fetch_add(1, std::memory_order_relaxed) + 1;
            if (ejemplosMax > 0) {
                std::lock_guard<std::mutex> lock(mtxEjemplos);
                if ((int)ejemplosCompartidos.size() < ejemplosMax)
                    ejemplosCompartidos.push_back(unirTokensBT(actual));
            }
            if (stopFirst && ejemplosMax > 0 && vf >= (uint64_t)ejemplosMax) {
                st.completo.store(false, std::memory_order_relaxed);
                st.abortar.store(true, std::memory_order_relaxed);
            }
        }
        return;
    }

    int base = (int)BT_ALPHABET.size();
    for (int idx = (esRaiz ? idHilo : 0); idx < base; idx += (esRaiz ? totalHilos : 1)) {
        if (st.abortar.load(std::memory_order_relaxed)) return;
        actual.push_back(BT_ALPHABET[idx]);
        backtrackSinPodaRec(actual, n, pol, st, ejemplosMax, ejemplosCompartidos, mtxEjemplos,
                             maxNodes, stopFirst, idHilo, totalHilos);
        actual.pop_back();
    }
}

// ---------- Orquestacion: lanzar hilos, esperar, consolidar resultado ----------

inline ResultadoBTHilos backtrackConPodaParalela(int n, const PoliticaBT& pol, int numHilos,
                                                  int ejemplosMax, uint64_t maxNodes, bool stopFirst) {
    StatsAtomicBT st;
    st.nodesVisited.store(1); // el nodo raiz (prefijo vacio) se cuenta una sola vez
    std::vector<std::string> ejemplos;
    std::mutex mtxEjemplos;
    std::vector<std::thread> hilos;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int h = 0; h < numHilos; ++h) {
        hilos.emplace_back([&, h]() {
            std::vector<std::string> actual;
            EstadoParcialBT estado;
            backtrackConPodaRec(actual, estado, n, pol, st, ejemplosMax, ejemplos, mtxEjemplos,
                                 maxNodes, stopFirst, h, numHilos);
        });
    }
    for (auto& t : hilos) t.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    ResultadoBTHilos r;
    r.tiempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    r.nodesVisited = st.nodesVisited.load();
    r.leavesChecked = st.leavesChecked.load();
    r.validFound = st.validFound.load();
    r.prunedBranches = st.prunedBranches.load();
    r.completo = st.completo.load();
    r.ejemplos = ejemplos;
    return r;
}

inline ResultadoBTHilos backtrackSinPodaParalela(int n, const PoliticaBT& pol, int numHilos,
                                                  int ejemplosMax, uint64_t maxNodes, bool stopFirst) {
    StatsAtomicBT st;
    st.nodesVisited.store(1); // el nodo raiz (prefijo vacio) se cuenta una sola vez
    std::vector<std::string> ejemplos;
    std::mutex mtxEjemplos;
    std::vector<std::thread> hilos;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int h = 0; h < numHilos; ++h) {
        hilos.emplace_back([&, h]() {
            std::vector<std::string> actual;
            backtrackSinPodaRec(actual, n, pol, st, ejemplosMax, ejemplos, mtxEjemplos,
                                 maxNodes, stopFirst, h, numHilos);
        });
    }
    for (auto& t : hilos) t.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    ResultadoBTHilos r;
    r.tiempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    r.nodesVisited = st.nodesVisited.load();
    r.leavesChecked = st.leavesChecked.load();
    r.validFound = st.validFound.load();
    r.prunedBranches = st.prunedBranches.load();
    r.completo = st.completo.load();
    r.ejemplos = ejemplos;
    return r;
}
