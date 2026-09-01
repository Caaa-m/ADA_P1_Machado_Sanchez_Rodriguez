## Módulo BT — Backtracking (generación de contraseñas por política, con hilos)

### Nota sobre archivos previos

Existía una implementación alterna del Módulo BT hecha por un compañero de
equipo (`bt_policy.hpp`, `bt_backtracking.hpp/cpp`, `bt_main.cpp`,
`semilla.hpp`), pero tenía dos problemas: (1) usaba un alfabeto de 67
símbolos sin la ñ/Ñ en vez de los 69 exigidos, y (2) tenía la semilla sin
actualizar (`"perezgomez"`, con un `TODO` pendiente nunca resuelto). Se
descartaron esos archivos y se consolidó todo en `BtAlphabet.hpp` +
`BtThreads.hpp` + `Bt.cpp`, que sí implementan el alfabeto correcto y la
semilla real del equipo (4613). `semilla.hpp` (con `calcularSemilla()`) se
conserva porque es correcto y reutilizable.

### Compilación

```bash
g++ -std=c++17 -O2 -pthread -o bt src/Bt.cpp
```

**Nota:** ahora requiere `-pthread` (antes no, porque la versión secuencial no usaba hilos).

### Paralelización

Igual que en el Módulo FB: se particiona el árbol por el **primer símbolo**
(la raíz). Cada hilo recorre los índices del alfabeto tales que
`índice % totalHilos == idHilo`, y bajo esa rama corre la recursión completa
(con o sin poda) de forma normal. La configuración de hilos vive en
`src/BtThreads.hpp`, separada de `Bt.cpp` (mismo patrón que
`FbThreads.hpp`/`Fb.cpp`).

Por defecto detecta automáticamente los hilos disponibles
(`std::thread::hardware_concurrency()`); puedes forzar un número con
`--threads N`.

### Alfabeto

Σ = minúsculas (incluye ñ) + mayúsculas (incluye Ñ) + dígitos + símbolos
`{!,@,#,$,%}` = 27 + 27 + 10 + 5 = **69 símbolos**. La ñ/Ñ ocupan 2 bytes en
UTF-8; el programa las trata como símbolos atómicos (no como 2 caracteres
sueltos).

### Política del equipo (semilla 4613)

```
minLower  = 2 + (4613 mod 3) = 4
minUpper  = 1 + (4613 mod 2) = 2
minDigit  = 1 + (4613 mod 3) = 3
minSymbol = 1
suma = 4+2+3+1 = 10 > n=8  ->  se reduce minLower en 2

Política final: minLower=2, minUpper=2, minDigit=3, minSymbol=1
sin caracteres idénticos consecutivos
```

### Ejecución

**Instancia de referencia común (validar antes de correr las propias):**
```bash
./bt referencia --examples 5 --stopfirst
```

**Las 5 variantes del equipo (Sección 9.2):**
```bash
./bt variante --id i   --examples 5 --stopfirst   # política completa, n=8
./bt variante --id ii  --examples 5 --stopfirst   # misma política, n=6
./bt variante --id iii --examples 5 --stopfirst   # misma política, n=10
./bt variante --id iv  --examples 5 --stopfirst   # relajada (solo minLower=1), n=8
./bt variante --id v   --examples 5 --stopfirst   # sin restricciones, n=6 ("poda nula")
```

**Con número de hilos explícito (si quieres comparar 1 vs N hilos, como en FB):**
```bash
./bt variante --id ii --threads 1
./bt variante --id ii --threads 24 --maxnodes 5000000
```

**Comparación con/sin poda (Sección 8.2) — instancia que sí completa entera:**
```bash
./bt custom --minLower 1 --minUpper 1 --minDigit 0 --minSymbol 0 --n 3
./bt custom --minLower 1 --minUpper 1 --minDigit 0 --minSymbol 0 --n 3 --sinpoda
```

**Comparación con/sin poda en instancias grandes (usar límite):**
```bash
./bt variante --id ii --sinpoda --maxnodes 5000000
./bt variante --id ii            --maxnodes 5000000
```

### Flags importantes (leer antes de correr nada a ciegas)

- `--examples K` : cuántas soluciones de ejemplo imprimir (no limita el conteo total a menos que se combine con `--stopfirst`).
- `--stopfirst` : detiene **toda** la búsqueda apenas se junten K ejemplos. Úsalo para *validar* que el algoritmo funciona rápido, sin esperar el conteo completo del espacio.
- `--maxnodes N` : aborta tras visitar N nodos, marcando `completo=NO` en la salida. Úsalo para medir el "muro exponencial" en instancias que no van a terminar en un tiempo razonable.
- `--sinpoda` : corre la versión de fuerza bruta pura (sin factibilidad), para la comparación de la Sección 8.2. **Extremadamente costosa** — combínala siempre con `--maxnodes`.

### ⚠️ Advertencia de tratabilidad (importante para planear la experimentación)

Con |Σ|=69, el árbol completo de n=6 ya tiene ~1.08×10¹¹ nodos. Además, como
los mínimos de la política de referencia (2+1+1+1=5) están muy cerca de n=6,
la poda por factibilidad actúa **tarde** (solo en las últimas posiciones), así
que incluso la versión *con poda* puede visitar una fracción grande del árbol
si se intenta **contar todas** las soluciones. Recomendaciones:

1. Para **validar correctitud**: usa `--examples K --stopfirst` (encuentra K
   soluciones rápido y se detiene, sin intentar contar todo el espacio).
2. Para **medir el efecto de la poda** de forma completa y honesta, usa
   instancias más chicas donde el conteo total sí termine (por ejemplo, un
   `custom` con n=4 o n=5, o Σ reducido) y documenta esa decisión en el
   informe como parte del "punto donde el crecimiento deja de ser manejable"
   que pide la Sección 8.
3. Para las variantes i/ii/iii/v tal como están definidas en el enunciado
   (n=8, n=10, |Σ|=69), es esperable que el conteo exhaustivo completo no
   termine en un computador personal — de hecho, identificar y documentar
   ese límite con evidencia (tiempos hasta el corte) es parte de lo que pide
   la Sección 8 ("identificación explícita del punto a partir del cual el
   crecimiento del costo deja de ser manejable").

### Semilla e integrantes

Semilla del equipo: **4613**, calculada con todos los apellidos (paterno y
materno) de los 3 integrantes, orden alfabético:
`gomezgonzalesmachadorodriguezsanchezsanchez` (ver `tests/verificar_semilla.cpp`).
