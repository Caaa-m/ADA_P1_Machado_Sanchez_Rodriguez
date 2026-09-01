#pragma once
// Configuracion y logica de paralelizacion del Modulo FB.
//
// Contiene:
//  - deteccion/config del numero de hilos a usar
//  - el trabajo que ejecuta cada hilo (particion del espacio por primer caracter)
//  - la orquestacion (lanzar hilos, esperar, consolidar resultado)
//
// Paralelizacion: se particiona el espacio por el primer caracter. Cada hilo
// recorre un subconjunto disjunto de valores del primer indice (round robin),
// y dentro de ese primer caracter usa su propio odometro sobre las n-1
// posiciones restantes. La union de los subconjuntos es el espacio completo
// y no hay solapamiento entre hilos.

#include "third_party/picosha2.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

// ---------- Configuracion de hilos ----------

// Determina cuantos hilos usar: si el usuario especifico --threads N (numHilosPedidos > 0)
// se respeta ese valor; si no, se detecta automaticamente con hardware_concurrency().
// Si la deteccion falla (devuelve 0, puede pasar en algunos entornos), se usa un
// valor de respaldo razonable.
inline int configurarNumHilos(int numHilosPedidos) {
    if (numHilosPedidos > 0) return numHilosPedidos;
    int detectados = (int)std::thread::hardware_concurrency();
    if (detectados <= 0) detectados = 4; // valor de respaldo
    return detectados;
}

// ---------- Utilidades ----------

inline std::string sha256Hex(const std::string& s) {
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(s.begin(), s.end(), hash.begin(), hash.end());
    return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

struct ResultadoAtaque {
    bool encontrada = false;
    std::string texto_plano;
    uint64_t candidatosEvaluados = 0;
    double tiempo_ms = 0.0;
};

// ---------- Trabajo de un hilo individual ----------
//
// Enumera Sigma^n con un contador tipo "odometro": un arreglo de n indices,
// cada uno en [0, |Sigma|), que se incrementa como un contador en base |Sigma|.
// Esto garantiza generar cada cadena exactamente una vez, sin omisiones ni
// repeticiones, sin necesidad de recursion.

inline void trabajoHilo(const std::string& alfabeto,
                         int n,
                         const std::string& hashObjetivo,
                         int idHilo,
                         int totalHilos,
                         std::atomic<bool>& encontrado,
                         std::atomic<uint64_t>& contadorGlobal,
                         std::string& resultadoTexto) {
    int base = (int)alfabeto.size();
    std::string candidato(n, alfabeto[0]);

    // Cada hilo atiende los primeros-caracteres cuyo indice % totalHilos == idHilo
    for (int primero = idHilo; primero < base; primero += totalHilos) {
        if (encontrado.load(std::memory_order_relaxed)) return;
        candidato[0] = alfabeto[primero];

        if (n == 1) {
            uint64_t local = 1;
            if (sha256Hex(candidato) == hashObjetivo) {
                resultadoTexto = candidato;
                encontrado.store(true, std::memory_order_relaxed);
            }
            contadorGlobal.fetch_add(local, std::memory_order_relaxed);
            continue;
        }

        // Odometro sobre las posiciones 1..n-1
        std::vector<int> idx(n - 1, 0);
        uint64_t localCount = 0;
        while (true) {
            for (int i = 0; i < n - 1; ++i) candidato[i + 1] = alfabeto[idx[i]];

            if (sha256Hex(candidato) == hashObjetivo) {
                resultadoTexto = candidato;
                encontrado.store(true, std::memory_order_relaxed);
                contadorGlobal.fetch_add(localCount + 1, std::memory_order_relaxed);
                return;
            }
            ++localCount;

            // Chequeo periodico de corte temprano (otro hilo ya encontro la respuesta)
            if ((localCount & 0xFFFF) == 0 && encontrado.load(std::memory_order_relaxed)) {
                contadorGlobal.fetch_add(localCount, std::memory_order_relaxed);
                return;
            }

            // Incrementar odometro (posicion mas a la derecha primero)
            int pos = n - 2;
            while (pos >= 0) {
                idx[pos]++;
                if (idx[pos] < base) break;
                idx[pos] = 0;
                pos--;
            }
            if (pos < 0) break; // se agotaron todas las combinaciones para este primer caracter
        }
        contadorGlobal.fetch_add(localCount, std::memory_order_relaxed);
    }
}

// ---------- Orquestacion: lanzar hilos, esperar, consolidar resultado ----------

inline ResultadoAtaque fuerzaBrutaParalela(const std::string& hashObjetivo,
                                            const std::string& alfabeto,
                                            int n,
                                            int numHilos) {
    std::atomic<bool> encontrado(false);
    std::atomic<uint64_t> contador(0);
    std::vector<std::string> resultadosPorHilo(numHilos);
    std::vector<std::thread> hilos;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int h = 0; h < numHilos; ++h) {
        hilos.emplace_back(trabajoHilo, std::cref(alfabeto), n, std::cref(hashObjetivo),
                            h, numHilos, std::ref(encontrado), std::ref(contador),
                            std::ref(resultadosPorHilo[h]));
    }
    for (auto& t : hilos) t.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    ResultadoAtaque r;
    r.tiempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    r.candidatosEvaluados = contador.load();
    r.encontrada = encontrado.load();
    if (r.encontrada) {
        for (auto& s : resultadosPorHilo) {
            if (!s.empty()) { r.texto_plano = s; break; }
        }
    }
    return r;
}
