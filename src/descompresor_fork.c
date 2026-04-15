#include "huffman.h"
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_PROCESOS 4

/* Esta función la ejecuta cada proceso hijo.
   Se encarga de abrir su archivo temporal específico y descomprimirlo
   en la carpeta de destino del fork.
*/
void ejecutar_descompresion_hilo(int id_hijo) {
    char nombre_entrada[50];
    // Cada hijo busca su pedazo correspondiente (ej: temp_hijo_0.bin)
    sprintf(nombre_entrada, "temp_hijo_%d.bin", id_hijo);
    
    FILE *entrada = fopen(nombre_entrada, "rb");
    if (!entrada) exit(0); // Si no encuentra el archivo, el hijo simplemente termina

    // Llama al motor de Huffman para procesar el archivo hasta que no quede nada
    while (descomprimir_archivo_serial(entrada, "descomprimido_fork"));

    fclose(entrada);
    
    // Una vez procesado, borra el temporal para no dejar basura en el sistema
    remove(nombre_entrada); 
    exit(0); 
}

int main() {
    struct timespec inicio, fin;
    const char *nombre_archivo = "comprimido_fork.bin";
    const char *carpeta_destino = "descomprimido_fork";
    long tamano = 0; 

    // --- Paso 1: Validar que el archivo comprimido exista y tenga datos ---
    FILE *entrada = fopen(nombre_archivo, "rb");
    if (!entrada) {
        fprintf(stderr, "\n[!] ERROR: No se encontró '%s'.\n", nombre_archivo);
        return 1;
    }

    // Mide el tamaño del archivo para mostrarlo en el mensaje inicial
    fseek(entrada, 0, SEEK_END);
    tamano = ftell(entrada); 
    if (tamano <= 0) {
        fprintf(stderr, "\n[!] ERROR: El archivo '%s' está vacío.\n", nombre_archivo);
        fclose(entrada);
        return 1;
    }
    fclose(entrada); 

    // --- Paso 2: Crear la carpeta de salida si no existe ---
    char comando[100];
    sprintf(comando, "mkdir -p %s", carpeta_destino);
    system(comando); 

    printf("Iniciando descompresión paralela (Fork) con %d procesos (%ld bytes)...\n", NUM_PROCESOS, tamano);
    
    // Inicia el cronómetro justo antes de crear los procesos
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // --- Paso 3: Lanzar los procesos hijos ---
    for (int i = 0; i < NUM_PROCESOS; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Bloque exclusivo del hijo
            ejecutar_descompresion_hilo(i);
            exit(0); 
        }
    }

    // --- Paso 4: Esperar a que todos los procesos terminen ---
    // El padre se queda aquí hasta que los 4 hijos confirmen su salida
    for (int i = 0; i < NUM_PROCESOS; i++) wait(NULL);

    // Captura el tiempo final tras la sincronización de todos los procesos
    clock_gettime(CLOCK_MONOTONIC, &fin);
    double tiempo = (fin.tv_sec - inicio.tv_sec) * 1000.0 + (fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
    
    printf("Descompresión paralela finalizada con éxito.\n");
    printf("Tiempo total (Fork): %.3f ms\n", tiempo);

    return 0;
}
