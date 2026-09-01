# Pseudocódigo — Módulo BT (Backtracking)

## Representación del estado

Un estado parcial es un prefijo de longitud k < n, junto con:
- contador de símbolos por clase ya usados (minúsculas, mayúsculas, dígitos, símbolos)
- el último símbolo agregado (para verificar la prohibición de repetidos consecutivos)

## Backtracking CON poda

```
FUNCION backtrackConPoda(prefijo, estado, n, politica)

    SI |prefijo| = n ENTONCES
        SI cumplePolitica(prefijo, politica) ENTONCES
            REGISTRAR prefijo como solucion valida
        FIN SI
        RETORNAR
    FIN SI

    PARA CADA simbolo s EN Σ HACER

        // Poda 1: prohibicion de dos caracteres identicos consecutivos
        SI politica.sinRepetidos Y prefijo no vacio Y s = ultimo(prefijo) ENTONCES
            PODAR esta rama (no generar el hijo)
            CONTINUAR con el siguiente simbolo
        FIN SI

        estadoSiguiente <- estado + clasificar(s)
        restantes <- n - |prefijo| - 1

        // Poda 2: factibilidad de completar los minimos de la politica
        faltantes <- max(0, minLower  - estadoSiguiente.nLower)
                   + max(0, minUpper  - estadoSiguiente.nUpper)
                   + max(0, minDigit  - estadoSiguiente.nDigit)
                   + max(0, minSymbol - estadoSiguiente.nSymbol)

        SI faltantes > restantes ENTONCES
            PODAR esta rama (no generar el hijo; ninguna extension puede
                              cumplir los minimos exigidos)
            CONTINUAR con el siguiente simbolo
        FIN SI

        // Rama factible: se extiende la busqueda
        backtrackConPoda(prefijo + s, estadoSiguiente, n, politica)

    FIN PARA

FIN FUNCION
```

**Justificación de la poda:** si en un estado con k posiciones ya fijas
todavía faltan `faltantes` símbolos de alguna clase por incluir, y solo
quedan `restantes = n - k` posiciones libres, entonces `faltantes > restantes`
implica que ninguna extensión del prefijo puede alcanzar los mínimos exigidos
— la rama completa (todos sus descendientes) es inválida sin necesidad de
generarlos ni evaluarlos.

## Enumeración exhaustiva SIN poda (Sección 8.2)

```
FUNCION backtrackSinPoda(prefijo, n, politica)

    SI |prefijo| = n ENTONCES
        SI cumplePolitica(prefijo, politica) ENTONCES
            REGISTRAR prefijo como solucion valida
        FIN SI
        RETORNAR
    FIN SI

    PARA CADA simbolo s EN Σ HACER
        // Sin verificacion de factibilidad: se generan TODOS los hijos
        backtrackSinPoda(prefijo + s, n, politica)
    FIN PARA

FIN FUNCION
```

La única diferencia estructural con la versión podada es que aquí **no** se
evalúa la factibilidad antes de recursar: se genera y visita cada nodo del
árbol completo (tamaño Σ_{k=0}^{n} |Σ|^k), filtrando la política solo al
llegar a las hojas. Esto es lo que hace posible cuantificar, nodo a nodo,
cuánto ahorra la poda en la versión con poda.

## Verificación de política sobre una cadena completa

```
FUNCION cumplePolitica(s, politica) -> booleano
    nLower, nUpper, nDigit, nSymbol <- 0

    PARA i DESDE 0 HASTA |s|-1 HACER
        clasificar s[i] y sumar al contador correspondiente
        SI i > 0 Y politica.sinRepetidos Y s[i] = s[i-1] ENTONCES
            RETORNAR falso
        FIN SI
    FIN PARA

    RETORNAR (nLower >= politica.minLower) Y (nUpper >= politica.minUpper)
         Y   (nDigit >= politica.minDigit) Y (nSymbol >= politica.minSymbol)
FIN FUNCION
```

## Nota sobre el alfabeto (69 símbolos)

Σ = {a..z, ñ} ∪ {A..Z, Ñ} ∪ {0..9} ∪ {!,@,#,$,%} = 27 + 27 + 10 + 5 = **69**
símbolos. La ñ/Ñ se cuentan como minúscula/mayúscula respectivamente para
efectos de la política de composición.

## Paralelización

```
FUNCION backtrackConPodaParalela(n, politica, numHilos) -> resultado

    contadorGlobal, solucionesGlobal, podadasGlobal <- 0 (atomicos, compartidos)
    ejemplosCompartidos <- [] (protegida por mutex)

    PARA CADA hilo k DESDE 0 HASTA numHilos-1 EN PARALELO HACER
        // cada hilo entra a la recursion desde el prefijo VACIO, pero en el
        // primer nivel (la raiz) solo itera los simbolos cuyo indice
        // cumple indice mod numHilos = k; en los niveles siguientes recorre
        // el alfabeto completo, como en la version secuencial
        backtrackConPoda(prefijo="", estado, n, politica, k, numHilos)
    FIN PARA CADA

    ESPERAR a que todos los hilos terminen
    RETORNAR resultado consolidado

FIN FUNCION
```

**Correctitud de la partición:** el nodo raíz (prefijo vacío) es conceptualmente
uno solo; cada hilo simplemente explora una porción disjunta de sus hijos
directos (los símbolos con `índice mod numHilos = k`). La unión de esas
porciones sobre todos los hilos cubre el árbol completo sin solapamiento —
misma idea que la partición usada en el Módulo FB, aplicada aquí al primer
nivel del árbol de backtracking en vez de al primer carácter del candidato.
