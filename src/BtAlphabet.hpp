#pragma once
#include <string>
#include <vector>

// Alfabeto base del Modulo BT (Seccion 9.2): minusculas + mayusculas +
// digitos + simbolos {!,@,#,$,%}.
// "Minusculas" y "mayusculas" en español incluyen la ñ/Ñ:
//   27 (a..z + ñ) + 27 (A..Z + Ñ) + 10 (digitos) + 5 (simbolos) = 69 simbolos
//
// Como la ñ/Ñ ocupan 2 bytes en UTF-8, el alfabeto se representa como una
// lista de "tokens" (cada uno un simbolo completo, de 1 o 2 bytes), en vez
// de un std::string de caracteres sueltos.

inline std::vector<std::string> construirAlfabetoBT() {
    std::vector<std::string> simbolos;
    for (char c = 'a'; c <= 'z'; ++c) simbolos.push_back(std::string(1, c));
    simbolos.push_back("\xC3\xB1"); // ñ (UTF-8)
    for (char c = 'A'; c <= 'Z'; ++c) simbolos.push_back(std::string(1, c));
    simbolos.push_back("\xC3\x91"); // Ñ (UTF-8)
    for (char c = '0'; c <= '9'; ++c) simbolos.push_back(std::string(1, c));
    for (char c : std::string("!@#$%")) simbolos.push_back(std::string(1, c));
    return simbolos;
}

inline const std::vector<std::string> BT_ALPHABET = construirAlfabetoBT();
// BT_ALPHABET.size() == 69

// Clase de simbolo, usada para verificar la politica sobre un prefijo parcial
enum class ClaseChar { MINUSCULA, MAYUSCULA, DIGITO, SIMBOLO };

inline ClaseChar claseDe(const std::string& tok) {
    if (tok == "\xC3\xB1") return ClaseChar::MINUSCULA;       // ñ
    if (tok == "\xC3\x91") return ClaseChar::MAYUSCULA;       // Ñ
    char c = tok[0];
    if (c >= 'a' && c <= 'z') return ClaseChar::MINUSCULA;
    if (c >= 'A' && c <= 'Z') return ClaseChar::MAYUSCULA;
    if (c >= '0' && c <= '9') return ClaseChar::DIGITO;
    return ClaseChar::SIMBOLO; // !,@,#,$,%
}

// Longitud fija de la politica (Seccion 9.2)
inline const int BT_N = 8;

// Parametros de la politica del equipo, derivados de la semilla (Seccion 9.2):
//   minLower  = 2 + (semilla mod 3)
//   minUpper  = 1 + (semilla mod 2)
//   minDigit  = 1 + (semilla mod 3)
//   minSymbol = 1
//   prohibicion de dos caracteres identicos consecutivos (aplica siempre)
// Si minLower+minUpper+minDigit+minSymbol > n=8, se reduce minLower en el
// exceso necesario (y se reporta en el informe).
struct PoliticaBT {
    int minLower;
    int minUpper;
    int minDigit;
    int minSymbol;
    bool sinRepetidosConsecutivos;
};

inline PoliticaBT politicaDelEquipo(long long semilla) {
    int minLower  = 2 + (int)(semilla % 3);
    int minUpper  = 1 + (int)(semilla % 2);
    int minDigit  = 1 + (int)(semilla % 3);
    int minSymbol = 1;

    int suma = minLower + minUpper + minDigit + minSymbol;
    if (suma > BT_N) {
        int exceso = suma - BT_N;
        minLower -= exceso; // se documenta en el informe (Seccion 9.2)
    }
    return PoliticaBT{minLower, minUpper, minDigit, minSymbol, true};
}

// Politica de referencia comun a todo el curso (Seccion 9.2), para validar
// la implementacion antes de correr las instancias propias:
//   minLower=2, minUpper=1, minDigit=1, minSymbol=1, sin repetidos, n=6
inline PoliticaBT politicaReferencia() {
    return PoliticaBT{2, 1, 1, 1, true};
}

// Politica relajada (variante iv de la Seccion 9.2): solo minLower=1,
// sin las demas restricciones de composicion.
inline PoliticaBT politicaRelajada() {
    return PoliticaBT{1, 0, 0, 0, true};
}

// Politica vacia (variante v, "poda nula"): ninguna restriccion de
// composicion. Equivalente a enumeracion exhaustiva.
inline PoliticaBT politicaVacia() {
    return PoliticaBT{0, 0, 0, 0, false};
}

// Semilla del equipo (Seccion 9.1 / 9.2): todos los apellidos (paterno y
// materno) de los 3 integrantes, orden alfabetico.
// apellidos = "gomezgonzalesmachadorodriguezsanchezsanchez" -> semilla = 4613
inline const long long SEMILLA_EQUIPO = 4613;
