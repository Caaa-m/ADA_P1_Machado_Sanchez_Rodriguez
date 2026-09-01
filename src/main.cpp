// CLI unico del proyecto (Seccion 11 del enunciado).
// No implementa logica de FB ni BT: solo recibe el primer argumento
// (fb o bt) y delega el resto de los argumentos tal cual al modulo
// correspondiente, cuyos main() fueron renombrados a fb_main/bt_main
// para poder compilar los tres archivos juntos en un solo binario.
//
// Compilar: g++ -std=c++17 -O2 -pthread -o ada_p1 src/main.cpp src/*.cpp
//
// Uso:
//   ./ada_p1 fb bruteforce --hash <hex> --alphabet a1|a2 --n <len> [--threads N]
//   ./ada_p1 fb dictionary --hash <hex> --dict <ruta>
//   ./ada_p1 fb batch --out <ruta_csv>
//
//   ./ada_p1 bt referencia [--examples K] [--sinpoda] [--maxnodes N] [--stopfirst] [--threads N]
//   ./ada_p1 bt variante --id i|ii|iii|iv|v [...]
//   ./ada_p1 bt custom --minLower L --minUpper U --minDigit D --minSymbol S --n N [...]

#include <iostream>
#include <string>

int fb_main(int argc, char** argv);
int bt_main(int argc, char** argv);

static void imprimirUso(const char* nombrePrograma) {
    std::cerr << "Uso: " << nombrePrograma << " <modulo> [argumentos...]\n\n"
              << "Modulos disponibles:\n"
              << "  fb   Modulo de Fuerza Bruta (ver src/fb.cpp para su uso detallado)\n"
              << "  bt   Modulo de Backtracking (ver src/bt.cpp para su uso detallado)\n\n"
              << "Ejemplos:\n"
              << "  " << nombrePrograma << " fb batch --out results/fb_results.csv\n"
              << "  " << nombrePrograma << " bt referencia --stopfirst --examples 5\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        imprimirUso(argv[0]);
        return 1;
    }

    std::string modulo = argv[1];

    // Se recorta argv para que el modulo reciba los argumentos como si
    // hubiera sido invocado directamente: argv[1] ("fb" o "bt") pasa a
    // ser su propio argv[0], y el resto de argumentos se conserva igual.
    int argcModulo = argc - 1;
    char** argvModulo = argv + 1;

    if (modulo == "fb") {
        return fb_main(argcModulo, argvModulo);
    } else if (modulo == "bt") {
        return bt_main(argcModulo, argvModulo);
    } else if (modulo == "--help" || modulo == "-h") {
        imprimirUso(argv[0]);
        return 0;
    } else {
        std::cerr << "Modulo desconocido: " << modulo << "\n\n";
        imprimirUso(argv[0]);
        return 1;
    }
}
