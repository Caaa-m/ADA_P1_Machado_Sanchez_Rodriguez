#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

// Incluye la biblioteca PicoSHA2 (coloca la ruta correcta)
#include "../src/third_party/picosha2.h"

using namespace std;

int main() {
    // Semilla del equipo (calculada según apellidos)
    const unsigned long long SEMILLA = 4613;

    // Alfabetos definidos en la práctica
    const string ALFABETO1 = "abcdefghijklmnopqrstuvwxyz";                 // 26 símbolos
    const string ALFABETO2 = "abcdefghijklmnopqrstuvwxyz0123456789";      // 36 símbolos

    // Longitudes y tipo de alfabeto para cada una de las 5 instancias
    // (orden exacto según enunciado: A1, A2, A1, A2, A1)
    const vector<int> LONGITUDES = {4, 4, 5, 5, 6};
    const vector<string> TIPOS = {"A1", "A2", "A1", "A2", "A1"};

    // Parámetros del LCG (lineal congruential generator)
    const unsigned long long a = 1103515245ULL;
    const unsigned long long c = 12345ULL;
    const unsigned long long m = 2147483648ULL;  // 2^31

    unsigned long long x = SEMILLA;

    cout << "=== Generación de instancias del Módulo FB ===\n";
    cout << "Semilla: " << SEMILLA << "\n\n";

    cout << "| Instancia | Alfabeto | n | Contraseña | Hash SHA-256 |\n";
    cout << "|-----------|----------|---|------------|------------------------------------------|\n";

    for (int i = 0; i < 5; ++i) {
        // Seleccionar alfabeto según el tipo
        const string& alfabeto = (TIPOS[i] == "A1") ? ALFABETO1 : ALFABETO2;
        int n = LONGITUDES[i];

        string password;
        password.reserve(n);

        // Generar los n caracteres consumiendo el LCG secuencialmente
        for (int j = 0; j < n; ++j) {
            x = (a * x + c) % m;
            int idx = x % alfabeto.size();
            password.push_back(alfabeto[idx]);
        }

        // Calcular el hash SHA-256 en hexadecimal
        string hash_hex = picosha2::hash256_hex_string(password);

        // Imprimir fila de la tabla
        cout << "| equipo_" << (i+1) << " | " << TIPOS[i] << " | " << n
             << " | " << password << " | " << hash_hex << " |\n";
    }

    cout << "\nNota: Verifica que estos hashes coincidan con los publicados en el informe.\n";
    cout << "Si algún hash difiere, revisa la semilla o el orden de los apellidos.\n";

    return 0;
}
