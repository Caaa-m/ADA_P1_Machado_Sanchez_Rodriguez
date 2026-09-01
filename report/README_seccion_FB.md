## Módulo FB — Fuerza Bruta (ataque a hash SHA-256)

### Compilación

```bash
g++ -std=c++17 -O2 -pthread -o fb src/Fb.cpp
```

### Ejecución

**Ataque de fuerza bruta contra un hash específico:**
```bash
./fb bruteforce --hash <hash_sha256_hex> --alphabet a1|a2 --n <longitud> [--threads N]
```
- `--alphabet a1` → minúsculas (26 símbolos), n ∈ {3,4,5,6}
- `--alphabet a2` → minúsculas + dígitos (36 símbolos), n ∈ {3,4,5}
- `--threads N` es opcional; por defecto usa todos los núcleos disponibles
  (`std::thread::hardware_concurrency()`).

Ejemplo (instancia de referencia del curso):
```bash
./fb bruteforce --hash 8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773 \
    --alphabet a2 --n 5
```

**Ataque por diccionario (comparación Sección 8.1):**
```bash
./fb dictionary --hash <hash_sha256_hex> --dict resources/diccionario.txt
```

**Correr todas las instancias (referencia + equipo) y exportar CSV:**
```bash
./fb batch --out results/fb_results.csv
```

### Tests

```bash
g++ -std=c++17 -O2 -pthread -o test_fb tests/test_fb.cpp
./test_fb
```
Verifica: enumeración sin repetidos/omisiones en espacios pequeños, coincidencia
del hash de la instancia de referencia, y validación de rangos de longitud por
alfabeto.

### Semilla e instancias del equipo

Semilla del equipo (todos los apellidos —paterno y materno— de los 3
integrantes, orden alfabético: Gómez, González, Machado, Rodríguez, Sánchez,
Sánchez): **4613**, verificable con `tests/verificar_semilla.cpp`.

Las 5 instancias del equipo se generaron con el LCG `x₀ = 4613`,
`x_{i+1} = (1103515245·x_i + 12345) mod 2³¹`, consumiendo la secuencia de
forma continua entre las 5 contraseñas (longitudes 4,4,5,5,6; alfabetos
alternados A1,A2,A1,A2,A1 — este orden es el único compatible con que n=6 no
es válido en A2; **verificar contra el enunciado exacto de InteractivaVirtual**).

| Instancia | Alfabeto | n | Hash SHA-256 |
|---|---|---|---|
| equipo_1 | A1 | 4 | 22072ff7764a1f68cb9d05e287d907e5e52cd88a6b9960f28b97b905cdb4a5a2 |
| equipo_2 | A2 | 4 | 5b78a6425aedd17a1683c982597afc0968030da8b1106b09fa7aaabecb85e5e4 |
| equipo_3 | A1 | 5 | 8308f1219366f836697fa47cd231ae591e6ef7ea09a09a49a0130f7fe1c104cb |
| equipo_4 | A2 | 5 | 81eb081297b8c50d7f27a7ea2ed3b9320fc253ef02338ade6ba727c50e37d5d3 |
| equipo_5 | A1 | 6 | dd03c5067c571911c07bc6fcab16a480d85e5a2ec2fc9200b01303ae8ec1d9aa |

Ninguna de las 5 contraseñas objetivo pertenece a `resources/diccionario.txt`
(verificado: 0/5 encontradas por el ataque de diccionario), lo cual es
esperado dado que se generan de forma pseudoaleatoria a partir de la semilla,
y es justamente el punto que la Sección 8.1 pide discutir.

**Estado:** las 6 instancias (referencia + 5 del equipo) fueron corridas y
verificadas exitosamente en la máquina real del equipo (i7-13700K, 24 hilos).
Ver `results/fb_results.csv` para los tiempos medidos.
