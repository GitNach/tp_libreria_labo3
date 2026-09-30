CC      = gcc
CFLAGS  = -std=c89 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror
LDLIBS  = -lpthread -lrt

TARGETS = entrada monitor

.PHONY: all clean stop

all: $(TARGETS)

entrada: entrada.o estacionamiento.o
	$(CC) $(CFLAGS) -o entrada entrada.o estacionamiento.o $(LDLIBS)

monitor: monitor.o estacionamiento.o
	$(CC) $(CFLAGS) -o monitor monitor.o estacionamiento.o $(LDLIBS)

entrada.o: entrada.c estacionamiento.h
	$(CC) $(CFLAGS) -c entrada.c

monitor.o: monitor.c estacionamiento.h
	$(CC) $(CFLAGS) -c monitor.c

estacionamiento.o: estacionamiento.c estacionamiento.h
	$(CC) $(CFLAGS) -c estacionamiento.c

stop:
	rm -f /dev/shm/estacionamiento_buffer
	rm -f /dev/shm/sem.estacionamiento_vacios
	rm -f /dev/shm/sem.estacionamiento_llenos
	rm -f /dev/shm/sem.estacionamiento_mutex

clean: stop
	rm -f *.o $(TARGETS)
