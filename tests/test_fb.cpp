// Tests del Modulo FB
// 1) Verifica que el odometro genera exactamente |Sigma|^n candidatos
//    (sin repetir ni omitir) cuando no hay hash objetivo que lo corte antes.
// 2) Verifica la instancia de referencia comun (abc12, A2, n=5).
//
// Compilar: g++ -std=c++17 -O2 -pthread -o test_fb tests/test_fb.cpp
// Ejecutar: ./test_fb   (debe correr desde la raiz del repo, para
//                        encontrar src/FbAlphabets.hpp y src/third_party/)

#include "../src/FbAlphabets.hpp"
#include "../src/third_party/picosha2.h"

#include <atomic>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

static std::string sha256Hex(const std::string& s) {
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(s.begin(), s.end(), hash.begin(), hash.end());
    return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

// Odometro secuencial simple (un solo hilo) usado solo para testear
// que la enumeracion es correcta: sin repetidos, sin omisiones.
static long long contarYVerificarUnicidad(const std::string& alfabeto, int n) {
    int base = (int)alfabeto.size();
    std::vector<int> idx(n, 0);
    std::set<std::string> vistos;
    long long total = 0;

    while (true) {
        std::string candidato(n, ' ');
        for (int i = 0; i < n; ++i) candidato[i] = alfabeto[idx[i]];

        // set::insert().second es false si ya existia -> hay repeticion
        if (!vistos.insert(candidato).second) {
            std::cerr << "FALLO: candidato repetido -> " << candidato << "\n";
            return -1;
        }
        total++;

        int pos = n - 1;
        while (pos >= 0) {
            idx[pos]++;
            if (idx[pos] < base) break;
            idx[pos] = 0;
            pos--;
        }
        if (pos < 0) break;
    }
    return total;
}

int main() {
    int fallos = 0;

    // --- Test 1: enumeracion completa sin repetidos ni omisiones ---
    // Usamos un espacio pequeño (A1, n=2) para que sea rapido: 26^2 = 676
    {
        long long esperado = 26LL * 26LL;
        long long obtenido = contarYVerificarUnicidad(A1, 2);
        if (obtenido == esperado) {
            std::cout << "[OK] Enumeracion A1 n=2: " << obtenido
                      << " candidatos (esperado " << esperado << ")\n";
        } else {
            std::cout << "[FALLO] Enumeracion A1 n=2: obtenido=" << obtenido
                      << " esperado=" << esperado << "\n";
            fallos++;
        }
    }
    {
        long long esperado = 36LL * 36LL * 36LL;
        long long obtenido = contarYVerificarUnicidad(A2, 3);
        if (obtenido == esperado) {
            std::cout << "[OK] Enumeracion A2 n=3: " << obtenido
                      << " candidatos (esperado " << esperado << ")\n";
        } else {
            std::cout << "[FALLO] Enumeracion A2 n=3: obtenido=" << obtenido
                      << " esperado=" << esperado << "\n";
            fallos++;
        }
    }

    // --- Test 2: instancia de referencia comun (abc12, A2, n=5) ---
    {
        std::string objetivo = "abc12";
        std::string hashEsperado =
            "8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773";
        std::string hashCalculado = sha256Hex(objetivo);
        if (hashCalculado == hashEsperado) {
            std::cout << "[OK] SHA-256(\"abc12\") coincide con el hash de referencia\n";
        } else {
            std::cout << "[FALLO] SHA-256(\"abc12\")=" << hashCalculado
                      << " esperado=" << hashEsperado << "\n";
            fallos++;
        }
    }

    // --- Test 3: verificacion de longitudes validas por alfabeto ---
    {
        bool ok = esLongitudValidaA1(6) && !esLongitudValidaA1(7) &&
                  esLongitudValidaA2(5) && !esLongitudValidaA2(6);
        if (ok) {
            std::cout << "[OK] Rangos de longitud validos por alfabeto\n";
        } else {
            std::cout << "[FALLO] Rangos de longitud validos por alfabeto\n";
            fallos++;
        }
    }

    std::cout << "\n" << (fallos == 0 ? "TODOS LOS TESTS PASARON" :
                           std::to_string(fallos) + " TEST(S) FALLARON") << "\n";
    return fallos == 0 ? 0 : 1;
}
