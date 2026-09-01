# Análisis de complejidad — Módulo FB

## Complejidad temporal

Sea Σ el alfabeto usado y n la longitud de la contraseña. El algoritmo evalúa,
en el peor caso, cada uno de los |Σ|ⁿ candidatos del espacio, y cada evaluación
cuesta calcular un SHA-256 y comparar 32 bytes. Para longitudes de contraseña
acotadas (n ≤ 6 en esta práctica), el costo de SHA-256 es O(1) respecto de n
(el hash procesa bloques de 64 bytes; una cadena de a lo sumo 6 caracteres cabe
siempre en un único bloque, así que el costo por hash no depende de n dentro
del rango evaluado).

**Complejidad temporal: O(|Σ|ⁿ)** en el peor caso (contraseña no encontrada, o
encontrada al final del recorrido). En el mejor caso, O(1) (la contraseña es el
primer candidato generado). El caso promedio, asumiendo una posición uniforme
del objetivo en el espacio, es O(|Σ|ⁿ / 2).

## Complejidad espacial

El algoritmo mantiene un único candidato activo (`std::string` de tamaño n) y
un vector de índices de tamaño n para el odómetro — **O(n)**, independiente de
|Σ|ⁿ. Esto contrasta con la alternativa incorrecta de generar y almacenar todo
el espacio antes de evaluarlo, que sería O(|Σ|ⁿ) en memoria e inviable incluso
para n=4.

## Contraste teórico vs. empírico (datos medidos en máquina real, 24 hilos, semilla 4613)

| Instancia | Alfabeto | n | \|Σ\|ⁿ (teórico) | Candidatos evaluados (suma de 24 hilos) | Tiempo (ms) |
|---|---|---|---|---|---|
| referencia | A2 | 5 | 60,466,176 | 1,557,577 | 156.3 |
| eq1 | A1 | 4 | 456,976 | 415,748 | 46.2 |
| eq2 | A2 | 4 | 1,679,616 | 1,108,814 | 112.5 |
| eq3 | A1 | 5 | 11,881,376 | 4,187,713 | 422.8 |
| eq4 | A2 | 5 | 60,466,176 | 39,039,772 | 4,013.2 |
| eq5 | A1 | 6 | 308,915,776 | 152,440,786 | 17,920.5 |

**Nota sobre "candidatos evaluados":** con 24 hilos trabajando en paralelo, este
número no es directamente comparable al de una corrida secuencial: cuando un
hilo encuentra la contraseña, los demás hilos alcanzan a evaluar algunos
candidatos adicionales antes de detectar la bandera de "encontrado" (chequeo
periódico, no instantáneo). Por eso la suma de candidatos evaluados en
paralelo suele ser mayor que en una corrida de 1 solo hilo para la misma
instancia — es el costo de coordinación del paralelismo, no un error.

## Efecto medido del paralelismo (1 hilo vs 24 hilos)

Se corrieron las mismas 4 instancias (eq1–eq4) en un entorno de 1 solo núcleo
y en la máquina real del equipo (24 hilos, i7-13700K):

| Instancia | Tiempo 1 hilo (ms) | Tiempo 24 hilos (ms) | Speedup |
|---|---|---|---|
| eq1 (A1, n=4) | 385.9 | 46.2 | 8.4× |
| eq2 (A2, n=4) | 191.1 | 112.5 | 1.7× |
| eq3 (A1, n=5) | 191.1 | 422.8 | 0.45× (más lento) |
| eq4 (A2, n=5) | 40,267.4 | 4,013.2 | 10.0× |

**Observación importante:** el speedup no es uniforme ni siempre positivo.
eq3 fue *más lento* en paralelo, porque en la corrida de 1 hilo la contraseña
objetivo cayó muy cerca del inicio del espacio de búsqueda (solo 124,481
candidatos de 11.9M, hallada casi de inmediato), mientras que con 24 hilos el
overhead de arrancar/coordinar los hilos y el hecho de que la partición por
primer carácter no garantiza que el hilo "afortunado" arranque de inmediato
dominan el tiempo total. Esto ilustra un punto importante: el paralelismo
reduce el tiempo esperado (caso promedio) al dividir el espacio entre más
trabajadores, pero **no garantiza mejora en cada corrida individual**,
especialmente cuando la solución está cerca del principio del espacio y el
camino secuencial ya la encuentra casi de inmediato. Para instancias donde se
recorre una fracción sustancial del espacio (eq4: 39M de 60M candidatos), el
beneficio del paralelismo es claro y consistente (10×, cercano al ideal de 24×
menos el overhead de coordinación y el desbalance de carga entre hilos).

**Observaciones sobre los datos de 24 hilos:**

- El número de candidatos evaluados en cada instancia es menor que |Σ|ⁿ
  porque la búsqueda se detiene apenas se encuentra la contraseña objetivo.
  eq4 recorrió el 64.6% del espacio teórico (39.0M de 60.5M) antes de
  encontrar `p6ncd`, mientras que eq1 recorrió el 91% (415,748 de 456,976) —
  la variación depende de en qué posición cae la contraseña generada por el
  LCG dentro del espacio, no solo de n y |Σ|.
- El "muro exponencial" es visible incluso con 24 hilos: eq4 y eq5 (decenas
  y cientos de millones de candidatos) toman segundos, mientras que eq1–eq3
  toman decenas o cientos de milisegundos.
- De eq3 (A1, n=5, espacio=11.9M) a eq5 (A1, n=6, espacio=308.9M) el espacio
  teórico crece ×26 (exactamente |Σ|), y el tiempo medido crece de 422.8 ms
  a 17,920.5 ms (~42×) — más que el factor teórico porque eq3 se resolvió
  temprano (35% del espacio) mientras que eq5 recorrió una fracción mayor
  (49% del espacio) antes de encontrar la solución. El paralelismo reduce la
  constante de tiempo, no el orden de complejidad |Σ|ⁿ.

## Relación tamaño–costo

Cada incremento de una unidad en n multiplica el tamaño teórico del espacio por
un factor de |Σ| (26 para A1, 36 para A2). Esto es exactamente lo que se observa
al pasar de n=4 a n=5 en A1: el espacio teórico crece de 456,976 a 11,881,376,
un factor de 26.0, tal como predice la fórmula |Σ|ⁿ.
