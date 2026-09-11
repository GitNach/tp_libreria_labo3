#ifndef ESTACIONAMIENTO_H
#define ESTACIONAMIENTO_H

#define SHM_NOMBRE "/estacionamiento_buffer"
#define SEM_VACIOS_NOMBRE "/estacionamiento_vacios"
#define SEM_LLENOS_NOMBRE "/estacionamiento_llenos"
#define SEM_MUTEX_NOMBRE "/estacionamiento_mutex"

#define CAPACIDAD_BUFFER 10
#define LARGO_PATENTE 8

struct Vehiculo
{
    int ticket;
    char patente[LARGO_PATENTE];
};

struct BufferEstacionamiento
{
    struct Vehiculo vehiculos[CAPACIDAD_BUFFER];
    int cabeza;
    int cola;
};

void generar_patente(char *patente);
void preparar_vehiculo(struct Vehiculo *vehiculo, int ticket);
void ingresar_vehiculo(struct BufferEstacionamiento *buffer,
                       const struct Vehiculo *vehiculo);
void retirar_vehiculo(struct BufferEstacionamiento *buffer,
                      struct Vehiculo *vehiculo);

#endif
