#ifndef CATALOGO_LIB_H
#define CATALOGO_LIB_H

#define ARCHIVO_CATALOGO "catalogo.txt"

#define LARGO_TITULO 128
#define LARGO_AUTOR 128
#define LARGO_LINEA 512

void agregar_libro(const char *titulo, const char *autor);

void listar_libros(void);

int buscar_libros(const char *texto);

#endif
