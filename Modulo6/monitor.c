#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>

#include "estacionamiento.h"

/* Cantidad por defecto de vehiculos a retirar */
#define CANTIDAD_DEFAULT 20

static struct BufferEstacionamiento *buffer = NULL;
static sem_t *sem_vacios = NULL;
static sem_t *sem_llenos = NULL;
static sem_t *sem_mutex = NULL;

static int inicializar_recursos(void);
static void liberar_recursos(void);

static int inicializar_recursos(void){
    int descriptor;
    int intentos;

    intentos = 0;
    descriptor = -1;
    while (intentos < 10) {
        descriptor = shm_open(SHM_NOMBRE, O_RDWR, 0);
        if (descriptor != -1) {
            break;
        }
        if (errno == ENOENT) {
            /* La entrada todavia no arranco, esperamos un poco. */
            printf("[Monitor] Esperando a la entrada...\n");
            sleep(1);
            intentos++;
        } else {
            perror("shm_open");
            return -1;
        }
    }

    if (descriptor == -1) {
        fprintf(stderr, "[Monitor] La entrada no esta corriendo.\n");
        return -1;
    }

    buffer = mmap(NULL, sizeof(struct BufferEstacionamiento), PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        close(descriptor);
        return -1;
    }

    close(descriptor);

    sem_vacios = sem_open(SEM_VACIOS_NOMBRE, 0);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open vacios");
        return -1;
    }

    sem_llenos = sem_open(SEM_LLENOS_NOMBRE, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open llenos");
        return -1;
    }

    sem_mutex = sem_open(SEM_MUTEX_NOMBRE, 0);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        return -1;
    }

    return 0;
}


static void liberar_recursos(void){
    if (buffer != NULL && buffer != MAP_FAILED) {
        munmap(buffer, sizeof(struct BufferEstacionamiento));
    }

    if (sem_vacios != NULL && sem_vacios != SEM_FAILED) {
        sem_close(sem_vacios);
        sem_unlink(SEM_VACIOS_NOMBRE);
    }

    if (sem_llenos != NULL && sem_llenos != SEM_FAILED) {
        sem_close(sem_llenos);
        sem_unlink(SEM_LLENOS_NOMBRE);
    }

    if (sem_mutex != NULL && sem_mutex != SEM_FAILED) {
        sem_close(sem_mutex);
        sem_unlink(SEM_MUTEX_NOMBRE);
    }

    shm_unlink(SHM_NOMBRE);
}

int main(int argc, char *argv[]){
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

    printf("[Monitor] Iniciando...\n");
    printf("[Monitor] Esperando %d vehiculos.\n\n", cantidad);

    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Monitor] Error inicializando recursos.\n");
        liberar_recursos();
        return EXIT_FAILURE;
    }

    printf("[Monitor] Conectado a la entrada. Registrando salidas...\n\n");

    for (i = 0; i < cantidad; i++) {
        if (sem_wait(sem_llenos) == -1) {
            perror("sem_wait llenos");
            liberar_recursos();
            return EXIT_FAILURE;
        }

        if (sem_wait(sem_mutex) == -1) {
            perror("sem_wait mutex");
            liberar_recursos();
            return EXIT_FAILURE;
        }

        retirar_vehiculo(buffer, &vehiculo);

        sem_post(sem_mutex);

        sem_post(sem_vacios);

        printf("[Monitor] #%d | Ticket %d | Patente %s\n", i + 1, vehiculo.ticket, vehiculo.patente);
    }

    printf("\n[Monitor] Terminado. Se registraron %d salidas.\n", cantidad);

    liberar_recursos();

    printf("[Monitor] Recursos eliminados del sistema.\n");

    return EXIT_SUCCESS;
}
