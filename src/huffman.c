#include "huffman.h"

/* Crea un nuevo nodo en memoria para el árbol. 
   Es importante poner los punteros en NULL para que el árbol sepa 
   dónde terminan las "hojas".
*/
Nodo* crear_nodo(unsigned char c, unsigned long freq) {
    Nodo* nuevo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevo) {
        nuevo->caracter = c;
        nuevo->frecuencia = freq;
        nuevo->izq = nuevo->der = NULL;
    }
    return nuevo;
}

/* Inicializa la cola de prioridad. 
   Esta cola nos ayuda a tener siempre a mano los caracteres que menos se repiten.
*/
ColaPrioridad* crear_cola(int capacidad) {
    ColaPrioridad* cola = (ColaPrioridad*)malloc(sizeof(ColaPrioridad));
    cola->size = 0;
    cola->capacidad = capacidad;
    cola->array = (Nodo**)malloc(capacidad * sizeof(Nodo*));
    return cola;
}

/* Inserta un nodo en la cola y lo acomoda según su frecuencia. 
   Es un "Min-Heap": el que tiene menor frecuencia siempre sube a la cima.
*/
void insertar_cola(ColaPrioridad* cola, Nodo* nodo) {
    int i = cola->size++;
    while (i > 0 && nodo->frecuencia < cola->array[(i - 1) / 2]->frecuencia) {
        cola->array[i] = cola->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    cola->array[i] = nodo;
}

/* Saca el nodo con menor frecuencia de la cola. 
   Después de sacarlo, reacomoda toda la cola para que el siguiente 
   más pequeño quede en la posición cero.
*/
Nodo* extraer_minimo(ColaPrioridad* cola) {
    Nodo* min = cola->array[0];
    cola->array[0] = cola->array[--cola->size];
    int i = 0;
    while (i * 2 + 1 < cola->size) {
        int j = i * 2 + 1;
        if (j + 1 < cola->size && cola->array[j + 1]->frecuencia < cola->array[j]->frecuencia) {
            j++;
        }
        if (cola->array[i]->frecuencia <= cola->array[j]->frecuencia) break;
        Nodo* temp = cola->array[i];
        cola->array[i] = cola->array[j];
        cola->array[j] = temp;
        i = j;
    }
    return min;
}

/* Esta es la magia de Huffman: construye el árbol binario.
   Toma los dos nodos más pequeños, los une en un "padre" cuya frecuencia 
   es la suma de ambos, y repite hasta que solo queda un nodo (la raíz).
*/
Nodo* construir_arbol(unsigned long frecuencias[]) {
    ColaPrioridad* cola = crear_cola(256);
    // Metemos en la cola solo los caracteres que sí aparecen en el texto
    for (int i = 0; i < 256; i++) {
        if (frecuencias[i] > 0) {
            insertar_cola(cola, crear_nodo((unsigned char)i, frecuencias[i]));
        }
    }
    
    // Mientras haya más de un nodo, seguimos uniendo parejas
    while (cola->size > 1) {
        Nodo* izq = extraer_minimo(cola);
        Nodo* der = extraer_minimo(cola);
        // El caracter '$' indica que es un nodo interno, no una letra real
        Nodo* padre = crear_nodo('$', izq->frecuencia + der->frecuencia);
        padre->izq = izq;
        padre->der = der;
        insertar_cola(cola, padre);
    }
    
    Nodo* raiz = extraer_minimo(cola);
    free(cola->array);
    free(cola);
    return raiz;
}

/* Recorre el árbol de arriba hacia abajo para asignar ceros y unos.
   Si va a la izquierda pone un '0', si va a la derecha un '1'.
   Al llegar a una hoja, guarda ese código en la tabla.
*/
void generar_codigos(Nodo* raiz, char* actual, int nivel, TablaCodigos tabla[]) {
    if (!raiz) return;
    
    // Si no tiene hijos, es una hoja: guardamos el código acumulado
    if (!raiz->izq && !raiz->der) {
        actual[nivel] = '\0';
        strcpy(tabla[raiz->caracter].codigo, actual);
        return;
    }
    
    actual[nivel] = '0';
    generar_codigos(raiz->izq, actual, nivel + 1, tabla);
    
    actual[nivel] = '1';
    generar_codigos(raiz->der, actual, nivel + 1, tabla);
}

/* Función necesaria para limpiar la memoria cada vez que terminamos 
   con un archivo, evitando que el programa se coma la RAM del sistema.
*/
void liberar_arbol(Nodo* raiz) {
    if (!raiz) return;
    liberar_arbol(raiz->izq);
    liberar_arbol(raiz->der);
    free(raiz);
}

/* Como el sistema operativo escribe en bytes (8 bits), esta función 
   va juntando los bits de Huffman y solo los guarda cuando completa el byte.
*/
void escribir_bit(FILE *destino, int bit, unsigned char *buffer, int *contador_bits) {
    if (bit) *buffer |= (1 << (7 - *contador_bits));
    (*contador_bits)++;
    
    // Si ya completamos los 8 bits, lo mandamos al archivo
    if (*contador_bits == 8) {
        fputc(*buffer, destino);
        *buffer = 0;
        *contador_bits = 0;
    }
}

/* Al terminar la compresión, es probable que el último byte no esté lleno. 
   Esta función lo "empuja" al archivo para no perder los últimos datos.
*/
void flush_bits(FILE *destino, unsigned char *buffer, int *contador_bits) {
    if (*contador_bits > 0) {
        fputc(*buffer, destino);
        *buffer = 0;
        *contador_bits = 0;
    }
}
