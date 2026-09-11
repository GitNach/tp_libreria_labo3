# Catalogo de libros

Trabajo practico para Laboratorio III.

## Compilacion

```sh
make
```

El ejecutable generado se llama `catalogo` y `make` crea el archivo `catalogo.txt`.

## Uso

Agregar un libro:

```sh
./catalogo agregar "El nombre de la rosa" "Umberto Eco" 1980
```

Listar los libros:

```sh
./catalogo listar
```

Buscar por titulo o autor:

```sh
./catalogo buscar rosa
```

El formato de cada registro es:

```text
id|titulo|autor|anio
```

Los titulos y autores no pueden contener `|` ni saltos de linea. El ID se genera
aleatoriamente y se verifica que no este repetido.

## Archivos

- `catalogo.c`: programa principal y argumentos de consola.
- `catalogo_lib.h`: interfaz de la biblioteca.
- `catalogo_lib.c`: operaciones sobre el archivo y busquedas.
- `Makefile`: compilacion incremental y limpieza.
