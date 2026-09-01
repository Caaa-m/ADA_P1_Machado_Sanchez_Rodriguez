# ADA_P1_Machado_Sanchez_Rodriguez

Práctica 1 — Fuerza Bruta y Backtracking (Análisis de Algoritmos)

## Integrantes

| Integrante | Nombre completo | Apellidos usados para la semilla |
|---|---|---|
| 1 | Cristobal Machado Sanchez | Machado, Sanchez |
| 2 | Sebastian Sanchez Gomez | Sanchez, Gomez |
| 3 | Maria Camila Rodriguez Gonzalez | Rodriguez, Gonzalez |

**Semilla del equipo:** `4613`

Calculada concatenando (en orden alfabético) los dos apellidos de cada integrante,
sumando el valor ASCII de cada carácter y aplicando módulo 100000. El procedimiento
completo y reproducible está en [`tests/verificar_semilla.cpp`](tests/verificar_semilla.cpp).

## Organización del proyecto

Este repositorio contiene la implementación de dos enfoques algorítmicos —
Fuerza Bruta (FB) y Backtracking (BT) — para el problema de recuperación de
contraseñas descrito en el enunciado de la Práctica 1. El código fuente de
ambos módulos vive en `src/`, las utilidades de verificación y los casos de
prueba en `tests/`, los datos de entrada (diccionario e instancias generadas)
en `resources/`, las salidas de los experimentos en `results/`, y el informe
técnico en `report/`. `main.cpp` actúa como punto de entrada único (CLI) que
invoca cada módulo por separado.

## Estructura del repositorio

```
.
├── README.md
├── src/
│   ├── main.cpp                  # CLI (en construcción)
│   ├── fb_*.cpp / fb_*.hpp       # Módulo de Fuerza Bruta (pendiente)
│   ├── bt_*.cpp / bt_*.hpp       # Módulo de Backtracking (pendiente)
│   └── third_party/
│       ├── picosha2.h            # Librería de hashing SHA-256 (terceros)
│       └── LICENSE_PICOSHA2      # Licencia de picosha2 (MIT, okdshin 2017)
├── tests/
│   ├── verificar_semilla.cpp     # Verifica el cálculo de la semilla del equipo
│   └── generar_instancias_fb.cpp # Genera las 5 instancias FB + hash SHA-256
├── resources/
│   └── diccionario.txt           # 500 candidatos sintéticos (Módulo FB, Sección 8.1)
├── results/                      # Tablas de tiempos (.csv), salidas (.txt), gráficas (.png)
└── report/
    └── Informe.pdf               # Informe técnico (Sección 12)
```

## Compilación

Requiere `g++` con soporte para C++17.

```bash
g++ -std=c++17 -O2 -o ada_p1 src/main.cpp src/*.cpp
```

> **Nota:** este comando quedará funcional una vez estén integrados los
> módulos `fb_*.cpp` y `bt_*.cpp`. Mientras tanto, las utilidades de
> `tests/` se compilan de forma independiente:
> ```bash
> g++ -std=c++17 -O2 tests/verificar_semilla.cpp -o verificar_semilla
> g++ -std=c++17 -O2 tests/generar_instancias_fb.cpp -o generar_instancias_fb
> ```

## Ejecución y reproducción de experimentos

```bash
./ada_p1 --modulo fb --instancia 1
./ada_p1 --modulo bt --instancia 1
```

> **TODO:** documentar aquí los argumentos definitivos de `main.cpp` una vez
> se implemente el CLI y se acuerden los nombres de las banderas con el
> resto del equipo.

Para verificar la semilla del equipo:
```bash
./verificar_semilla
```

Para regenerar las 5 instancias privadas del Módulo FB (con sus hashes SHA-256):
```bash
./generar_instancias_fb
```

## Estado actual del proyecto

- [x] Estructura de carpetas
- [x] Verificación de semilla
- [x] Generación de instancias FB + hashing SHA-256
- [ ] Implementación Módulo FB
- [ ] Implementación Módulo BT
- [ ] CLI (`main.cpp`)
- [ ] Pruebas unitarias automatizadas
- [ ] Informe técnico (LaTeX)
