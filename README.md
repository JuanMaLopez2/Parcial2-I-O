# Instrucciones de Compilación y Ejecución

```
# Compilar
make clean
make

# Ver ayuda
./smart_backup --help

# Realizar respaldo simple
./smart_backup -b archivo.txt backup.txt

# Comparar rendimiento
./smart_backup -c archivo.txt

# Ejecutar benchmark completo
./smart_backup --benchmark

# Ejecutar todas las pruebas automáticas
make test-all
```
# Estructura de Archivos Entregados
├── smart_copy.h          # Definiciones y prototipos
├── backup_engine.c       # Lógica central de copia
├── main.c                # Interfaz y benchmarking
├── Makefile              # Automatización
└── reporte.pdf           # Este documento
