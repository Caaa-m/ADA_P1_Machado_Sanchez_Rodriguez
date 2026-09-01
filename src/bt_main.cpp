#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "bt_policy.hpp"
#include "bt_backtracking.hpp"
#include "semilla.hpp"

std::string construirAlfabetoBT() {
    std::string a;
    for (char c = 'a'; c <= 'z'; c++) a += c;
    for (char c = 'A'; c <= 'Z'; c++) a += c;
    for (char c = '0'; c <= '9'; c++) a += c;
    a += "!@#$%";
    return a;
}

double tamanoEspacio(int tamAlfabeto, int n) {
    double t = 1.0;
    for (int i = 0; i < n; i++) t *= tamAlfabeto;
    return t;
}

int main() {
    std::string alfabeto = construirAlfabetoBT();

    // TODO: cambiar por los apellidos reales del equipo, ordenados
    // alfabeticamente, sin espacios ni tildes, en minuscula
    std::string apellidos = "perezgomez";
    long long semilla = calcularSemilla(apellidos);
    std::cout << "Semilla del equipo: " << semilla << "\n\n";

    int minLower = 2 + (semilla % 3);
    int minUpper = 1 + (semilla % 2);
    int minDigit = 1 + (semilla % 3);
    int minSymbol = 1;

    if (minLower + minUpper + minDigit + minSymbol > 8) {
        int exceso = (minLower + minUpper + minDigit + minSymbol) - 8;
        minLower -= exceso;
        std::cout << "Se ajusto minLower en " << exceso << " para que la politica sea satisfacible\n\n";
    }

    Politica politicaEquipo{8, minLower, minUpper, minDigit, minSymbol, true};
    std::cout << "Politica: minLower=" << minLower << " minUpper=" << minUpper
              << " minDigit=" << minDigit << " minSymbol=" << minSymbol << "\n\n";

    Politica referencia{6, 2, 1, 1, 1, true};

    std::vector<std::pair<std::string, Politica>> variantes;
    variantes.push_back({"Referencia (n=6)", referencia});
    variantes.push_back({"(i) Politica completa, n=8", politicaEquipo});

    Politica v2 = politicaEquipo; v2.n = 6;
    variantes.push_back({"(ii) Politica completa, n=6", v2});

    Politica v3 = politicaEquipo; v3.n = 10;
    variantes.push_back({"(iii) Politica completa, n=10", v3});

    variantes.push_back({"(iv) Politica relajada, n=8", Politica{8, 1, 0, 0, 0, false}});
    variantes.push_back({"(v) Sin restricciones (poda nula), n=6", Politica{6, 0, 0, 0, 0, false}});

    std::ofstream csv("results/bt_resultados.csv");
    csv << "instancia,n,tiempo_con_poda_ms,nodos_visitados_con_poda,soluciones,"
        << "tiempo_sin_poda_ms,nodos_sin_poda,reduccion_pct\n";

    const double LIMITE_TRATABLE = 5e7; // si el espacio es mas grande que esto, no corremos la version sin poda

    for (auto& [nombre, pol] : variantes) {
        std::cout << "=== " << nombre << " ===\n";

        ResultadoBT conPoda = backtrackingConPoda(alfabeto, pol, true, 5);
        std::cout << "  Con poda -> nodos: " << conPoda.nodosVisitados
                  << " | soluciones: " << conPoda.soluciones
                  << " | tiempo: " << conPoda.tiempoMs << " ms\n";
        if (!conPoda.ejemplos.empty()) {
            std::cout << "  Ejemplos: ";
            for (auto& e : conPoda.ejemplos) std::cout << e << " ";
            std::cout << "\n";
        }

        double espacio = tamanoEspacio((int)alfabeto.size(), pol.n);
        double reduccionPct = -1, tiempoSinPoda = -1;
        long long nodosSinPoda = -1;

        if (espacio <= LIMITE_TRATABLE) {
            ResultadoBT sinPoda = enumeracionSinPoda(alfabeto, pol, false, 0);
            nodosSinPoda = sinPoda.nodosVisitados;
            tiempoSinPoda = sinPoda.tiempoMs;
            reduccionPct = 100.0 * (1.0 - (double)conPoda.nodosVisitados / (double)sinPoda.nodosVisitados);
            std::cout << "  Sin poda -> nodos: " << sinPoda.nodosVisitados
                      << " | soluciones: " << sinPoda.soluciones
                      << " | tiempo: " << sinPoda.tiempoMs << " ms\n";
            std::cout << "  Reduccion: " << reduccionPct << "%\n";
            if (sinPoda.soluciones != conPoda.soluciones)
                std::cout << "  *** las soluciones NO coinciden, revisar poda ***\n";
        } else {
            std::cout << "  Sin poda -> espacio de " << espacio << " no es tratable, se omite\n";
        }

        csv << nombre << "," << pol.n << "," << conPoda.tiempoMs << "," << conPoda.nodosVisitados
            << "," << conPoda.soluciones << "," << tiempoSinPoda << "," << nodosSinPoda << "," << reduccionPct << "\n";
        std::cout << "\n";
    }

    csv.close();
    std::cout << "Resultados guardados en results/bt_resultados.csv\n";
    return 0;
}