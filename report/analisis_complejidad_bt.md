# Análisis de complejidad — Módulo BT

## Complejidad temporal y espacial (teórica)

En el peor caso, backtracking conserva la cota exponencial de fuerza bruta:
si la política no restringe efectivamente nada (variante v, "poda nula"),
el árbol recorrido es del mismo orden que el árbol completo, **O(|Σ|ⁿ)**.
La poda no cambia el orden de complejidad — reduce la constante multiplicativa
al evitar generar subárboles completos que se sabe de antemano que son
inválidos.

La complejidad espacial está gobernada por la profundidad del árbol: O(n)
para la pila de recursión (cada hilo mantiene su propio prefijo parcial de
tamaño ≤ n), más el costo de almacenar los ejemplos solicitados si se piden
explícitamente.

## Tamaño teórico del espacio por instancia

| n | \|Σ\|ⁿ (Σ=69) |
|---|---|
| 6 | 107,918,163,081 (~1.08×10¹¹) |
| 8 | 513,798,374,428,641 (~5.14×10¹⁴) |
| 10 | 2,446,194,060,654,759,801 (~2.45×10¹⁸) |

Ninguna de las instancias del equipo con n≥8 es exhaustivamente enumerable en
un computador personal — esto **no es una falla de la implementación**, es
justamente el "muro exponencial" que la Sección 8 pide identificar y
documentar con evidencia.

## Resultados medidos (máquina real, i7-13700K, 24 hilos, tope de 50M nodos)

| Instancia | n | Política | nodesVisited | prunedBranches | validFound (parcial) | Tiempo (ms) | Completo |
|---|---|---|---|---|---|---|---|
| referencia | 6 | min=2,1,1,1 | 14 | 174 | 5 (con `--stopfirst`) | 0.94 | no aplica* |
| i | 8 | equipo (2,2,3,1) | 50,000,003 | 323,903,552 | 44,581,041 | 11,179.3 | **NO** |
| ii | 6 | equipo (2,2,3,1) | **1** | 69 | **0** | 1.24 | **SÍ (exacto)** |
| iii | 10 | equipo (2,2,3,1) | 50,000,007 | 324,354,279 | 44,574,461 | 7,106.25 | **NO** |
| iv | 8 | relajada (1,0,0,0) | 50,000,009 | 735,354 | 49,264,575 | 2,449.49 | **NO** |
| v | 6 | vacía (poda nula) | 50,000,006 | 0 | 49,275,280 | 3,371.17 | **NO** |

\* la fila de referencia se corrió con `--stopfirst` (se detiene tras 5
ejemplos), por lo que su bajo nodesVisited no es representativo de la
dificultad completa de esa instancia — solo confirma correctitud.

## Interpretación: efectividad de la poda por instancia

- **Instancia ii (mejor caso posible):** poda perfecta. Como los mínimos
  exigidos (2+2+3+1=8) exceden n=6, ninguna cadena de longitud 6 puede
  satisfacerlos — la infactibilidad es detectable desde el primer nivel del
  árbol. Se prueban los 69 símbolos posibles en la raíz, los 69 se podan de
  inmediato, y la búsqueda termina en **1 nodo** frente a un espacio teórico
  de ~1.08×10¹¹. Esto demuestra el caso ideal descrito en la Sección 6.2: "el
  mejor caso ocurre cuando la política es tan restrictiva que casi cualquier
  prefijo se descarta tempranamente."

- **Instancias i y iii (política ajustada, restrictiva pero satisfacible):**
  la razón `prunedBranches / nodesVisited` es ~6.5× en ambas — por cada nodo
  efectivamente visitado, se evitó generar en promedio 6.5 ramas completas.
  Aun así, no alcanzó a completarse en 50M nodos (tope), lo cual es evidencia
  del muro exponencial para n≥8 con esta política.

- **Instancia iv (política relajada, solo minLower=1):** la razón
  `prunedBranches / nodesVisited` es de apenas ~0.015 — la poda casi no actúa,
  porque con un solo requisito mínimo (fácil de satisfacer) casi cualquier
  prefijo sigue siendo factible hasta el final. Coherente con la teoría: una
  política débil no genera oportunidades de poda.

- **Instancia v (poda nula, sin restricciones de composición):**
  `prunedBranches=0` exactamente — es el caso de calibración: sin ninguna
  restricción de composición, la única poda posible es la de "no repetidos
  consecutivos", que aquí también está desactivada. El árbol recorrido es
  literalmente Σⁿ sin reducción alguna, confirmando el peor caso teórico.

## Throughput medido (nodos por segundo, 24 hilos)

| Instancia | Nodos/segundo |
|---|---|
| i (n=8, política estricta) | 4.47 M/s |
| iii (n=10, política estricta) | 7.04 M/s |
| iv (n=8, relajada) | 20.41 M/s |
| v (n=6, sin restricción) | 14.83 M/s |

Las instancias con política más restrictiva (i, iii) son más lentas por nodo
visitado porque cada nodo ejecuta la verificación de factibilidad completa
(4 comparaciones), mientras que iv y v hacen ese trabajo con resultado casi
siempre trivial (factible), y v ni siquiera evalúa la condición de repetidos
consecutivos.

## Comparación con/sin poda — instancia completa (n=3, validación de correctitud)

| Métrica | Con poda | Sin poda |
|---|---|---|
| nodesVisited | 185,344 | 333,340 (= Σ_{k=0}^{3} 69^k exacto) |
| validFound | 180,792 | 180,792 |
| Tiempo (ms) | 15.6 | 15.6 |

**validFound coincide exactamente entre ambas versiones** (180,792) — esto
es la verificación de correctitud que exige la Sección 8.2: la poda no
descarta soluciones válidas ni acepta inválidas, solo evita visitar nodos
que de antemano se sabe que no llevan a ninguna.

**Reducción del espacio de búsqueda: 44.4%** ((333,340−185,344)/333,340).
Nótese que para esta política (minLower=1, minUpper=1, sin otros mínimos) la
poda es moderada porque los requisitos son fáciles de satisfacer temprano;
para políticas más estrictas cercanas al límite de n (como la del equipo,
Sección 9.2), la poda es dramáticamente más efectiva, como muestra la
instancia ii (reducción del 100%, 1 nodo vs. ~10¹¹ teóricos).

## Mejor caso, caso promedio y peor caso (discusión)

- **Mejor caso** (observado, instancia ii): la poda descarta el árbol
  completo desde la raíz cuando los mínimos exigidos exceden n. Costo: O(|Σ|)
  (un solo nivel evaluado).
- **Peor caso** (observado, instancia v): sin restricciones de composición
  efectivas, la poda no actúa y el costo es O(|Σ|ⁿ), igual que fuerza bruta
  pura.
- **Caso promedio** (instancias i, iii, iv): depende de qué tan cerca estén
  los mínimos exigidos del límite n. Entre más ajustada la política respecto
  de n (más cerca de agotar las posiciones disponibles), más tarde en el
  árbol actúa la poda y más nodos se visitan antes de poder descartar una
  rama — visible en la razón `prunedBranches/nodesVisited`, que varía desde
  0 (v) hasta ~6.5× (i, iii) en las instancias medidas.
