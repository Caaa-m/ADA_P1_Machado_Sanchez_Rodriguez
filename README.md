# ADA Práctica 1 — Fuerza Bruta y Backtracking

**Curso:** 0360_2666 — Estructura de Datos y Algoritmos 2
**Integrantes:** Machado Sanchez, Cristobal — Rodriguez Gonzalez, Maria Camila — Sanchez Gomez, Sebastian
**Semilla del equipo:** `4613` (ver Sección "Semilla" más abajo)

## Estructura del proyecto

El repositorio tiene dos módulos independientes que comparten un mismo dominio
(contraseñas sobre un alfabeto): **Módulo FB** (`src/fb.cpp`), que ataca un
hash SHA-256 objetivo por enumeración exhaustiva, y **Módulo BT**
(`src/bt.cpp`), que genera/cuenta contraseñas que cumplen una política de
composición usando backtracking con poda. Un único punto de entrada,
`src/main.cpp`, expone ambos módulos bajo un solo ejecutable (`ada_p1`),
como exige la Sección 11 del enunciado; internamente delega a `fb_main()` y
`bt_main()` según el primer argumento. Cada módulo tiene su propia lógica de
paralelización por hilos (`fb_threads.hpp` / `bt_threads.hpp`) y su propio
archivo de alfabeto/reglas (`fb_alphabets.hpp` / `bt_alphabet.hpp`).
`src/third_party/picosha2.h` es la biblioteca externa (MIT license) usada
para calcular SHA-256, según autoriza la Sección 10 del enunciado. `tests/`
contiene los tests automatizados y los scripts de verificación de
semilla/instancias; `resources/diccionario.txt` es la lista sintética de 500
contraseñas comunes para la comparación de la Sección 8.1; `results/` tiene
los CSV y gráficas (.png) de la experimentación; `report/` tiene el informe
técnico y los documentos de apoyo (pseudocódigo, análisis de complejidad).

```
.
├── src/
│   ├── main.cpp                 CLI unico: despacha a fb_main()/bt_main()
│   ├── fb.cpp                   Modulo FB: ataque diccionario, batch
│   ├── fb_alphabets.hpp         Alfabetos A1 (26) y A2 (36) del Modulo FB
│   ├── fb_threads.hpp           Config. de hilos + fuerza bruta paralela
│   ├── bt.cpp                   Modulo BT: variantes, referencia, custom
│   ├── bt_alphabet.hpp          Alfabeto (69 simbolos, incluye ñ/Ñ) + politicas
│   ├── bt_threads.hpp           Config. de hilos + backtracking paralelo
│   └── third_party/
│       ├── picosha2.h           Biblioteca SHA-256 (MIT, okdshin/PicoSHA2)
│       └── LICENSE_PICOSHA2
├── tests/
│   ├── test_fb.cpp              Tests del Modulo FB
│   ├── test_bt.cpp              Tests del Modulo BT
│   ├── test_herramientas.cpp    Tests de verificar_semilla / generar_instancias_fb
│   ├── verificar_semilla.cpp    Verifica la semilla del equipo (4613)
│   └── generar_instancias_fb.cpp  Genera y verifica las 5 instancias FB
├── resources/
│   └── diccionario.txt          500 contraseñas sinteticas (Seccion 8.1)
├── results/
│   ├── fb_results.csv
│   ├── fb_dict_comparison.csv
│   ├── bt_results.csv
│   ├── bt_poda_comparacion.csv
│   ├── fb_tiempo_vs_instancia.png
│   ├── bt_poda_vs_visitados.png
│   └── bt_con_vs_sin_poda_n3.png
└── report/
    ├── Informe.pdf
    ├── pseudocodigo_fb.md
    ├── pseudocodigo_bt.md
    ├── analisis_complejidad_fb.md
    └── analisis_complejidad_bt.md
```

## Compilación

Un único binario para todo el proyecto (Sección 11 del enunciado; requiere
`-pthread` porque ambos módulos usan hilos):

```bash
g++ -std=c++17 -O2 -pthread -o ada_p1 src/Fb.cpp src/Bt.cpp src/main.cpp
```

Tests:
```bash
g++ -std=c++17 -O2 -pthread -o test_fb tests/test_fb.cpp
g++ -std=c++17 -O2 -pthread -o test_bt tests/test_bt.cpp
g++ -std=c++17 -O2 -o test_herramientas tests/test_herramientas.cpp
```

Utilidades de verificación:
```bash
g++ -std=c++17 -O2 -o verificar_semilla tests/verificar_semilla.cpp
g++ -std=c++17 -O2 -o generar_instancias_fb tests/generar_instancias_fb.cpp
```

## Ejecución — un único comando por módulo

**Módulo FB** (corre la instancia de referencia + las 5 del equipo, exporta CSV):
```bash
./ada_p1 fb batch --out results/fb_results.csv
```

**Módulo BT** (corre las 5 variantes de la Sección 9.2; usa `--stopfirst` para
validar rápido, o `--maxnodes` para instancias grandes que no terminan):
```bash
./ada_p1 bt variante --id i   --maxnodes 50000000
./ada_p1 bt variante --id ii  --maxnodes 50000000
./ada_p1 bt variante --id iii --maxnodes 50000000
./ada_p1 bt variante --id iv  --maxnodes 50000000
./ada_p1 bt variante --id v   --maxnodes 50000000
```

Ver `report/README_seccion_FB.md` y `report/README_seccion_BT.md` para el
detalle completo de todos los modos, flags, y las instancias/hashes exactos
del equipo.

## Semilla

Apellidos de los 3 integrantes (paterno y materno), orden alfabético, sin
espacios ni tildes: `gomezgonzalesmachadorodriguezsanchezsanchez`.
Semilla = suma de códigos ASCII mod 100000 = **4613**.
Verificable con `tests/verificar_semilla.cpp` y, de forma independiente,
con `tests/generar_instancias_fb.cpp` (que además recalcula las 5 contraseñas
objetivo del Módulo FB y sus hashes SHA-256).

## Nota sobre el alfabeto del Módulo BT (69 símbolos)

El enunciado especifica "minúsculas, mayúsculas, dígitos y símbolos
`{!,@,#,$,%}` (69 símbolos en total)". Con el alfabeto inglés (26+26+10+5)
solo se llega a 67; los 69 símbolos exactos se obtienen incluyendo la ñ/Ñ
(27+27+10+5=69), ya que "minúsculas"/"mayúsculas" en español las incluyen.
El código representa el alfabeto como una lista de tokens UTF-8 (no
`std::string` de caracteres sueltos) para manejar correctamente la ñ/Ñ, que
ocupan 2 bytes cada una.

