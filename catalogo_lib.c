#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalogo_lib.h"

static int leer_registro(const char *linea, int *id, char *titulo,
                         char *autor, int *anio);
static void quitar_espacios_extremos(char *texto);
static int texto_valido(const char *texto, int limite);
static int id_existente(int id);
static int generar_id(void);

static int leer_registro(const char *linea, int *id, char *titulo,
                         char *autor, int *anio)
{
    if (sscanf(linea, "%d | %127[^|] | %127[^|] | %d",
               id, titulo, autor, anio) != 4) {
        return 0;
    }

    quitar_espacios_extremos(titulo);
    quitar_espacios_extremos(autor);

    return titulo[0] != '\0' && autor[0] != '\0';
}

static void quitar_espacios_extremos(char *texto)
{
    char *inicio;
    size_t largo;

    inicio = texto;

    while (*inicio == ' ' || *inicio == '\t') {
        inicio++;
    }

    if (inicio != texto) {
        memmove(texto, inicio, strlen(inicio) + 1);
    }

    largo = strlen(texto);
    while (largo > 0 && (texto[largo - 1] == ' ' ||
                         texto[largo - 1] == '\t' ||
                         texto[largo - 1] == '\n' ||
                         texto[largo - 1] == '\r')) {
        texto[largo - 1] = '\0';
        largo--;
    }
}

static int texto_valido(const char *texto, int limite)
{
    size_t i;

    if (texto == NULL || texto[0] == '\0' ||
        strlen(texto) >= (size_t)limite) {
        return 0;
    }

    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] == '|' || texto[i] == '\n' || texto[i] == '\r') {
            return 0;
        }
    }

    return 1;
}

static int id_existente(int id)
{
    FILE *fp;
    char linea[LARGO_LINEA];
    char titulo[LARGO_TITULO];
    char autor[LARGO_AUTOR];
    int id_guardado;
    int anio;

    fp = fopen(ARCHIVO_CATALOGO, "r");
    if (fp == NULL) {
        return 0;
    }

    while (fgets(linea, LARGO_LINEA, fp) != NULL) {
        if (leer_registro(linea, &id_guardado, titulo, autor, &anio) &&
            id_guardado == id) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

static int generar_id(void)
{
    int intento;
    int id;

    for (intento = 0; intento < 100; intento++) {
        id = rand() % 9000 + 1000;
        if (!id_existente(id)) {
            return id;
        }
    }

    return -1;
}

int agregar_libro(const char *titulo, const char *autor, int anio)
{
    FILE *fp;
    int id;

    if (!texto_valido(titulo, LARGO_TITULO) ||
        !texto_valido(autor, LARGO_AUTOR) || anio <= 0) {
        fprintf(stderr, "Datos invalidos para el libro.\n");
        return 0;
    }

    id = generar_id();
    if (id < 0) {
        fprintf(stderr, "No se pudo generar un ID disponible.\n");
        return 0;
    }

    fp = fopen(ARCHIVO_CATALOGO, "a");

    if (fp == NULL) {
        perror("Error al abrir " ARCHIVO_CATALOGO);
        return 0;
    }

    if (fprintf(fp, "%d|%s|%s|%d\n", id, titulo, autor, anio) < 0) {
        perror("Error al escribir " ARCHIVO_CATALOGO);
        fclose(fp);
        return 0;
    }

    if (fclose(fp) != 0) {
        perror("Error al cerrar " ARCHIVO_CATALOGO);
        return 0;
    }

    printf("Libro agregado con ID: %d\n", id);
    return 1;
}

int listar_libros(void)
{
    FILE *fp;
    char linea[LARGO_LINEA];

    fp = fopen(ARCHIVO_CATALOGO, "r");

    if (fp == NULL) {
        perror("Error al abrir " ARCHIVO_CATALOGO);
        return 0;
    }

    while (fgets(linea, LARGO_LINEA, fp) != NULL) {
        printf("%s", linea);
    }

    if (ferror(fp)) {
        perror("Error al leer " ARCHIVO_CATALOGO);
        fclose(fp);
        return 0;
    }

    if (fclose(fp) != 0) {
        perror("Error al cerrar " ARCHIVO_CATALOGO);
        return 0;
    }

    return 1;
}

int buscar_libros(const char *texto)
{
    FILE *fp;
    char linea[LARGO_LINEA];
    char titulo[LARGO_TITULO];
    char autor[LARGO_AUTOR];
    int id;
    int anio;
    int coincidencias;

    coincidencias = 0;

    if (!texto_valido(texto, LARGO_LINEA)) {
        fprintf(stderr, "El texto de busqueda no es valido.\n");
        return -1;
    }

    fp = fopen(ARCHIVO_CATALOGO, "r");

    if (fp == NULL) {
        perror("Error al abrir " ARCHIVO_CATALOGO);
        return -1;
    }

    while (fgets(linea, LARGO_LINEA, fp) != NULL) {
        if (leer_registro(linea, &id, titulo, autor, &anio)) {
            if (strstr(titulo, texto) != NULL ||
                strstr(autor, texto) != NULL) {

                printf("%d|%s|%s|%d\n", id, titulo, autor, anio);
                coincidencias++;
            }
        }
    }

    if (ferror(fp)) {
        perror("Error al leer " ARCHIVO_CATALOGO);
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0) {
        perror("Error al cerrar " ARCHIVO_CATALOGO);
        return -1;
    }

    return coincidencias;
}
