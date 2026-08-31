#include <iostream>
#include <cassert>
#include "../src/bt_policy.hpp"
#include "../src/bt_backtracking.hpp"

int main() {
    std::string alfabetoChico = "ab12"; // 4 simbolos
    Politica pol{4, 1, 0, 1, 0, true};  // n=4, min 1 minuscula, min 1 digito, sin repetidos seguidos

    ResultadoBT conPoda = backtrackingConPoda(alfabetoChico, pol, true, 100);
    ResultadoBT sinPoda = enumeracionSinPoda(alfabetoChico, pol, true, 100);

    std::cout << "Con poda: " << conPoda.soluciones << " soluciones, " << conPoda.nodosVisitados << " nodos\n";
    std::cout << "Sin poda: " << sinPoda.soluciones << " soluciones, " << sinPoda.nodosVisitados << " nodos\n";

    assert(conPoda.soluciones == sinPoda.soluciones);
    assert(conPoda.nodosVisitados <= sinPoda.nodosVisitados);

    std::cout << "\nOK, las soluciones coinciden y la poda visito menos nodos.\n";
    return 0;
}