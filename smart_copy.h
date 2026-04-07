#ifndef SMART_COPY_H
#define SMART_COPY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

#define BUFFER_SIZE 4096  // Tamaño de página (4KB)

// Flags para opciones de copia
#define COPY_FLAG_RECURSIVE 0x01
#define COPY_FLAG_PRESERVE_PERMS 0x02
#define COPY_FLAG_VERBOSE 0x04

// Prototipos de funciones (capa kernel simulada)
int sys_smart_copy(const char *src, const char *dest, int flags);
int copy_file_syscall(const char *src, const char *dest, int preserve_perms);
int copy_directory_syscall(const char *src, const char *dest, int flags);

// Prototipos de funciones (capa usuario - stdio)
int copy_file_stdio(const char *src, const char *dest);
int copy_directory_stdio(const char *src, const char *dest);

// Funciones de benchmarking
typedef struct {
    double syscall_time;
    double stdio_time;
    size_t file_size;
    const char *filename;
} benchmark_result;

benchmark_result benchmark_copy(const char *src, const char *dest_syscall, const char *dest_stdio);

// Funciones auxiliares
void print_help(const char *prog_name);
int ensure_directory_exists(const char *path);

#endif // SMART_COPY_H