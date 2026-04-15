#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Estructura para los nodos del árbol de Huffman.
   Guarda el carácter, cuántas veces aparece (frecuencia) 
   y las direcciones de sus hijos izquierdo y derecho.
*/
typedef struct Nodo {
    unsigned char caracter;
    unsigned long frecuencia;
    struct Nodo *izq, *der;
} Nodo;

/* Estructura para la tabla de traducción.
   Aquí guardamos el código binario (ej: "101") que le corresponde 
   a cada uno de los 256 posibles caracteres.
*/
typedef struct {
    char codigo[256]; 
} TablaCodigos;

/* Estructura para la cola de prioridad (Min-Heap).
   Nos permite organizar los nodos para extraer siempre el de menor 
   frecuencia de forma eficiente.
*/
typedef struct {
    int size;
    int capacidad;
    Nodo **array;
} ColaPrioridad;

/* Estructura para pasar información a los hilos.
   Contiene el ID del hilo y el nombre del archivo temporal 
   con el que debe trabajar.
*/
typedef struct {
    int id_hijo;
    char nombre_archivo_temp[50];
} DatosHilo;

// --- Funciones para manejar el Árbol y la Cola de Prioridad ---
Nodo* crear_nodo(unsigned char c, unsigned long freq);
ColaPrioridad* crear_cola(int capacidad);
void insertar_cola(ColaPrioridad* cola, Nodo* nodo);
Nodo* extraer_minimo(ColaPrioridad* cola);
Nodo* construir_arbol(unsigned long frecuencias[]);
void generar_codigos(Nodo* raiz, char* actual, int nivel, TablaCodigos* tabla);
void liberar_arbol(Nodo* raiz);

// --- Funciones para la gestión de Bits y Archivos ---
// Estas funciones empaquetan bits en bytes para ahorrar espacio real en disco
void escribir_bit(FILE *destino, int bit, unsigned char *buffer, int *contador_bits);
void flush_bits(FILE *destino, unsigned char *buffer, int *contador_bits);

// Motores principales de compresión y descompresión
void comprimir_archivo_serial(const char *ruta_entrada, FILE *destino);
int descomprimir_archivo_serial(FILE *entrada, const char *nombre_carpeta_base);

// Función utilitaria para organizar las carpetas de salida
void crear_directorio(const char *ruta);

#endif
