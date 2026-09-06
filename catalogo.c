#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catalogo_lib.h"

static void mostrar_uso(const char *programa);
static int convertir_anio(const char *texto, int *anio);

static void mostrar_uso(const char *programa)
{
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "  %s agregar <titulo> <autor> <anio>\n", programa);
    fprintf(stderr, "  %s listar\n", programa);
    fprintf(stderr, "  %s buscar <texto>\n", programa);
}

static int convertir_anio(const char *texto, int *anio)
{
    char extra;

    if (sscanf(texto, "%d %c", anio, &extra) != 1 || *anio <= 0) {
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    int anio;
    int coincidencias;

    if (argc < 2) {
        mostrar_uso(argv[0]);
        return EXIT_FAILURE;
    }

    srand((unsigned int) time(NULL));

    if (strcmp(argv[1], "agregar") == 0) {
        if (argc != 5 || !convertir_anio(argv[4], &anio)) {
            mostrar_uso(argv[0]);
            return EXIT_FAILURE;
        }

        if (!agregar_libro(argv[2], argv[3], anio)) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "listar") == 0) {
        if (argc != 2 || !listar_libros()) {
            if (argc != 2) {
                mostrar_uso(argv[0]);
            }
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "buscar") == 0) {
        if (argc != 3) {
            mostrar_uso(argv[0]);
            return EXIT_FAILURE;
        }

        coincidencias = buscar_libros(argv[2]);
        if (coincidencias < 0) {
            return EXIT_FAILURE;
        }

        printf("Coincidencias: %d\n", coincidencias);
        return EXIT_SUCCESS;
    }

    mostrar_uso(argv[0]);
    return EXIT_FAILURE;
}
