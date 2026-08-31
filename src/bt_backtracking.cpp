#include "bt_backtracking.hpp"
#include <chrono>
#include <algorithm>

// Revisa si un prefijo parcial todavia puede llegar a cumplir la politica
// con los caracteres que faltan. Esta funcion es la que hace posible la poda.
static bool esFactible(const Politica& pol, int nLower, int nUpper, int nDigit, int nSymbol, int restantes) {
    int faltanLower = std::max(0, pol.minLower - nLower);
    int faltanUpper = std::max(0, pol.minUpper - nUpper);
    int faltanDigit = std::max(0, pol.minDigit - nDigit);
    int faltanSymbol = std::max(0, pol.minSymbol - nSymbol);
    int faltanTotal = faltanLower + faltanUpper + faltanDigit + faltanSymbol;
    return faltanTotal <= restantes;
}

static void backtrackRec(std::string& prefijo, const std::string& alfabeto, const Politica& pol,
                          int nLower, int nUpper, int nDigit, int nSymbol,
                          bool guardarSoluciones, int maxEjemplos, ResultadoBT& res) {
    res.nodosVisitados++;
    int k = (int)prefijo.size();

    if (k == pol.n) {
        res.soluciones++;
        if (guardarSoluciones && (int)res.ejemplos.size() < maxEjemplos)
            res.ejemplos.push_back(prefijo);
        return;
    }

    for (char c : alfabeto) {
        // restriccion local: no dos caracteres iguales seguidos
        if (pol.prohibirRepetidosConsecutivos && !prefijo.empty() && prefijo.back() == c)
            continue;

        int t = tipoCaracter(c);
        int nl = nLower + (t == 0);
        int nu = nUpper + (t == 1);
        int nd = nDigit + (t == 2);
        int ns = nSymbol + (t == 3);
        int restantes = pol.n - (k + 1);

        // aca esta la poda: si ya no alcanza para cumplir minimos, no seguimos
        if (!esFactible(pol, nl, nu, nd, ns, restantes))
            continue;

        prefijo.push_back(c);
        backtrackRec(prefijo, alfabeto, pol, nl, nu, nd, ns, guardarSoluciones, maxEjemplos, res);
        prefijo.pop_back();
    }
}

ResultadoBT backtrackingConPoda(const std::string& alfabeto, const Politica& pol,
                                 bool guardarSoluciones, int maxEjemplos) {
    ResultadoBT res;
    std::string prefijo;
    prefijo.reserve(pol.n);

    auto t0 = std::chrono::high_resolution_clock::now();
    backtrackRec(prefijo, alfabeto, pol, 0, 0, 0, 0, guardarSoluciones, maxEjemplos, res);
    auto t1 = std::chrono::high_resolution_clock::now();

    res.tiempoMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    res.nodosGenerados = res.nodosVisitados; // aca no aplica, se compara contra la version sin poda
    return res;
}

static bool cumplePolitica(const std::string& s, const Politica& pol) {
    int nl = 0, nu = 0, nd = 0, ns = 0;
    for (size_t i = 0; i < s.size(); i++) {
        if (pol.prohibirRepetidosConsecutivos && i > 0 && s[i] == s[i - 1]) return false;
        int t = tipoCaracter(s[i]);
        if (t == 0) nl++; else if (t == 1) nu++; else if (t == 2) nd++; else ns++;
    }
    return nl >= pol.minLower && nu >= pol.minUpper && nd >= pol.minDigit && ns >= pol.minSymbol;
}

static void enumRec(std::string& prefijo, const std::string& alfabeto, const Politica& pol,
                     bool guardarSoluciones, int maxEjemplos, ResultadoBT& res) {
    res.nodosVisitados++; // aca no hay poda, se generan todos los nodos

    if ((int)prefijo.size() == pol.n) {
        if (cumplePolitica(prefijo, pol)) {
            res.soluciones++;
            if (guardarSoluciones && (int)res.ejemplos.size() < maxEjemplos)
                res.ejemplos.push_back(prefijo);
        }
        return;
    }

    for (char c : alfabeto) {
        prefijo.push_back(c);
        enumRec(prefijo, alfabeto, pol, guardarSoluciones, maxEjemplos, res);
        prefijo.pop_back();
    }
}

ResultadoBT enumeracionSinPoda(const std::string& alfabeto, const Politica& pol,
                                bool guardarSoluciones, int maxEjemplos) {
    ResultadoBT res;
    std::string prefijo;
    prefijo.reserve(pol.n);

    auto t0 = std::chrono::high_resolution_clock::now();
    enumRec(prefijo, alfabeto, pol, guardarSoluciones, maxEjemplos, res);
    auto t1 = std::chrono::high_resolution_clock::now();

    res.tiempoMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    res.nodosGenerados = res.nodosVisitados;
    return res;
}