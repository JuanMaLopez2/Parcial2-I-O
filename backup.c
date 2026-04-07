#include "smart_copy.h"
#include <limits.h>  // Para PATH_MAX
#include <sys/time.h> // Para struct timeval

// Definir PATH_MAX si no está definido
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * sys_smart_copy - Función principal que simula una llamada al sistema
 * Esta función actúa como la interfaz "kernel" para realizar copias optimizadas
 * 
 * @src: Ruta del archivo/directorio origen
 * @dest: Ruta del destino
 * @flags: Flags de comportamiento (recursivo, preservar permisos, etc.)
 * 
 * Return: 0 en éxito, -1 en error
 */
int sys_smart_copy(const char *src, const char *dest, int flags) {
    struct stat st;
    
    // Validación de parámetros (comportamiento de kernel)
    if (src == NULL || dest == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    // Verificar que el origen existe
    if (stat(src, &st) == -1) {
        // errno ya está establecido por stat
        return -1;
    }
    
    // Verificar permisos de lectura (simulación)
    if (access(src, R_OK) == -1) {
        errno = EACCES;
        return -1;
    }
    
    // Decidir si es archivo o directorio
    if (S_ISDIR(st.st_mode)) {
        // Copiar directorio
        if (!(flags & COPY_FLAG_RECURSIVE)) {
            errno = EISDIR;
            return -1;
        }
        return copy_directory_syscall(src, dest, flags);
    } else if (S_ISREG(st.st_mode)) {
        // Copiar archivo regular
        int preserve = (flags & COPY_FLAG_PRESERVE_PERMS) ? 1 : 0;
        return copy_file_syscall(src, dest, preserve);
    } else {
        // Otros tipos de archivo no soportados
        errno = EINVAL;
        return -1;
    }
}

/**
 * copy_file_syscall - Copia un archivo usando system calls (read/write)
 * Esta función simula el comportamiento optimizado del kernel con buffer de 4KB
 */
int copy_file_syscall(const char *src, const char *dest, int preserve_perms) {
    int fd_src, fd_dest;
    ssize_t bytes_read, bytes_written;
    char *buffer;
    struct stat st;
    mode_t mode = 0644;  // Permisos por defecto
    
    // Asignar buffer dinámicamente (4KB)
    buffer = (char *)malloc(BUFFER_SIZE);
    if (!buffer) {
        errno = ENOMEM;
        return -1;
    }
    
    // Obtener permisos del original si se solicita
    if (preserve_perms && stat(src, &st) == 0) {
        mode = st.st_mode;
    }
    
    // System call: open (lectura)
    fd_src = open(src, O_RDONLY);
    if (fd_src < 0) {
        free(buffer);
        return -1;
    }
    
    // System call: open (escritura, creación, truncado)
    fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd_dest < 0) {
        close(fd_src);
        free(buffer);
        return -1;
    }
    
    // System calls: read y write en ciclo (copia optimizada)
    while ((bytes_read = read(fd_src, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(fd_dest, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            // Error de escritura parcial
            close(fd_src);
            close(fd_dest);
            free(buffer);
            return -1;
        }
    }
    
    // Verificar error en lectura
    if (bytes_read < 0) {
        close(fd_src);
        close(fd_dest);
        free(buffer);
        return -1;
    }
    
    // System calls: close
    close(fd_src);
    close(fd_dest);
    free(buffer);
    
    return 0;
}

/**
 * copy_directory_syscall - Copia directorio recursivamente usando system calls
 */
int copy_directory_syscall(const char *src, const char *dest, int flags) {
    struct stat st;
    DIR *dir;
    struct dirent *entry;
    char next_src[PATH_MAX];
    char next_dest[PATH_MAX];
    
    // Verificar origen
    if (stat(src, &st) == -1) {
        return -1;
    }
    
    // Crear directorio destino (system call: mkdir)
    mode_t mode = (flags & COPY_FLAG_PRESERVE_PERMS) ? st.st_mode : 0755;
    if (mkdir(dest, mode) == -1 && errno != EEXIST) {
        return -1;
    }
    
    // Abrir directorio origen (opendir usa system calls internamente)
    dir = opendir(src);
    if (!dir) {
        return -1;
    }
    
    // Recorrer contenido
    while ((entry = readdir(dir)) != NULL) {
        // Ignorar . y ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Construir rutas
        snprintf(next_src, sizeof(next_src), "%s/%s", src, entry->d_name);
        snprintf(next_dest, sizeof(next_dest), "%s/%s", dest, entry->d_name);
        
        // Obtener información del elemento
        if (lstat(next_src, &st) == -1) {
            continue;  // Ignorar errores en elementos individuales
        }
        
        if (S_ISDIR(st.st_mode)) {
            // Subdirectorio: recursión
            copy_directory_syscall(next_src, next_dest, flags);
        } else if (S_ISREG(st.st_mode)) {
            // Archivo regular
            int preserve = (flags & COPY_FLAG_PRESERVE_PERMS) ? 1 : 0;
            copy_file_syscall(next_src, next_dest, preserve);
        }
        // Otros tipos se ignoran
    }
    
    closedir(dir);
    return 0;
}

/**
 * copy_file_stdio - Copia archivo usando funciones de librería (fopen/fread/fwrite)
 * Para comparación de rendimiento con la versión syscall
 */
int copy_file_stdio(const char *src, const char *dest) {
    FILE *f_src, *f_dest;
    char *buffer;
    size_t bytes_read, bytes_written;
    
    // Asignar buffer
    buffer = (char *)malloc(BUFFER_SIZE);
    if (!buffer) {
        errno = ENOMEM;
        return -1;
    }
    
    // Abrir archivo origen (stdio)
    f_src = fopen(src, "rb");
    if (!f_src) {
        free(buffer);
        return -1;
    }
    
    // Abrir archivo destino (stdio)
    f_dest = fopen(dest, "wb");
    if (!f_dest) {
        fclose(f_src);
        free(buffer);
        return -1;
    }
    
    // Copia usando fread/fwrite con buffer de 4KB
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, f_src)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, f_dest);
        if (bytes_written != bytes_read) {
            fclose(f_src);
            fclose(f_dest);
            free(buffer);
            return -1;
        }
    }
    
    // Verificar error en lectura
    if (ferror(f_src)) {
        fclose(f_src);
        fclose(f_dest);
        free(buffer);
        return -1;
    }
    
    fclose(f_src);
    fclose(f_dest);
    free(buffer);
    return 0;
}

/**
 * copy_directory_stdio - Copia directorio usando funciones stdio
 */
int copy_directory_stdio(const char *src, const char *dest) {
    struct stat st;
    DIR *dir;
    struct dirent *entry;
    char next_src[PATH_MAX];
    char next_dest[PATH_MAX];
    
    if (stat(src, &st) == -1) {
        return -1;
    }
    
    if (mkdir(dest, 0755) == -1 && errno != EEXIST) {
        return -1;
    }
    
    dir = opendir(src);
    if (!dir) {
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(next_src, sizeof(next_src), "%s/%s", src, entry->d_name);
        snprintf(next_dest, sizeof(next_dest), "%s/%s", dest, entry->d_name);
        
        if (stat(next_src, &st) == -1) {
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            copy_directory_stdio(next_src, next_dest);
        } else if (S_ISREG(st.st_mode)) {
            copy_file_stdio(next_src, next_dest);
        }
    }
    
    closedir(dir);
    return 0;
}