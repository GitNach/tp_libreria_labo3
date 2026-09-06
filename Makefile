CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror

catalogo_lib.o: catalogo_lib.c catalogo_lib.h
	$(CC) $(CFLAGS) -c catalogo_lib.c

clean:
	rm -f *.o
