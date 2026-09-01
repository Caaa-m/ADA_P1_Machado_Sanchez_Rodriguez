#ifndef BT_POLICY_HPP
#define BT_POLICY_HPP

// Parametros de la politica de contraseñas del Modulo BT (Seccion 9.2)
struct Politica {
    int n;          // longitud fija de la contraseña
    int minLower;
    int minUpper;
    int minDigit;
    int minSymbol;
    bool prohibirRepetidosConsecutivos;
};

// Clasifica un caracter: 0=minuscula, 1=mayuscula, 2=digito, 3=simbolo
inline int tipoCaracter(char c) {
    if (c >= 'a' && c <= 'z') return 0;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c >= '0' && c <= '9') return 2;
    return 3;
}

#endif