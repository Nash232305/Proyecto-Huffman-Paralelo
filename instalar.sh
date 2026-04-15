#!/bin/bash

# Este script automatiza la preparación del proyecto Huffman Paralelo
# para el entorno del curso de Sistemas Operativos.

echo "===================================================="
echo "    Iniciando Instalación del Proyecto Huffman"
echo "===================================================="

# 1. Instalación de dependencias de C
# build-essential incluye gcc, make y librerías base de C
echo "[1/2] Instalando dependencias del sistema..."
sudo apt-get update
sudo apt-get install -y build-essential gcc

# 2. Compilación del proyecto
# Usamos el Makefile para generar los 6 ejecutables (serial, fork, threads)
echo "[2/2] Compilando código fuente..."
if [ -f "Makefile" ]; then
    make clean
    make
else
    echo "Error: No se encontró el Makefile en la raíz."
    exit 1
fi

echo "===================================================="
echo "    ¡Instalación exitosa!"
echo "    Uso sugerido:"
echo "    1. ./compresor_serial   (Genera el .bin)"
echo "    2. ./descompresor_serial (Crea la carpeta y extrae)"

echo "Nota: Si desea eliminar los ejecutables creados, puede ejecutar 'make clean' en cualquier momento."
echo "===================================================="
