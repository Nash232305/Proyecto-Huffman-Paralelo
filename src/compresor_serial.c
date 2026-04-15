#include "huffman.h"
#include <time.h>
#include <dirent.h>

// Función para convertir la diferencia de tiempo a milisegundos
double calcular_milisegundos(struct timespec inicio, struct timespec fin) {
    return (double)(fin.tv_sec - inicio.tv_sec) * 1000.0 +
           (double)(fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
}

void comprimir_archivo_serial(const char *ruta_entrada, FILE *destino);

int main() {
    struct timespec inicio, fin;
    const char *dir_path = "datos/Top100_Gutenberg/";
    
    // Se crea el archivo binario donde se guardarán todos los libros comprimidos
    FILE *binario_salida = fopen("comprimido_serial.bin", "wb");

    if (!binario_salida) {
        perror("Error al crear el archivo binario");
        return 1;
    }

    printf("Procesando libros en serie...\n");
    
    // Se captura el tiempo justo antes de empezar el proceso
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Se abre la carpeta que contiene los 100 libros
    DIR *dir = opendir(dir_path);
    struct dirent *ent;
    
    if (dir) {
        // Se recorre la carpeta archivo por archivo
        while ((ent = readdir(dir)) != NULL) {
            // Se ignoran archivos ocultos o directorios del sistema (.)
            if (ent->d_name[0] != '.') {
                char ruta_completa[1024];
                // Se construye la ruta completa (ej: datos/Top100_Gutenberg/libro1.txt)
                sprintf(ruta_completa, "%s%s", dir_path, ent->d_name);
                
                // Se comprime el archivo y se escribe directamente en el binario único
                comprimir_archivo_serial(ruta_completa, binario_salida);
            }
        }
        closedir(dir); // Se cierra el directorio al terminar
    }
    
    // Se toma el tiempo final después de procesar todos los archivos
    clock_gettime(CLOCK_MONOTONIC, &fin);
    fclose(binario_salida);

    printf("Finalizado. Archivo generado: comprimido_serial.bin\n");
    printf("Tiempo total: %.3f ms\n", calcular_milisegundos(inicio, fin));

    return 0;
}
