// Pruebas para verificar_semilla.cpp y generar_instancias_fb.cpp.
// Como esos dos ya tienen su propio main(), aca replico la misma logica
// y comparo contra los valores que ya verificamos a mano.

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

// misma cuenta de verificar_semilla.cpp
long long sumaAscii(const std::string& apellidos) {
    long long total = 0;
    for (unsigned char c : apellidos) total += (int)c;
    return total % 100000;
}

// mismo LCG de generar_instancias_fb.cpp
std::vector<std::string> generarInstancias(long long semillaEq) {
    const std::string A1 = "abcdefghijklmnopqrstuvwxyz";
    const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";
    const std::vector<int> largos = {4, 4, 5, 5, 6};
    const std::vector<std::string> tipos = {"A1", "A2", "A1", "A2", "A1"};

    unsigned long long x = semillaEq;
    const unsigned long long MOD = 1ULL << 31;
    const unsigned long long A = 1103515245ULL;
    const unsigned long long C = 12345ULL;

    std::vector<std::string> resultado;
    for (int idx = 0; idx < 5; idx++) {
        int n = largos[idx];
        const std::string& alf = (tipos[idx] == "A1") ? A1 : A2;
        std::string pass;
        for (int i = 0; i < n; i++) {
            x = (A * x + C) % MOD;
            pass += alf[x % alf.size()];
        }
        resultado.push_back(pass);
    }
    return resultado;
}

int main() {
    // la semilla del equipo tiene que dar 4613, siempre
    long long semillaEq = sumaAscii("gomezgonzalesmachadorodriguezsanchezsanchez");
    assert(semillaEq == 4613);
    std::cout << "[OK] semilla del equipo da 4613\n";

    assert(sumaAscii("") == 0);
    std::cout << "[OK] cadena vacia da 0\n";

    // este test existe por el bug del espacio que tuvimos antes
    assert(sumaAscii("gomezgonzalesmachadorodriguezsanchezsanchez ") != 4613);
    std::cout << "[OK] un espacio de mas SI arruina la semilla (por eso el test)\n";

    std::vector<std::string> instancias = generarInstancias(4613);

    assert(instancias.size() == 5);
    std::cout << "[OK] salen las 5 instancias\n";

    std::vector<int> largosEsperados = {4, 4, 5, 5, 6};
    for (int i = 0; i < 5; i++) assert((int)instancias[i].size() == largosEsperados[i]);
    std::cout << "[OK] los largos son 4,4,5,5,6 como toca\n";

    std::vector<std::string> esperadas = {"orah", "c1un", "ahcds", "p6ncd", "qnqlyf"};
    for (int i = 0; i < 5; i++) assert(instancias[i] == esperadas[i]);
    std::cout << "[OK] las contrasenas coinciden con las que ya sacamos a mano\n";

    // correrlo dos veces con la misma semilla tiene que dar exactamente lo mismo
    assert(instancias == generarInstancias(4613));
    std::cout << "[OK] es determinista, no cambia entre corridas\n";

    // y una semilla distinta obvio que tiene que dar otra cosa
    assert(instancias != generarInstancias(4614));
    std::cout << "[OK] con otra semilla cambia todo\n";

    std::cout << "\nTODO OK\n";
    return 0;
}
