#ifndef SEMILLA_HPP
#define SEMILLA_HPP

#include <string>

// Calcula la semilla a partir de los apellidos ya ordenados alfabeticamente,
// concatenados, sin espacios ni tildes, en minuscula (Seccion 9.1)
inline long long calcularSemilla(const std::string& apellidosConcatenados) {
    long long suma = 0;
    for (unsigned char c : apellidosConcatenados) suma += (int)c;
    return suma % 100000;
}

#endif