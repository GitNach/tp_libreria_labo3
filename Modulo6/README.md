# Estacionamiento: productor-consumidor con memoria compartida

Trabajo practico del Modulo 6 de Laboratorio III.

Dos procesos independientes se coordinan a traves de un buffer circular en
memoria compartida POSIX y tres semaforos POSIX nombrados:

- `entrada <cantidad>` (productor): la barrera del estacionamiento. Genera
  `cantidad` autos, cada uno con un numero de ticket y una patente al azar, y
  los deposita en el buffer. Si no hay lugar, se bloquea hasta que haya.
- `monitor <cantidad>` (consumidor): registra las salidas. Retira `cantidad`
  autos del buffer en orden de llegada. Si no hay autos, se bloquea hasta que
  llegue alguno. Al terminar elimina los recursos IPC.

## Compilacion

```sh
make
```

Usa `gcc -std=c89 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror` y enlaza con
`-lpthread -lrt`.

## Uso

En dos terminales (el orden no importa, el monitor espera hasta 10 segundos a
que aparezca la entrada):

```sh
./entrada 20
./monitor 20
```

Si no se indica cantidad, o es invalida, se usan 20.

Para ver el back pressure, correr solo `./entrada 20`: genera 10 autos, el
buffer se llena y se queda esperando. Al arrancar `./monitor 20` en otra
terminal, la entrada continua a medida que se liberan lugares.

## Limpieza

```sh
make stop    # borra los recursos IPC huerfanos de /dev/shm (sin tocar binarios)
make clean   # make stop + borra los .o y los ejecutables
```

`make stop` es necesario si un programa se interrumpe con Ctrl+C: los
recursos quedan en `/dev/shm` y una corrida nueva arrancaria con semaforos
en un estado viejo.

## Archivos

- `estacionamiento.h`: nombres de los recursos IPC, constantes, estructuras
  compartidas y prototipos de la biblioteca.
- `estacionamiento.c`: biblioteca. Generacion de patentes y operaciones sobre
  el buffer circular (`ingresar_vehiculo`, `retirar_vehiculo`).
- `entrada.c`: productor.
- `monitor.c`: consumidor.
- `Makefile`: compilacion incremental y limpieza.

---

## Parte escrita

### Identificacion de los elementos

| Elemento | En este sistema |
| --- | --- |
| Productor | `entrada`: genera autos y los deposita |
| Consumidor | `monitor`: retira autos y registra su salida |
| Buffer | `struct BufferEstacionamiento`: arreglo circular de `CAPACIDAD_BUFFER` (10) `struct Vehiculo` mas los indices `cabeza` y `cola`, en memoria compartida |
| Seccion critica | La escritura del auto en `vehiculos[cabeza]` y el avance de `cabeza` (productor), y la lectura de `vehiculos[cola]` y el avance de `cola` (consumidor) |

### Memoria compartida

El productor crea el segmento con `shm_open(O_CREAT | O_RDWR)`, le da tamano
con `ftruncate` y lo mapea con `mmap(MAP_SHARED)`. El consumidor lo abre con
`shm_open(O_RDWR)` (sin `O_CREAT`: tiene que existir) y lo mapea igual. Como
ambos incluyen `estacionamiento.h`, el layout de la estructura es identico en
los dos procesos.

### Semaforos

Se usa el patron canonico de tres semaforos nombrados:

| Semaforo | Valor inicial | Significado |
| --- | --- | --- |
| `vacios` | `CAPACIDAD_BUFFER` (10) | Lugares libres. El productor espera en el. |
| `llenos` | 0 | Autos disponibles. El consumidor espera en el. |
| `mutex` | 1 | Exclusion mutua sobre el buffer y sus indices. |

Protocolo del productor:

```text
sem_wait(vacios)    esperar un lugar libre
sem_wait(mutex)     entrar a la seccion critica
  escribir en vehiculos[cabeza]; cabeza = (cabeza + 1) % CAPACIDAD_BUFFER
sem_post(mutex)     salir de la seccion critica
sem_post(llenos)    avisar que hay un auto mas
```

Protocolo del consumidor (simetrico):

```text
sem_wait(llenos)    esperar un auto disponible
sem_wait(mutex)     entrar a la seccion critica
  leer vehiculos[cola]; cola = (cola + 1) % CAPACIDAD_BUFFER
sem_post(mutex)     salir de la seccion critica
sem_post(vacios)    avisar que hay un lugar libre
```

El orden importa: se espera primero en `vacios`/`llenos` y despues en
`mutex`. Si se invirtiera, un proceso podria quedarse dormido dentro de la
seccion critica con el mutex tomado (por ejemplo, el productor con el buffer
lleno), y el otro no podria entrar nunca a liberar un lugar: deadlock.

### Por que hacen falta los tres

- `vacios` impide escribir sobre un auto que todavia no se retiro (overflow).
- `llenos` impide leer una posicion que todavia no se escribio (underflow).
- `mutex` impide que los dos procesos modifiquen a la vez el buffer y los
  indices. Con un buffer circular de un solo productor y un solo consumidor
  `cabeza` y `cola` las toca cada uno por separado, pero el mutex mantiene el
  protocolo correcto tambien si se agregan mas productores o consumidores.

### Back pressure

Como `vacios` arranca en `CAPACIDAD_BUFFER`, el productor puede depositar como
mucho 10 autos sin que nadie retire ninguno. El auto 11 se queda bloqueado en
`sem_wait(vacios)` y el productor no consume CPU mientras espera. Cada vez que
el consumidor retira un auto hace `sem_post(vacios)` y destraba al productor.
Es decir, el ritmo del productor queda limitado por el del consumidor: el
consumidor "empuja hacia atras" sin que haga falta ningun mensaje explicito ni
espera activa. Lo mismo ocurre al reves: con el buffer vacio, el consumidor se
duerme en `llenos` hasta que llega un auto.

### Buffer circular

`cabeza` es la posicion donde el productor escribe y `cola` la posicion de la
que el consumidor lee. Ambas avanzan con aritmetica modular
(`(i + 1) % CAPACIDAD_BUFFER`), asi que al llegar al final vuelven a 0. No hace
falta un contador de elementos porque la cantidad de autos en el buffer ya
esta representada por el semaforo `llenos` (y los lugares libres por
`vacios`). Los autos salen en el mismo orden en que entraron (FIFO).

### Liberacion de recursos IPC

- Cada proceso, al terminar, hace `munmap` y `sem_close` de lo que abrio.
- El productor **no** hace `unlink`: si lo hiciera y terminara antes de que el
  consumidor abriera los recursos, el consumidor no los encontraria o,
  peor, los perderia con autos todavia en el buffer.
- El consumidor es el ultimo en irse y por eso hace `sem_unlink` de los tres
  semaforos y `shm_unlink` del segmento. Los recursos de kernel se destruyen
  cuando el ultimo proceso que los tiene abiertos los cierra.
- Si alguno se interrumpe antes de terminar (Ctrl+C, error), los recursos
  quedan en `/dev/shm`; se eliminan con `make stop`.

### Chequeo de errores

Se verifica el retorno de `shm_open`, `ftruncate`, `mmap`, `sem_open` y
`sem_wait`, se informa con `perror` y se sale con `EXIT_FAILURE` liberando lo
que se haya abierto. El monitor, si la entrada todavia no arranco
(`shm_open` devuelve `ENOENT`), reintenta una vez por segundo hasta 10 veces
antes de rendirse.

### Limitaciones conocidas

- Si quedaron recursos de una corrida interrumpida, los semaforos se reabren
  con su valor viejo. Correr `make stop` antes de arrancar.
- El productor y el consumidor deben acordar la cantidad de autos: si el
  monitor pide mas de los que genera la entrada, se queda bloqueado esperando
  autos que nunca llegan.
- Hay una ventana minima entre que la entrada crea el segmento y crea los
  semaforos; si el monitor abre justo en ese instante fallara el `sem_open`.
  Basta con volver a ejecutarlo.
