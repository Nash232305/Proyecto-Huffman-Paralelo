#include "huffman.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

/* --- FUNCIÓN DE COMPRESIÓN ---
   Esta función toma un archivo de texto y lo convierte en un flujo de bits
   dentro del archivo binario de destino.
*/
void comprimir_archivo_serial(const char *ruta_entrada, FILE *destino) {
    unsigned long frecuencias[256] = {0};
    FILE *f_entrada = fopen(ruta_entrada, "rb");
    if (!f_entrada) return;

    // 1. Contamos qué tanto se repite cada carácter para armar el árbol
    int c;
    unsigned long total_caracteres = 0;
    while ((c = fgetc(f_entrada)) != EOF) {
        frecuencias[(unsigned char)c]++;
        total_caracteres++;
    }
    rewind(f_entrada); // Regresamos al inicio para la lectura real

    // 2. Construimos el árbol y la tabla de códigos (ej: 'A' -> "101")
    Nodo *raiz = construir_arbol(frecuencias);
    TablaCodigos tabla_final[256]; 
    memset(tabla_final, 0, sizeof(tabla_final));
    
    char codigo_aux[256];
    generar_codigos(raiz, codigo_aux, 0, tabla_final);

    // 3. Escribimos los Metadatos (La "receta" para que el descompresor entienda el archivo)
    uint16_t nombre_len = (uint16_t)strlen(ruta_entrada);
    fwrite(&nombre_len, sizeof(uint16_t), 1, destino); // Guardamos cuánto mide el nombre
    fwrite(ruta_entrada, sizeof(char), nombre_len, destino); // Guardamos el nombre del archivo
    fwrite(frecuencias, sizeof(unsigned long), 256, destino); // Guardamos las frecuencias
    fwrite(&total_caracteres, sizeof(unsigned long), 1, destino); // Guardamos el conteo total

    // 4. Escribimos los datos reales ya convertidos a códigos binarios
    unsigned char bit_buffer = 0;
    int bit_count = 0;
    while ((c = fgetc(f_entrada)) != EOF) {
        char *bit_str = tabla_final[(unsigned char)c].codigo;
        for (int i = 0; bit_str[i] != '\0'; i++) {
            // Empaquetamos bit por bit
            escribir_bit(destino, bit_str[i] == '1', &bit_buffer, &bit_count);
        }
    }
    flush_bits(destino, &bit_buffer, &bit_count); // Vaciamos el último byte si quedó incompleto

    fclose(f_entrada);
    liberar_arbol(raiz);
}

/* --- FUNCIÓN DE DESCOMPRESIÓN ---
   Lee el archivo binario y, usando la tabla de frecuencias guardada, 
   reconstruye el texto original.
*/
int descomprimir_archivo_serial(FILE *entrada, const char *nombre_carpeta_base) {
    // 1. Leemos los metadatos para saber qué estamos descomprimiendo
    uint16_t nombre_len;
    if (fread(&nombre_len, sizeof(uint16_t), 1, entrada) != 1) return 0;

    char nombre_archivo[1024];
    fread(nombre_archivo, sizeof(char), nombre_len, entrada);
    nombre_archivo[nombre_len] = '\0';

    unsigned long frecuencias[256];
    fread(frecuencias, sizeof(unsigned long), 256, entrada);

    unsigned long total_caracteres;
    fread(&total_caracteres, sizeof(unsigned long), 1, entrada);

    // 2. Reconstruimos el mismo árbol que se usó al comprimir
    Nodo *raiz = construir_arbol(frecuencias);
    
    // Preparamos la ruta de salida (ej: descomprimido_serial/datos/Top100/archivo.txt)
    char ruta_salida[1200];
    sprintf(ruta_salida, "%s/%s", nombre_carpeta_base, nombre_archivo);
    
    // Creamos las subcarpetas necesarias para que fopen no falle
    char carpeta_solo[1200];
    strcpy(carpeta_solo, ruta_salida);
    char *ultimo_slash = strrchr(carpeta_solo, '/');
    if (ultimo_slash) {
        *ultimo_slash = '\0';
        crear_directorio(carpeta_solo);
    }

    FILE *f_salida = fopen(ruta_salida, "wb");
    if (!f_salida) {
        liberar_arbol(raiz);
        return 1;
    }

    // 3. Leemos los bits y navegamos por el árbol para recuperar los caracteres
    Nodo *actual = raiz;
    unsigned long caracteres_escritos = 0;
    unsigned char byte;
    
    while (caracteres_escritos < total_caracteres && fread(&byte, 1, 1, entrada) == 1) {
        for (int i = 7; i >= 0 && caracteres_escritos < total_caracteres; i--) {
            int bit = (byte >> i) & 1;
            // 0 va a la izquierda, 1 va a la derecha
            actual = bit ? actual->der : actual->izq;
            if (!actual) break;

            // Si llegamos a una hoja, encontramos un carácter original
            if (!actual->izq && !actual->der) {
                fputc(actual->caracter, f_salida);
                caracteres_escritos++;
                actual = raiz; // Regresamos a la cima del árbol para el siguiente bit
            }
        }
    }

    fclose(f_salida);
    liberar_arbol(raiz);
    return 1; 
}

/* Función utilitaria para crear carpetas una dentro de otra (recursivo)
   evitando errores de "no existe el directorio".
*/
void crear_directorio(const char *ruta) {
    char tmp[1024];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp), "%s", ruta);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
}
