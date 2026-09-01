#include <iostream>
#include <string>

using namespace std;

int main() {
    string apellidos = "gomezgonzalesmachadorodriguezsanchezsanchez";

    long long suma_ascii = 0;

    for (char caracter : apellidos) {
        suma_ascii += static_cast<unsigned char>(caracter);

    }

    int semilla = suma_ascii % 100000;

    cout << "Apellidos: " << apellidos << endl;
    cout << "Suma ASCII: " << suma_ascii << endl;
    cout << "Semilla: " << semilla << endl;

    return 0;

}
