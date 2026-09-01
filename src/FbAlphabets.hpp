#pragma once
#include <string>

//Alfabeto A1
inline const std::string A1 = "abcdefghijklmnopqrstuvwxyz";

//Alfabeto A2
inline const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";

// Reglas del espacio de búsqueda (Sección 9.1 del enunciado):
// Longitudes permitidas por alfabeto
inline const int A1_MIN_N = 3;
inline const int A1_MAX_N = 6;   //{3,4,5,6} para A1
 
inline const int A2_MIN_N = 3;
inline const int A2_MAX_N = 5;   //{3,4,5} para A2
 
// Verifica si es válida en A1
inline bool esLongitudValidaA1(int n) {
    return n >= A1_MIN_N && n <= A1_MAX_N;
}
 
// Verifica si es válida en A2
inline bool esLongitudValidaA2(int n) {
    return n >= A2_MIN_N && n <= A2_MAX_N;
}
 