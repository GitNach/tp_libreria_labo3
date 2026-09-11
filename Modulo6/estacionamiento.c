#include <stdlib.h>

#include "estacionamiento.h"

void generar_patente(char *patente)
{
    patente[0] = (char)('A' + rand() % 26);
    patente[1] = (char)('A' + rand() % 26);
    patente[2] = (char)('A' + rand() % 26);
    patente[3] = (char)('0' + rand() % 10);
    patente[4] = (char)('0' + rand() % 10);
    patente[5] = (char)('0' + rand() % 10);
    patente[6] = '\0';
}

void preparar_vehiculo(struct Vehiculo *vehiculo, int ticket)
{
    vehiculo->ticket = ticket;
    generar_patente(vehiculo->patente);
}

void ingresar_vehiculo(struct BufferEstacionamiento *buffer,
                       const struct Vehiculo *vehiculo)
{
    buffer->vehiculos[buffer->cabeza] = *vehiculo;
    buffer->cabeza = (buffer->cabeza + 1) % CAPACIDAD_BUFFER;
}

void retirar_vehiculo(struct BufferEstacionamiento *buffer,
                      struct Vehiculo *vehiculo)
{
    *vehiculo = buffer->vehiculos[buffer->cola];
    buffer->cola = (buffer->cola + 1) % CAPACIDAD_BUFFER;
}
