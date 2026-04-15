#include "huffman.h"
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

int main() {
    struct timespec inicio, fin;
    const char *nombre_archivo = "comprimido_serial.bin";
    const char *carpeta_destino = "descomprimido_serial";

    // --- Paso 1: Intentar abrir el archivo comprimido ---
    FILE *entrada = fopen(nombre_archivo, "rb");
    if (!entrada) {
        fprintf(stderr, "\n[!] ERROR: No se pudo abrir '%s'.\n", nombre_archivo);
        fprintf(stderr, "    Asegúrese de que el archivo exista antes de descomprimir.\n\n");
        return 1;
    }

    // --- Paso 2: Verificar que el archivo no esté vacío ---
    fseek(entrada, 0, SEEK_END);
    long tamano = ftell(entrada); // Obtiene el tamaño total en bytes
    
    if (tamano <= 0) {
        fprintf(stderr, "\n[!] ERROR: El archivo '%s' está vacío (0 bytes).\n", nombre_archivo);
        fclose(entrada);
        return 1;
    }
    
    // Regresa el puntero al inicio del archivo para empezar la lectura real
    rewind(entrada); 

    // Crea la carpeta donde se guardarán los libros recuperados
    system("mkdir -p descomprimido_serial");

    printf("Descomprimiendo archivos en modo serial (%ld bytes)...\n", tamano);
    
    // Captura el tiempo inicial
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // --- Paso 3: Motor de descompresión ---
    /* El bucle 'while' llama a la función de descompresión repetidamente.
       Cada llamada recupera un libro completo. La función devuelve 'false' 
       cuando llega al final del archivo binario.
    */
    while (descomprimir_archivo_serial(entrada, carpeta_destino));

    // Captura el tiempo final tras procesar todo el binario
    clock_gettime(CLOCK_MONOTONIC, &fin);

    fclose(entrada);

    // --- Paso 4: Mostrar resultados ---
    double tiempo = (fin.tv_sec - inicio.tv_sec) * 1000.0 + (fin.tv_nsec - inicio.tv_nsec) / 1000000.0;

    printf("Descompresión finalizada con éxito.\n");
    printf("Tiempo total (Serial): %.3f ms\n", tiempo);

    return 0;
}
