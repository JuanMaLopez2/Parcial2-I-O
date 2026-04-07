CC = gcc
CFLAGS = -Wall -Wextra -g -D_GNU_SOURCE
LDFLAGS = 
TARGET = smart_backup
OBJS = main.o backup.o

# Regla por defecto
all: $(TARGET)

# Construir el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compilar main.c
main.o: main.c smart_copy.h
	$(CC) $(CFLAGS) -c main.c

# Compilar backup.c
backup_engine.o: backup.c smart_copy.h
	$(CC) $(CFLAGS) -c backup.c

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJS)
	rm -rf test_dir test_dir_backup
	rm -f bench_*.txt bench_*.syscall bench_*.stdio
	rm -f test_file.txt test_backup.txt

# Ejecutar pruebas básicas
test: $(TARGET)
	@echo "\n=== EJECUTANDO PRUEBAS BÁSICAS ===\n"
	@echo "1. Probar ayuda:"
	./$(TARGET) --help
	@echo "\n2. Crear archivo de prueba:"
	echo "Hola mundo" > test_file.txt
	@echo "\n3. Backup con system calls:"
	./$(TARGET) -b test_file.txt test_backup.txt
	@echo "\n4. Comparar rendimiento:"
	./$(TARGET) -c test_file.txt
	@echo "\n5. Limpiar archivos de prueba:"
	rm -f test_file.txt test_backup.txt test_file.txt.syscall test_file.txt.stdio

# Ejecutar benchmark completo
benchmark: $(TARGET)
	./$(TARGET) --benchmark

# Ejecutar pruebas con directorios
test-dir: $(TARGET)
	@echo "\n=== PRUEBA CON DIRECTORIOS ===\n"
	mkdir -p test_dir/subdir
	echo "archivo1" > test_dir/file1.txt
	echo "archivo2" > test_dir/file2.txt
	echo "subarchivo" > test_dir/subdir/subfile.txt
	./$(TARGET) -b test_dir test_dir_backup
	@echo "\nContenido del backup:"
	ls -R test_dir_backup
	rm -rf test_dir test_dir_backup

# Ejecutar todas las pruebas
test-all: test test-dir benchmark

# Compilación rápida y ejecución
quick: clean all
	@echo "\n=== COMPILACIÓN EXITOSA ===\n"
	./$(TARGET) --help

.PHONY: all clean test benchmark test-dir test-all quick