# Pseudocódigo — Módulo FB (Fuerza Bruta)

## Algoritmo principal: enumeración exhaustiva por odómetro

```
FUNCION fuerzaBruta(hashObjetivo, alfabeto Σ, longitud n) -> (encontrada, textoPlano, candidatosEvaluados)

    idx[0..n-1] <- 0                      // vector de índices, uno por posición
    candidato[0..n-1] <- Σ[0]             // candidato inicial: todo ceros
    contador <- 0

    MIENTRAS verdadero HACER
        PARA i DESDE 0 HASTA n-1 HACER
            candidato[i] <- Σ[idx[i]]
        FIN PARA

        h <- SHA256(candidato)
        contador <- contador + 1

        SI h = hashObjetivo ENTONCES
            RETORNAR (verdadero, candidato, contador)
        FIN SI

        // Incrementar el odómetro: posición más a la derecha primero,
        // con acarreo hacia la izquierda (como un contador en base |Σ|)
        pos <- n - 1
        MIENTRAS pos >= 0 HACER
            idx[pos] <- idx[pos] + 1
            SI idx[pos] < |Σ| ENTONCES
                SALIR_DEL_BUCLE_INTERNO
            FIN SI
            idx[pos] <- 0
            pos <- pos - 1
        FIN MIENTRAS

        SI pos < 0 ENTONCES          // se agotó Σ^n completo sin éxito
            RETORNAR (falso, "", contador)
        FIN SI
    FIN MIENTRAS

FIN FUNCION
```

**Corrección (completitud y no repetición):** el vector `idx` representa un número
en base |Σ| de n dígitos. Incrementar el dígito menos significativo con acarreo
hacia los más significativos recorre, en orden, todos los enteros de 0 a
|Σ|ⁿ − 1 exactamente una vez, sin omisiones ni repeticiones. Cada entero
corresponde biunívocamente a una cadena de Σⁿ, luego el procedimiento enumera
Σⁿ exactamente una vez.

## Paralelización (partición del espacio)

```
FUNCION fuerzaBrutaParalela(hashObjetivo, Σ, n, numHilos) -> resultado

    encontrado <- falso              // bandera atómica compartida
    contadorGlobal <- 0              // contador atómico compartido

    PARA CADA hilo k DESDE 0 HASTA numHilos-1 EN PARALELO HACER
        PARA primero DESDE k HASTA |Σ|-1 CON PASO numHilos HACER
            SI encontrado ENTONCES RETORNAR
            // recorre con un odómetro de n-1 posiciones todas las cadenas
            // que empiezan con Σ[primero]
            resultadoParcial <- odometro(Σ, n-1, prefijo=Σ[primero], hashObjetivo)
            contadorGlobal <- contadorGlobal + resultadoParcial.evaluados
            SI resultadoParcial.encontrado ENTONCES
                encontrado <- verdadero
                GUARDAR resultadoParcial.texto
            FIN SI
        FIN PARA
    FIN PARA CADA

    ESPERAR a que todos los hilos terminen
    RETORNAR (encontrado, textoEncontrado, contadorGlobal)

FIN FUNCION
```

**Correctitud de la partición:** cada hilo `k` recorre los valores de la primera
posición cuyo índice cumple `índice mod numHilos = k`. La unión de esos
subconjuntos sobre todos los hilos es el conjunto completo {0, ..., |Σ|-1}, y
son disjuntos entre sí, por lo que no hay solapamiento ni candidatos sin cubrir.

## Ataque por diccionario (Sección 8.1, no exhaustivo)

```
FUNCION ataquePorDiccionario(hashObjetivo, rutaDiccionario) -> (encontrada, texto, evaluados)

    contador <- 0
    PARA CADA linea EN archivo(rutaDiccionario) HACER
        contador <- contador + 1
        SI SHA256(linea) = hashObjetivo ENTONCES
            RETORNAR (verdadero, linea, contador)
        FIN SI
    FIN PARA CADA

    RETORNAR (falso, "", contador)   // la contraseña no está en el diccionario;
                                       // esto NO implica que no exista una solución,
                                       // solo que no está en esta lista finita
FIN FUNCION
```
