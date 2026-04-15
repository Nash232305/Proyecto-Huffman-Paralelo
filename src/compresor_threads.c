#include "huffman.h"
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#define NUM_THREADS 4

/* Esta estructura sirve para pasarle a cada hilo los archivos que debe procesar,
   su cantidad y su identificador para nombrar el archivo temporal.
*/
typedef struct {
    char archivos[512][1024];
    int cantidad;
    int id_hilo;
} DatosCarga;

// Función que ejecuta cada hilo individualmente
void* ejecutar_compresion_hilo(void* arg) {
    DatosCarga *carga = (DatosCarga*)arg;
    char nombre_salida[50];
    // Cada hilo crea su propio archivo temporal para guardar su progreso
    sprintf(nombre_salida, "temp_hijo_%d.bin", carga->id_hilo);

    if (carga->cantidad == 0) pthread_exit(NULL);

    FILE *destino = fopen(nombre_salida, "wb");
    if (!destino) pthread_exit(NULL);

    // El hilo comprime su lista de archivos uno tras otro
    for (int i = 0; i < carga->cantidad; i++) {
        comprimir_archivo_serial(carga->archivos[i], destino);
    }

    fclose(destino);
    pthread_exit(NULL); // Termina el hilo de forma segura
}

int main() {
    struct timespec inicio, fin;
    pthread_t hilos[NUM_THREADS];
    DatosCarga *cargas[NUM_THREADS];

    // --- Paso 1: Reservar memoria para los datos de cada hilo ---
    for(int i = 0; i < NUM_THREADS; i++) {
        cargas[i] = malloc(sizeof(DatosCarga));
        cargas[i]->cantidad = 0;
        cargas[i]->id_hilo = i;
    }

    // --- Paso 2: Repartir los archivos entre los hilos ---
    const char *ruta_libros = "./datos/Top100_Gutenberg";
    DIR *d = opendir(ruta_libros);
    if (!d) return 1;

    struct dirent *dir;
    int cuenta = 0;
    while ((dir = readdir(d)) != NULL) {
        // Solo tomamos archivos con extensión .txt
        if (strstr(dir->d_name, ".txt")) {
            // Usamos el operador residuo (%) para repartir los archivos como si fuera una baraja de cartas
            int hilo_dest = cuenta % NUM_THREADS;
            sprintf(cargas[hilo_dest]->archivos[cargas[hilo_dest]->cantidad], "%s/%s", ruta_libros, dir->d_name);
            cargas[hilo_dest]->cantidad++;
            cuenta++;
        }
    }
    closedir(d);

    printf("Iniciando compresión paralela (Threads) con %d hilos...\n", NUM_THREADS);
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // --- Paso 3: Lanzar los hilos (pthread_create) ---
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&hilos[i], NULL, ejecutar_compresion_hilo, (void*)cargas[i]);
    }

    // --- Paso 4: Esperar a que todos los hilos terminen (pthread_join) ---
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(hilos[i], NULL);
        free(cargas[i]); // Liberamos la memoria de la estructura una vez usada
    }

    // --- Paso 5: Unir los temporales en un solo archivo binario ---
    FILE *final = fopen("comprimido_threads.bin", "wb");
    for (int i = 0; i < NUM_THREADS; i++) {
        char nombre_temp[50];
        sprintf(nombre_temp, "temp_hijo_%d.bin", i);
        FILE *temp = fopen(nombre_temp, "rb");
        if (temp) {
            char buffer[1024];
            size_t bytes;
            // Pasamos los datos del temporal al archivo final
            while ((bytes = fread(buffer, 1, sizeof(buffer), temp)) > 0) {
                fwrite(buffer, 1, bytes, final);
            }
            fclose(temp);
            // remove(nombre_temp); // Se puede descomentar para borrar temporales automáticamente
        }
    }
    fclose(final);

    clock_gettime(CLOCK_MONOTONIC, &fin);
    double tiempo = (fin.tv_sec - inicio.tv_sec) * 1000.0 + (fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
    
    printf("Finalizado. Archivos temp_hijo_X.bin generados.\n");
    printf("Tiempo total (Threads): %.3f ms\n", tiempo);

    return 0;
}
