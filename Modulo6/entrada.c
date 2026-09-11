#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>

#include "estacionamiento.h"

#define CANTIDAD_DEFAULT 20

static struct BufferEstacionamiento *buffer = NULL;
static sem_t *sem_vacios = NULL;
static sem_t *sem_llenos = NULL;
static sem_t *sem_mutex = NULL;

static int inicializar_recursos(void);
static void liberar_recursos(void);

static int inicializar_recursos(void)
{
    int descriptor;

    /* Crear la memoria compartida. */
    descriptor = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0600);
    if (descriptor == -1) {
        perror("shm_open");
        return -1;
    }

    /* Definir el tamaño de la memoria compartida. */
    if (ftruncate(descriptor, sizeof(struct BufferEstacionamiento)) == -1) {
        perror("ftruncate");
        close(descriptor);
        return -1;
    }

    /* Mapear la memoria compartida en el espacio de direcciones. */
    buffer = mmap(NULL, sizeof(struct BufferEstacionamiento),
                  PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        close(descriptor);
        return -1;
    }

    /* El descriptor ya no es necesario después de mmap. */
    close(descriptor);

    /* El productor inicializa los índices del buffer circular. */
    buffer->cabeza = 0;
    buffer->cola = 0;

    /* Crear los semáforos del patrón productor-consumidor. */
    sem_vacios = sem_open(SEM_VACIOS_NOMBRE, O_CREAT,
                          0600, CAPACIDAD_BUFFER);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open vacios");
        return -1;
    }

    sem_llenos = sem_open(SEM_LLENOS_NOMBRE, O_CREAT, 0600, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open llenos");
        return -1;
    }

    sem_mutex = sem_open(SEM_MUTEX_NOMBRE, O_CREAT, 0600, 1);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        return -1;
    }

    return 0;
}

static void liberar_recursos(void)
{
    if (buffer != NULL && buffer != MAP_FAILED) {
        munmap(buffer, sizeof(struct BufferEstacionamiento));
    }

    if (sem_vacios != NULL && sem_vacios != SEM_FAILED) {
        sem_close(sem_vacios);
    }

    if (sem_llenos != NULL && sem_llenos != SEM_FAILED) {
        sem_close(sem_llenos);
    }

    if (sem_mutex != NULL && sem_mutex != SEM_FAILED) {
        sem_close(sem_mutex);
    }
}

int main(int argc, char *argv[])
{
    struct Vehiculo vehiculo;
    int cantidad;
    int i;

    if (argc > 1) {
        cantidad = atoi(argv[1]);
        if (cantidad <= 0) {
            cantidad = CANTIDAD_DEFAULT;
        }
    } else {
        cantidad = CANTIDAD_DEFAULT;
    }

    srand(time(NULL));

    printf("[Entrada] Generando %d vehiculos.\n", cantidad);

    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Entrada] Error inicializando recursos.\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < cantidad; i++) {
        preparar_vehiculo(&vehiculo, i + 1);

        /* Esperar a que exista un lugar libre. */
        sem_wait(sem_vacios);

        /* Proteger la escritura del vehículo y el avance de cabeza. */
        sem_wait(sem_mutex);

        ingresar_vehiculo(buffer, &vehiculo);

        sem_post(sem_mutex);

        /* Avisar que hay un vehículo disponible para el monitor. */
        sem_post(sem_llenos);

        printf("[Entrada] Ticket %d | Patente %s\n",
               vehiculo.ticket, vehiculo.patente);
    }

    printf("[Entrada] Finalizo la generacion.\n");

    liberar_recursos();

    return EXIT_SUCCESS;
}
