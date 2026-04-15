#include "huffman.h"
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define NUM_THREADS 4

/* Esta estructura guarda los datos que necesita cada hilo:
   su ID y el nombre del archivo binario unificado que va a leer.
*/
typedef struct {
    int id_hilo;
    const char* nombre_entrada;
} DatosDescompresion;

// Esta función es la que ejecuta cada hilo de forma concurrente
void* ejecutar_descompresion_hilo(void* arg) {
    DatosDescompresion *datos = (DatosDescompresion*)arg;
    
    /* Es fundamental que cada hilo abra su propia instancia del archivo.
       Así cada uno tiene su propio puntero de lectura y no se estorban entre ellos.
    */
    FILE *entrada = fopen(datos->nombre_entrada, "rb");
    if (!entrada) {
        free(datos);
        pthread_exit(NULL);
    }

    /* El hilo procesa el archivo binario. Gracias a la estructura de Huffman,
       el motor sabe ir rescatando los libros uno por uno.
    */
    while (descomprimir_archivo_serial(entrada, "descomprimido_threads"));

    fclose(entrada);
    free(datos); // Se libera la memoria que se reservó para los datos del hilo
    pthread_exit(NULL);
}

int main() {
    struct timespec inicio, fin;
    pthread_t hilos[NUM_THREADS];
    const char *nombre_archivo = "comprimido_threads.bin";
    const char *carpeta_destino = "descomprimido_threads";

    // --- Paso 1: Validación del archivo binario ---
    FILE *prueba = fopen(nombre_archivo, "rb");
    if (!prueba) {
        fprintf(stderr, "\n[!] ERROR: No se encontró '%s'.\n", nombre_archivo);
        fprintf(stderr, "    Ejecute primero './compresor_threads'.\n\n");
        return 1;
    }
    
    // Se calcula el tamaño total para mostrarlo en pantalla
    fseek(prueba, 0, SEEK_END);
    long tamano_total = ftell(prueba);
    fclose(prueba);

    if (tamano_total <= 0) {
        fprintf(stderr, "[!] ERROR: El archivo '%s' está vacío.\n", nombre_archivo);
        return 1;
    }

    // --- Paso 2: Preparar la carpeta de salida ---
    char comando[100];
    sprintf(comando, "mkdir -p %s", carpeta_destino);
    system(comando); 

    printf("Iniciando descompresión por hilos (Pthreads) con %d hilos (%ld bytes)...\n", NUM_THREADS, tamano_total);
    
    // Se captura el tiempo justo antes de lanzar los hilos
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // --- Paso 3: Lanzar los hilos de descompresión ---
    for (int i = 0; i < NUM_THREADS; i++) {
        // Reservamos memoria para los datos de cada hilo de forma independiente
        DatosDescompresion *datos = malloc(sizeof(DatosDescompresion));
        datos->id_hilo = i;
        datos->nombre_entrada = nombre_archivo;
        
        pthread_create(&hilos[i], NULL, ejecutar_descompresion_hilo, (void*)datos);
    }

    // --- Paso 4: Sincronización (Esperar a los hilos) ---
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // --- Paso 5: Cálculo del tiempo final ---
    clock_gettime(CLOCK_MONOTONIC, &fin);
    double tiempo = (fin.tv_sec - inicio.tv_sec) * 1000.0 + (fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
    
    printf("\nDescompresión por hilos finalizada con éxito.\n");
    printf("Tiempo total (Threads): %.3f ms\n", tiempo);

    return 0;
}
