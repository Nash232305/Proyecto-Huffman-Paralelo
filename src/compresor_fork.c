#include "huffman.h"
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_PROCESOS 4

/* Esta función la ejecuta cada proceso hijo. 
   Su trabajo es comprimir el grupo de archivos que le toca y guardarlo 
   en un archivo temporal propio para no chocar con los demás.
*/
void procesar_lote(char archivos[][1024], int inicio, int fin, int id_hijo) {
    char nombre_salida[50];
    // Se crea un nombre único para el archivo temporal (ej: temp_hijo_0.bin)
    sprintf(nombre_salida, "temp_hijo_%d.bin", id_hijo);
    FILE *destino = fopen(nombre_salida, "wb");
    
    if (destino == NULL) return;

    // Comprime uno por uno los archivos asignados a este proceso
    for (int i = inicio; i < fin; i++) {
        comprimir_archivo_serial(archivos[i], destino);
    }
    fclose(destino);
}

int main() {
    struct timespec inicio, fin;
    const char *dir_path = "datos/Top100_Gutenberg/";
    char lista_archivos[200][1024];
    int total_archivos = 0;

    // --- Paso 1: Leer los nombres de los archivos ---
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Error: No se pudo abrir la carpeta de los libros");
        return 1;
    }

    struct dirent *ent;
    // Se guardan los nombres en una lista, ignorando archivos ocultos
    while ((ent = readdir(dir)) != NULL && total_archivos < 200) {
        if (ent->d_name[0] != '.') {
            sprintf(lista_archivos[total_archivos++], "%s%s", dir_path, ent->d_name);
        }
    }
    closedir(dir);

    printf("Iniciando compresión paralela (Fork) con %d procesos...\n", NUM_PROCESOS);
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // --- Paso 2: Crear los procesos con fork() ---
    int archivos_por_proceso = total_archivos / NUM_PROCESOS;
    for (int i = 0; i < NUM_PROCESOS; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Error al crear proceso hijo");
            return 1;
        }

        if (pid == 0) { // Bloque del proceso hijo
            // Define dónde empieza y termina su grupo de archivos
            int fin_lote = (i == NUM_PROCESOS - 1) ? total_archivos : (i + 1) * archivos_por_proceso;
            
            procesar_lote(lista_archivos, i * archivos_por_proceso, fin_lote, i);
            
            // El hijo termina su tarea y sale para no seguir ejecutando el main
            exit(0);
        }
    }

    // --- Paso 3: Esperar a que todos terminen ---
    // El proceso padre espera a que los 4 hijos cierren sus archivos temporales
    for (int i = 0; i < NUM_PROCESOS; i++) wait(NULL);

    // --- Paso 4: Unir todo en el archivo final ---
    FILE *final = fopen("comprimido_fork.bin", "wb");
    for (int i = 0; i < NUM_PROCESOS; i++) {
        char nombre_temp[50];
        sprintf(nombre_temp, "temp_hijo_%d.bin", i);
        FILE *temp = fopen(nombre_temp, "rb");
        
        if (temp) {
            unsigned char buffer[8192];
            size_t n;
            // Copia el contenido del temporal al archivo final usando un buffer
            while ((n = fread(buffer, 1, sizeof(buffer), temp)) > 0) {
                fwrite(buffer, 1, n, final);
            }
            fclose(temp);
            // Borra el archivo temporal para dejar la carpeta limpia
            remove(nombre_temp);
        }
    }
    fclose(final);

    // --- Paso 5: Cálculo de tiempo y cierre ---
    clock_gettime(CLOCK_MONOTONIC, &fin);
    double tiempo = (fin.tv_sec - inicio.tv_sec) * 1000.0 + (fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
    
    printf("Finalizado. Archivo: comprimido_fork.bin\n");
    printf("Tiempo total (Fork): %.3f ms\n", tiempo);

    return 0;
}
