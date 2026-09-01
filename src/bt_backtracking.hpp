#ifndef BT_BACKTRACKING_HPP
#define BT_BACKTRACKING_HPP

#include <string>
#include <vector>
#include "bt_policy.hpp"

struct ResultadoBT {
    long long nodosGenerados = 0;
    long long nodosVisitados = 0;
    long long soluciones = 0;
    std::vector<std::string> ejemplos;
    double tiempoMs = 0.0;
};

// Backtracking CON poda
ResultadoBT backtrackingConPoda(const std::string& alfabeto, const Politica& pol,
                                 bool guardarSoluciones, int maxEjemplos = 20);

// Enumeracion exhaustiva SIN poda (genera todo y filtra al final) - para la comparacion 8.2
ResultadoBT enumeracionSinPoda(const std::string& alfabeto, const Politica& pol,
                                bool guardarSoluciones, int maxEjemplos = 20);

#endif