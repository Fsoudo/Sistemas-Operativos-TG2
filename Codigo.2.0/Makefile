CC = gcc
CFLAGS = -Wall -g
TARGET = ja

# Regra principal (default)
all: $(TARGET)

# Cria o executável ligando os objetos
$(TARGET): main.o map.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o map.o

# Compila o main.c
main.o: main.c game.h map.h
	$(CC) $(CFLAGS) -c main.c

# Compila o map.c
map.o: map.c map.h game.h
	$(CC) $(CFLAGS) -c map.c

# Limpeza
clean:
	rm -f *.o $(TARGET)
