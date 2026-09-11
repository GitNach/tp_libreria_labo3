CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror
TARGET = catalogo
OBJECTS = catalogo.o catalogo_lib.o
DATA = catalogo.txt

.PHONY: all clean

all: $(TARGET) $(DATA)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

catalogo.o: catalogo.c catalogo_lib.h
	$(CC) $(CFLAGS) -c catalogo.c

catalogo_lib.o: catalogo_lib.c catalogo_lib.h
	$(CC) $(CFLAGS) -c catalogo_lib.c

$(DATA):
	touch $(DATA)

clean:
	rm -f $(OBJECTS) $(TARGET) $(DATA)
