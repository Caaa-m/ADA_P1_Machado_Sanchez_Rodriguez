// Tests del Modulo BT
//
// Verifica:
//  1) El alfabeto tiene exactamente 69 simbolos, incluyendo ñ/Ñ.
//  2) La politica de referencia y la politica del equipo (semilla 4613)
//     se calculan correctamente, incluyendo el ajuste de minLower cuando
//     la suma de minimos excede n.
//  3) La funcion de factibilidad (esFactibleBT) poda correctamente.
//  4) cumplePoliticaBT clasifica correctamente cadenas validas/invalidas
//     construidas a mano.
//  5) La enumeracion SIN poda visita exactamente Sigma_{k=0}^{n} |Sigma|^k
//     nodos (completitud, sin omisiones ni repeticiones) en una instancia
//     pequeña.
//  6) CON poda y SIN poda coinciden en el numero de soluciones validas
//     encontradas (verificacion de correctitud de la poda, Seccion 8.2).
//  7) La version con 1 hilo y con varios hilos dan el mismo resultado
//     (paralelizar no cambia la respuesta).
//
// Compilar: g++ -std=c++17 -O2 -pthread -o test_bt tests/test_bt.cpp
// Ejecutar: ./test_bt   (debe correr desde la raiz del repo, para
//                        encontrar src/BtAlphabet.hpp y src/BtThreads.hpp)

#include "../src/BtAlphabet.hpp"
#include "../src/BtThreads.hpp"

#include <iostream>
#include <set>
#include <string>
#include <vector>

static int g_fallos = 0;

static void check(bool condicion, const std::string& nombre) {
    if (condicion) {
        std::cout << "[OK] " << nombre << "\n";
    } else {
        std::cout << "[FALLO] " << nombre << "\n";
        g_fallos++;
    }
}

// ---------- 1) Alfabeto: 69 simbolos, incluye ñ/Ñ ----------

static void test_alfabeto() {
    check(BT_ALPHABET.size() == 69, "El alfabeto tiene exactamente 69 simbolos");

    bool tieneMinuscula_n = false, tieneMayuscula_N = false;
    for (auto& tok : BT_ALPHABET) {
        if (tok == "\xC3\xB1") tieneMinuscula_n = true;
        if (tok == "\xC3\x91") tieneMayuscula_N = true;
    }
    check(tieneMinuscula_n, "El alfabeto incluye la ñ minuscula");
    check(tieneMayuscula_N, "El alfabeto incluye la Ñ mayuscula");

    // Sin duplicados
    std::set<std::string> unicos(BT_ALPHABET.begin(), BT_ALPHABET.end());
    check(unicos.size() == BT_ALPHABET.size(), "El alfabeto no tiene simbolos repetidos");
}

// ---------- 2) Politicas: referencia y equipo (semilla 4613) ----------

static void test_politicas() {
    PoliticaBT ref = politicaReferencia();
    check(ref.minLower == 2 && ref.minUpper == 1 && ref.minDigit == 1 && ref.minSymbol == 1,
          "Politica de referencia: minLower=2, minUpper=1, minDigit=1, minSymbol=1");

    // Semilla del equipo = 4613
    // minLower = 2 + (4613 % 3) = 2 + 2 = 4
    // minUpper = 1 + (4613 % 2) = 1 + 1 = 2
    // minDigit = 1 + (4613 % 3) = 1 + 2 = 3
    // minSymbol = 1
    // suma = 4+2+3+1 = 10 > 8  ->  minLower se reduce en 2  ->  minLower=2
    check(SEMILLA_EQUIPO == 4613, "La semilla del equipo es 4613");

    PoliticaBT eq = politicaDelEquipo(SEMILLA_EQUIPO);
    check(eq.minLower == 2, "Politica del equipo: minLower ajustado a 2 (era 4, se redujo en 2)");
    check(eq.minUpper == 2, "Politica del equipo: minUpper = 2");
    check(eq.minDigit == 3, "Politica del equipo: minDigit = 3");
    check(eq.minSymbol == 1, "Politica del equipo: minSymbol = 1");
    check(eq.minLower + eq.minUpper + eq.minDigit + eq.minSymbol == 8,
          "Politica del equipo: suma de minimos ajustada = 8 (cabe exacto en n=8)");
}

// ---------- 3) Factibilidad (poda) ----------

static void test_factibilidad() {
    PoliticaBT pol{2, 1, 1, 1, true}; // igual a la de referencia

    // Estado vacio, quedan 6 posiciones: 5 requisitos, 6 restantes -> factible
    EstadoParcialBT vacio;
    check(esFactibleBT(vacio, 6, pol), "Estado vacio con 6 restantes es factible (suma minimos=5)");

    // Estado vacio, solo quedan 4 posiciones: 5 requisitos > 4 restantes -> NO factible
    check(!esFactibleBT(vacio, 4, pol), "Estado vacio con solo 4 restantes NO es factible (suma minimos=5)");

    // Estado que ya cumple todo, con 0 restantes -> factible (nada falta)
    EstadoParcialBT completo;
    completo.nLower = 2; completo.nUpper = 1; completo.nDigit = 1; completo.nSymbol = 1;
    check(esFactibleBT(completo, 0, pol), "Estado que ya cumple todos los minimos es factible con 0 restantes");

    // Politica del equipo (suma=8) aplicada a n=6 (restantes=6 en la raiz) -> NO factible
    PoliticaBT eq = politicaDelEquipo(SEMILLA_EQUIPO);
    check(!esFactibleBT(vacio, 6, eq),
          "Politica del equipo (suma=8) es infactible desde la raiz cuando solo hay 6 posiciones");
}

// ---------- 4) cumplePoliticaBT sobre cadenas construidas a mano ----------

static std::vector<std::string> tokenizar(const std::string& s) {
    // Solo para strings ASCII simples en estos tests (sin ñ), un caracter = un token
    std::vector<std::string> r;
    for (char c : s) r.push_back(std::string(1, c));
    return r;
}

static void test_cumple_politica() {
    PoliticaBT pol{2, 1, 1, 1, true}; // minLower=2, minUpper=1, minDigit=1, minSymbol=1, sin repetidos

    check(cumplePoliticaBT(tokenizar("abaA0!"), pol),
          "\"abaA0!\" cumple la politica de referencia (2 min, 1 May, 1 dig, 1 simbolo)");

    check(!cumplePoliticaBT(tokenizar("aaaA0!"), pol),
          "\"aaaA0!\" NO cumple: tiene \"aa\" repetido consecutivo");

    check(!cumplePoliticaBT(tokenizar("abA001"), pol),
          "\"abA001\" NO cumple: le falta un simbolo (minSymbol=1)");

    check(!cumplePoliticaBT(tokenizar("ab0001"), pol),
          "\"ab0001\" NO cumple: le falta una mayuscula (minUpper=1)");
}

// ---------- 5) Completitud de la enumeracion SIN poda (instancia pequeña) ----------

static void test_completitud_sin_poda() {
    // n=2, sin ninguna restriccion de composicion: debe visitar exactamente
    // Sigma_{k=0}^{2} |Sigma|^k = 1 + 69 + 69^2 = 1 + 69 + 4761 = 4831 nodos
    PoliticaBT sinRestriccion{0, 0, 0, 0, false};
    long long esperado = 1 + 69 + 69 * 69;

    ResultadoBTHilos r = backtrackSinPodaParalela(2, sinRestriccion, /*numHilos=*/1,
                                                   /*ejemplosMax=*/0, /*maxNodes=*/0, /*stopFirst=*/false);
    check((long long)r.nodesVisited == esperado,
          "Enumeracion SIN poda (n=2, sin restriccion): nodesVisited = 1+69+69^2 = " + std::to_string(esperado));
    check((long long)r.leavesChecked == 69 * 69,
          "Enumeracion SIN poda (n=2): hojas evaluadas = 69^2");
    check((long long)r.validFound == 69 * 69,
          "Enumeracion SIN poda (n=2, sin restriccion): todas las hojas son validas");
}

// ---------- 6) CON poda y SIN poda deben coincidir en soluciones (correctitud) ----------

static void test_poda_vs_sin_poda() {
    // Instancia pequeña y tratable: n=3, minLower=1, minUpper=1
    PoliticaBT pol{1, 1, 0, 0, true};

    ResultadoBTHilos conPoda = backtrackConPodaParalela(3, pol, /*numHilos=*/1, 0, 0, false);
    ResultadoBTHilos sinPoda = backtrackSinPodaParalela(3, pol, /*numHilos=*/1, 0, 0, false);

    check(conPoda.validFound == sinPoda.validFound,
          "CON poda y SIN poda encuentran el mismo numero de soluciones (n=3, minLower=1,minUpper=1)");
    check(conPoda.validFound == 180792,
          "El numero de soluciones coincide con el valor ya validado manualmente (180792)");
    check(sinPoda.nodesVisited == 333340,
          "SIN poda visita exactamente Sigma_{k=0}^{3} 69^k = 333340 nodos");
    check(conPoda.nodesVisited < sinPoda.nodesVisited,
          "CON poda visita menos nodos que SIN poda (la poda efectivamente reduce el arbol)");
}

// ---------- 7) Paralelismo no cambia el resultado (1 hilo vs varios hilos) ----------

static void test_paralelismo_consistente() {
    PoliticaBT pol{1, 1, 0, 0, true};

    ResultadoBTHilos unHilo = backtrackConPodaParalela(3, pol, 1, 0, 0, false);
    ResultadoBTHilos variosHilos = backtrackConPodaParalela(3, pol, 4, 0, 0, false);

    check(unHilo.validFound == variosHilos.validFound,
          "1 hilo y 4 hilos encuentran el mismo numero de soluciones (n=3)");
    check(unHilo.nodesVisited == variosHilos.nodesVisited,
          "1 hilo y 4 hilos visitan exactamente el mismo numero de nodos (particion sin solapamiento)");
}

int main() {
    test_alfabeto();
    test_politicas();
    test_factibilidad();
    test_cumple_politica();
    test_completitud_sin_poda();
    test_poda_vs_sin_poda();
    test_paralelismo_consistente();

    std::cout << "\n" << (g_fallos == 0 ? "TODOS LOS TESTS PASARON" :
                          std::to_string(g_fallos) + " TEST(S) FALLARON") << "\n";
    return g_fallos == 0 ? 0 : 1;
}
