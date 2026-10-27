# Algoritmo genético: función de Rastrigin

Algoritmo genético con codificación binaria, escrito en C, que minimiza la
función de Rastrigin de dos variables.

**Autor:** Ernesto Michel Rodríguez Hernández

## El problema

```
f(x, y) = 20 + (x² − 10·cos(2πx)) + (y² − 10·cos(2πy))
x, y ∈ [−5.12, 5.12]
```

El mínimo global está en (0, 0) con f = 0. La función tiene muchísimos mínimos
locales, por eso es un problema clásico para probar algoritmos evolutivos.

## Representación

Cada individuo es una cadena de **64 bits** (`unsigned long long`):

```
bits 63..32  →  x (32 bits)
bits 31..0   →  y (32 bits)
```

Para obtener el valor real de cada variable se decodifica con:

```
X = A + (valor_decimal / (2^L − 1)) · (B − A)
```

con `A = −5.12`, `B = 5.12` y `L = 32`.

## Parámetros

| Parámetro | Valor |
|---|---|
| Individuos | 50 |
| Generaciones | 100 |
| Probabilidad de cruza | 0.9 |
| Probabilidad de mutación | 0.1 |
| Bits por variable | 32 |

Se pueden cambiar en la parte de configuración al inicio del código.

## Pasos del algoritmo

1. **Individuo**: cromosoma de 64 bits con su valor `f(x, y)`.
2. **Población inicial**: 50 individuos con bits aleatorios.
3. **Cruza**: se eligen dos padres al azar y, con probabilidad 0.9, se intercambian
   las mitades usando máscaras con AND/OR: un hijo toma los 32 bits de x de un
   padre y los de y del otro, y el segundo hijo al revés.
4. **Mutación**: a cada hijo, con probabilidad 0.1, se le invierte un bit
   al azar (máscara con un solo 1 y XOR).
5. **Evaluación**: se decodifican x e y y se calcula `f(x, y)`.
6. **Selección**: los hijos se ordenan con `qsort`; los 25 mejores reemplazan a los
   25 peores padres, de modo que la población siempre queda en 50 individuos.
7. Se repiten los pasos 3 a 6 durante todas las generaciones.

## Compilación y uso

```bash
gcc -O2 -Wall algoritmo_genetico_rastrigin.c -o ag -lm
./ag          # semilla según la hora
./ag 42       # semilla fija, para repetir resultados
```

La bandera `-lm` es necesaria para enlazar la biblioteca matemática (`cos`, `acos`).

## Ejemplo de salida (`./ag 42`)

```
Gen   x            y            f(x,y)
0     1.047212     -1.007196    2.558085
10    -0.022591    -1.007196    1.125744
50    -0.000003    -1.000315    1.000650
100   -0.000001    -1.000000    1.000000

Mejor solucion encontrada
  cromosoma = 01111111111111111111111011111111 01100110111111111111111111111101
  x = -0.000001
  y = -1.000000
  f(x,y) = 1.000000
```

No siempre se llega a f = 0: el algoritmo puede quedarse en un mínimo local,
como en este ejemplo, donde x converge a 0 pero y se queda en −1. Con otra semilla
el resultado cambia.
