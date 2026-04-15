CC = gcc
CFLAGS = -Wall -Wextra -Isrc
LDFLAGS = -lpthread
SRC_DIR = src

ARBOL = $(SRC_DIR)/huffman.o
MOTOR = $(SRC_DIR)/motor.o

# Lista completa de ejecutables
all: compresor_serial descompresor_serial compresor_fork descompresor_fork compresor_threads descompresor_threads

# --- Reglas de Compresión ---
compresor_serial: $(ARBOL) $(MOTOR) $(SRC_DIR)/compresor_serial.o
	$(CC) $(CFLAGS) -o compresor_serial $(ARBOL) $(MOTOR) $(SRC_DIR)/compresor_serial.o

compresor_fork: $(ARBOL) $(MOTOR) $(SRC_DIR)/compresor_fork.o
	$(CC) $(CFLAGS) -o compresor_fork $(ARBOL) $(MOTOR) $(SRC_DIR)/compresor_fork.o

compresor_threads: $(ARBOL) $(MOTOR) $(SRC_DIR)/compresor_threads.o
	$(CC) $(CFLAGS) -o compresor_threads $(ARBOL) $(MOTOR) $(SRC_DIR)/compresor_threads.o $(LDFLAGS)

# --- Reglas de Descompresión ---
descompresor_serial: $(ARBOL) $(MOTOR) $(SRC_DIR)/descompresor_serial.o
	$(CC) $(CFLAGS) -o descompresor_serial $(ARBOL) $(MOTOR) $(SRC_DIR)/descompresor_serial.o

descompresor_fork: $(ARBOL) $(MOTOR) $(SRC_DIR)/descompresor_fork.o
	$(CC) $(CFLAGS) -o descompresor_fork $(ARBOL) $(MOTOR) $(SRC_DIR)/descompresor_fork.o

descompresor_threads: $(ARBOL) $(MOTOR) $(SRC_DIR)/descompresor_threads.o
	$(CC) $(CFLAGS) -o descompresor_threads $(ARBOL) $(MOTOR) $(SRC_DIR)/descompresor_threads.o $(LDFLAGS)

# --- Regla Genérica para Objetos ---
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Limpieza ---
clean:
	rm -f $(SRC_DIR)/*.o compresor_serial descompresor_serial compresor_fork descompresor_fork compresor_threads descompresor_threads *.bin
