/*
 * Tarea 5 - Algoritmo evolutivo (codificacion binaria) para la funcion
 * de Rastrigin de dos variables.
 *
 *   f(x,y) = 20 + [x^2 - 10 cos(2 pi x)] + [y^2 - 10 cos(2 pi y)]
 *   x, y en [-5.12, 5.12]        minimo global en (0,0) con f = 0
 *
 * Cromosoma: un unsigned long long (64 bits)
 *   [ 32 bits de x | 32 bits de y ]
 *
 * Decodificacion: X = A + (decimal / (2^L - 1)) * (B - A)
 *
 * Estrategia de reemplazo: los hijos sustituyen a los peores padres
 * (la poblacion siempre se mantiene en 50 individuos).
 *
 * Compilar: gcc algoritmo_genetico_rastrigin.c -o ag -lm
 * Ejecutar: ./ag [semilla]
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ------------------------- configuracion ------------------------- */
enum { TAM_POB = 50, GENERACIONES = 100, BITS_VAR = 32, BITS_TOTAL = 64 };

static const double PROB_CRUZA    = 0.9;
static const double PROB_MUTACION = 0.1;
static const double LIM_INF       = -5.12;
static const double LIM_SUP       =  5.12;
static const double MAX_DECIMAL   = 4294967295.0;      /* 2^32 - 1 */

typedef unsigned long long cromosoma;

/* mascaras para separar x (mitad alta) de y (mitad baja) */
static const cromosoma MASK_X = 0xFFFFFFFF00000000ULL;
static const cromosoma MASK_Y = 0x00000000FFFFFFFFULL;

typedef struct {
    cromosoma bits;     /* representacion binaria xy       */
    double    valor_x;  /* x real                          */
    double    valor_y;  /* y real                          */
    double    aptitud;  /* f(x,y); mientras menor, mejor   */
} Persona;

/* ---------------------- numeros aleatorios ----------------------- */
static double aleatorio_unitario(void) {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

static int aleatorio_entero(int limite) {          /* [0, limite) */
    return (int)(aleatorio_unitario() * limite);
}

static cromosoma cromosoma_aleatorio(void) {
    cromosoma c = 0;
    for (int i = 0; i < BITS_TOTAL; i++)           /* bit por bit */
        c = (c << 1) | (cromosoma)(aleatorio_entero(2));
    return c;
}

/* ------------------------ funcion objetivo ----------------------- */
static double a_real(cromosoma decimal) {
    return LIM_INF + (decimal / MAX_DECIMAL) * (LIM_SUP - LIM_INF);
}

static double rastrigin(double x, double y) {
    const double dos_pi = 2.0 * acos(-1.0);
    double termino_x = x * x - 10.0 * cos(dos_pi * x);
    double termino_y = y * y - 10.0 * cos(dos_pi * y);
    return 20.0 + termino_x + termino_y;
}

static void calcular_aptitud(Persona *p) {
    cromosoma parte_x = (p->bits & MASK_X) >> BITS_VAR;
    cromosoma parte_y =  p->bits & MASK_Y;
    p->valor_x = a_real(parte_x);
    p->valor_y = a_real(parte_y);
    p->aptitud = rastrigin(p->valor_x, p->valor_y);
}

static Persona crear_persona(cromosoma bits) {
    Persona p;
    p.bits = bits;
    calcular_aptitud(&p);
    return p;
}

/* --------------------- operadores geneticos ---------------------- */
/* Cruza: el hijo A hereda x del padre 1 e y del padre 2; el B al reves */
static void cruza_por_mascara(cromosoma padre1, cromosoma padre2,
                              cromosoma *hijoA, cromosoma *hijoB) {
    *hijoA = (padre1 & MASK_X) | (padre2 & MASK_Y);
    *hijoB = (padre2 & MASK_X) | (padre1 & MASK_Y);
}

/* Mutacion: con probabilidad PROB_MUTACION se voltea un bit elegido al azar */
static cromosoma mutacion_un_bit(cromosoma c) {
    if (aleatorio_unitario() < PROB_MUTACION) {
        cromosoma mascara = 1ULL << aleatorio_entero(BITS_TOTAL);
        c ^= mascara;
    }
    return c;
}

/* --------------------------- utilidades -------------------------- */
static int menor_aptitud_primero(const void *pa, const void *pb) {
    const Persona *a = pa, *b = pb;
    if (a->aptitud < b->aptitud) return -1;
    if (a->aptitud > b->aptitud) return  1;
    return 0;
}

static void mostrar_binario(cromosoma c) {
    for (int pos = BITS_TOTAL - 1; pos >= 0; pos--) {
        cromosoma mascara = 1ULL << pos;
        putchar((c & mascara) ? '1' : '0');
        if (pos == BITS_VAR) putchar(' ');
    }
}

static void reporte(int gen, const Persona *mejor) {
    printf("%-5d %-12.6f %-12.6f %-12.6f\n",
           gen, mejor->valor_x, mejor->valor_y, mejor->aptitud);
}

/* ----------------------------- main ------------------------------ */
int main(int argc, char *argv[]) {
    unsigned int semilla = (argc > 1) ? (unsigned int)strtoul(argv[1], NULL, 10)
                                      : (unsigned int)time(NULL);
    srand(semilla);

    Persona poblacion[TAM_POB];
    Persona hijos[TAM_POB];

    printf("Algoritmo evolutivo | Rastrigin 2D | poblacion=%d generaciones=%d "
           "cruza=%.2f mutacion=%.2f semilla=%u\n\n",
           TAM_POB, GENERACIONES, PROB_CRUZA, PROB_MUTACION, semilla);

    /* Pasos 1 y 2: poblacion inicial aleatoria, evaluada y ordenada */
    for (int i = 0; i < TAM_POB; i++)
        poblacion[i] = crear_persona(cromosoma_aleatorio());
    qsort(poblacion, TAM_POB, sizeof(Persona), menor_aptitud_primero);

    printf("%-5s %-12s %-12s %-12s\n", "Gen", "x", "y", "f(x,y)");
    reporte(0, &poblacion[0]);

    /* Paso 7: repetir pasos 3 a 6 */
    for (int gen = 1; gen <= GENERACIONES; gen++) {

        /* Pasos 3 y 4: generar TAM_POB hijos (cruza + mutacion) */
        for (int k = 0; k < TAM_POB; k += 2) {
            cromosoma papa = poblacion[aleatorio_entero(TAM_POB)].bits;
            cromosoma mama = poblacion[aleatorio_entero(TAM_POB)].bits;
            cromosoma c1 = papa, c2 = mama;

            if (aleatorio_unitario() < PROB_CRUZA)
                cruza_por_mascara(papa, mama, &c1, &c2);

            hijos[k] = crear_persona(mutacion_un_bit(c1));     /* paso 5 */
            if (k + 1 < TAM_POB)
                hijos[k + 1] = crear_persona(mutacion_un_bit(c2));
        }

        /* Paso 6: ordenar hijos y dejar que sustituyan a los peores padres.
         * Se conservan los mejores padres (elitismo): los 25 mejores padres
         * se quedan y los 25 mejores hijos ocupan el resto.               */
        qsort(hijos, TAM_POB, sizeof(Persona), menor_aptitud_primero);
        int mitad = TAM_POB / 2;
        for (int i = 0; i < TAM_POB - mitad; i++)
            poblacion[mitad + i] = hijos[i];
        qsort(poblacion, TAM_POB, sizeof(Persona), menor_aptitud_primero);

        if (gen % 10 == 0) reporte(gen, &poblacion[0]);
    }

    printf("\nMejor solucion encontrada\n  cromosoma = ");
    mostrar_binario(poblacion[0].bits);
    printf("\n  x = %.6f\n  y = %.6f\n  f(x,y) = %.6f\n",
           poblacion[0].valor_x, poblacion[0].valor_y, poblacion[0].aptitud);
    return 0;
}
