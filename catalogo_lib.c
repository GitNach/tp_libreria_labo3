#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalogo_lib.h"

void agregar_libro(const char *titulo, const char *autor)
{
    FILE *fp;
    int id;

    fp = fopen(ARCHIVO_CATALOGO, "a");

    if (fp == NULL) {
        perror("Error al abrir " ARCHIVO_CATALOGO);
        exit(EXIT_FAILURE);
    }

    id = rand();

    fprintf(fp, "%d | %s | %s\n", id, titulo, autor);

    fclose(fp);
}

void listar_libros(void)
{
    FILE *fp;
    char linea[LARGO_LINEA];

    fp = fopen(ARCHIVO_CATALOGO, "r");

    if (fp == NULL) {
        return;
    }

    while (fgets(linea, LARGO_LINEA, fp) != NULL) {
        printf("%s", linea);
    }

    fclose(fp);
}

int buscar_libros(const char *texto)
{
    FILE *fp;
    char linea[LARGO_LINEA];
    char titulo[LARGO_TITULO];
    char autor[LARGO_AUTOR];
    int id;
    int coincidencias;

    coincidencias = 0;

    fp = fopen(ARCHIVO_CATALOGO, "r");

    if (fp == NULL) {
        return 0;
    }

    while (fgets(linea, LARGO_LINEA, fp) != NULL) {

        if (sscanf(linea,
                   "%d | %127[^|] | %127[^\n]",
                   &id,
                   titulo,
                   autor) == 3) {

            if (strstr(titulo, texto) != NULL ||
                strstr(autor, texto) != NULL) {

                printf("%d | %s | %s\n", id, titulo, autor);
                coincidencias++;
            }
        }
    }

    fclose(fp);

    return coincidencias;
}
