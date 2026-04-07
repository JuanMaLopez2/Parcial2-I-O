#include "smart_copy.h"
#include <sys/time.h>   // Para gettimeofday
#include <limits.h>     // Para PATH_MAX

// Definir PATH_MAX si no está definido
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * benchmark_copy - Compara rendimiento entre syscall y stdio
 * Crea archivos temporales para cada método y mide tiempos
 */
benchmark_result benchmark_copy(const char *src, const char *dest_syscall, const char *dest_stdio) {
    benchmark_result result;
    struct timeval start, end;
    struct stat st;
    
    // Inicializar resultado
    result.filename = src;
    result.syscall_time = 0;
    result.stdio_time = 0;
    result.file_size = 0;
    
    // Obtener tamaño del archivo
    if (stat(src, &st) == 0) {
        result.file_size = st.st_size;
    }
    
    // Benchmark con system calls
    gettimeofday(&start, NULL);
    if (copy_file_syscall(src, dest_syscall, 1) == 0) {
        gettimeofday(&end, NULL);
        result.syscall_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    }
    
    // Benchmark con stdio
    gettimeofday(&start, NULL);
    if (copy_file_stdio(src, dest_stdio) == 0) {
        gettimeofday(&end, NULL);
        result.stdio_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    }
    
    return result;
}

/**
 * print_benchmark_table - Imprime tabla comparativa de resultados
 */
void print_benchmark_table(benchmark_result results[], int count) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                      TABLA COMPARATIVA DE RENDIMIENTO                          ║\n");
    printf("╠══════════════════════════╦══════════════╦══════════════╦══════════════════════╣\n");
    printf("║ Archivo                  ║ Tamaño (KB)  ║ Syscall (s)  ║ Stdio (s)            ║\n");
    printf("╠══════════════════════════╬══════════════╬══════════════╬══════════════════════╣\n");
    
    for (int i = 0; i < count; i++) {
        double size_kb = results[i].file_size / 1024.0;
        double speedup = 0;
        const char *winner = "";
        
        if (results[i].syscall_time > 0 && results[i].stdio_time > 0) {
            if (results[i].syscall_time < results[i].stdio_time) {
                speedup = results[i].stdio_time / results[i].syscall_time;
                winner = "Syscall";
            } else {
                speedup = results[i].syscall_time / results[i].stdio_time;
                winner = "Stdio";
            }
        }
        
        printf("║ %-28s ║ %10.2f ║ %10.4f ║ %10.4f ║\n", 
               results[i].filename, size_kb, 
               results[i].syscall_time, results[i].stdio_time);
               
        if (speedup > 0) {
            printf("║ %-28s ║ %10s ║ %10s ║ [%s es %.2fx más rápido] ║\n",
                   "", "", "", winner, speedup);
        }
    }
    printf("╚══════════════════════════╩══════════════╩══════════════╩══════════════════════╝\n");
    printf("\n");
}

/**
 * print_analysis - Imprime análisis técnico de los resultados
 */
void print_analysis() {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    printf("                         ANÁLISIS TÉCNICO DE RENDIMIENTO                        \n");
    printf("═══════════════════════════════════════════════════════════════════════════════\n\n");
    
    printf("📊 ¿Por qué fread() es más eficiente para archivos pequeños?\n");
    printf("   • fread() implementa buffering en espacio de usuario (usualmente 4KB-8KB)\n");
    printf("   • Reduce el número de llamadas al sistema (context switches)\n");
    printf("   • Para archivos < buffer, fread() puede completarse con una sola syscall\n\n");
    
    printf("🔄 Context Switch - Explicación técnica:\n");
    printf("   • Cada llamada a read()/write() implica un cambio de contexto:\n");
    printf("     Usuario → Kernel → Usuario\n");
    printf("   • Costo aproximado: 1-10 microsegundos por switch\n");
    printf("   • Con buffer de 4KB, un archivo de 1MB requiere ~250 syscalls\n");
    printf("   • stdio reduce esto a ~125 syscalls (buffer interno + syscall buffer)\n\n");
    
    printf("⚡ Comparativa de capas:\n");
    printf("   ┌─────────────────────────────────────────────────────────────────┐\n");
    printf("   │ Capa Usuario (stdio)  │ fread() → buffer usuario → write()    │\n");
    printf("   │───────────────────────┼───────────────────────────────────────│\n");
    printf("   │ Capa Kernel (syscall) │ read() → buffer kernel → write()      │\n");
    printf("   └─────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("🎯 Conclusión:\n");
    printf("   • Archivos < 4KB: stdio es más eficiente (menos syscalls)\n");
    printf("   • Archivos grandes: La diferencia se reduce, pero stdio mantiene ventaja\n");
    printf("   • System calls ofrecen más control y son necesarias para operaciones\n");
    printf("     de bajo nivel (permisos, metadata, etc.)\n");
}

/**
 * run_comprehensive_benchmark - Ejecuta pruebas con diferentes tamaños de archivo
 */
void run_comprehensive_benchmark() {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    printf("                    INICIANDO PRUEBAS DE RENDIMIENTO COMPLETAS                 \n");
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    
    // Crear archivos de prueba con diferentes tamaños
    const char *test_files[] = {
        "bench_1KB.txt",    // 1 KB
        "bench_64KB.txt",   // 64 KB
        "bench_1MB.txt",    // 1 MB
        "bench_10MB.txt"    // 10 MB
    };
    
    const size_t sizes[] = {
        1024,           // 1 KB
        64 * 1024,      // 64 KB
        1024 * 1024,    // 1 MB
        10 * 1024 * 1024 // 10 MB
    };
    
    const int num_tests = 4;
    benchmark_result results[num_tests];
    
    // Generar archivos de prueba
    printf("\n📁 Generando archivos de prueba...\n");
    for (int i = 0; i < num_tests; i++) {
        FILE *f = fopen(test_files[i], "wb");
        if (f) {
            char *data = (char *)malloc(sizes[i]);
            if (data) {
                memset(data, 'X', sizes[i]);
                fwrite(data, 1, sizes[i], f);
                free(data);
            }
            fclose(f);
            printf("   ✓ Creado: %s (%.2f KB)\n", test_files[i], sizes[i] / 1024.0);
        }
    }
    
    // Ejecutar benchmarks
    printf("\n🔬 Ejecutando pruebas...\n");
    for (int i = 0; i < num_tests; i++) {
        char dest_syscall[PATH_MAX];
        char dest_stdio[PATH_MAX];
        snprintf(dest_syscall, sizeof(dest_syscall), "%s.syscall", test_files[i]);
        snprintf(dest_stdio, sizeof(dest_stdio), "%s.stdio", test_files[i]);
        
        printf("   Probando: %s...\n", test_files[i]);
        results[i] = benchmark_copy(test_files[i], dest_syscall, dest_stdio);
    }
    
    // Mostrar resultados
    print_benchmark_table(results, num_tests);
    print_analysis();
    
    // Limpiar archivos de prueba
    printf("🧹 Limpiando archivos de prueba...\n");
    for (int i = 0; i < num_tests; i++) {
        char dest_syscall[PATH_MAX];
        char dest_stdio[PATH_MAX];
        snprintf(dest_syscall, sizeof(dest_syscall), "%s.syscall", test_files[i]);
        snprintf(dest_stdio, sizeof(dest_stdio), "%s.stdio", test_files[i]);
        remove(test_files[i]);
        remove(dest_syscall);
        remove(dest_stdio);
    }
    printf("   ✓ Limpieza completada\n");
}

/**
 * print_help - Muestra ayuda del programa
 */
void print_help(const char *prog_name) {
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║           SMART BACKUP KERNEL-SPACE UTILITY                      ║\n");
    printf("║           Sistema de Respaldo con Análisis de Rendimiento        ║\n");
    printf("╚══════════════════════════════════════════════════════════════════╝\n\n");
    printf("Uso: %s [OPCIÓN] [ORIGEN] [DESTINO]\n\n", prog_name);
    printf("OPCIONES:\n");
    printf("  -h, --help              Muestra esta ayuda\n");
    printf("  -b, --backup ORIGEN DEST   Realiza respaldo (usa system calls)\n");
    printf("  -s, --stdio ORIGEN DEST    Realiza respaldo (usa stdio)\n");
    printf("  -c, --compare ORIGEN       Compara rendimiento de ambos métodos\n");
    printf("  --benchmark                Ejecuta pruebas completas de rendimiento\n\n");
    printf("EJEMPLOS:\n");
    printf("  %s -b archivo.txt backup.txt\n", prog_name);
    printf("  %s -c archivo.txt\n", prog_name);
    printf("  %s --benchmark\n\n", prog_name);
    printf("NOTA TÉCNICA:\n");
    printf("  La función sys_smart_copy() simula una llamada al sistema,\n");
    printf("  operando con buffer de página (4KB) para maximizar throughput.\n");
}

/**
 * ensure_directory_exists - Crea directorio si no existe
 */
int ensure_directory_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        return mkdir(path, 0755);
    }
    return 0;
}

/**
 * main - Punto de entrada del programa
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }
    
    // Opción de ayuda
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    
    // Opción de benchmark completo
    if (strcmp(argv[1], "--benchmark") == 0) {
        run_comprehensive_benchmark();
        return EXIT_SUCCESS;
    }
    
    // Opción de comparación directa
    if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--compare") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Error: Uso correcto: %s -c archivo_origen\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        const char *src = argv[2];
        struct stat st;
        
        if (stat(src, &st) == -1) {
            perror("Error: Archivo origen no encontrado");
            return EXIT_FAILURE;
        }
        
        // CORREGIDO: Usar S_ISREG en lugar de IS_ISREG
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "Error: Solo se pueden comparar archivos regulares\n");
            return EXIT_FAILURE;
        }
        
        char dest_syscall[PATH_MAX];
        char dest_stdio[PATH_MAX];
        snprintf(dest_syscall, sizeof(dest_syscall), "%s.syscall", src);
        snprintf(dest_stdio, sizeof(dest_stdio), "%s.stdio", src);
        
        printf("\n--- Comparando rendimiento para: %s (%.2f KB) ---\n", 
               src, st.st_size / 1024.0);
        
        // CORREGIDO: Asignar el resultado a una variable
        benchmark_result result = benchmark_copy(src, dest_syscall, dest_stdio);
        print_benchmark_table(&result, 1);
        
        // Limpiar archivos temporales
        remove(dest_syscall);
        remove(dest_stdio);
        
        return EXIT_SUCCESS;
    }
    
    // Opción de backup con system calls
    if (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--backup") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: Uso correcto: %s -b origen destino\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        const char *src = argv[2];
        const char *dest = argv[3];
        
        printf("--- Iniciando respaldo (System Calls) ---\n");
        printf("Origen: %s\n", src);
        printf("Destino: %s\n", dest);
        
        struct timeval start, end;
        gettimeofday(&start, NULL);
        
        int result = sys_smart_copy(src, dest, COPY_FLAG_RECURSIVE | COPY_FLAG_PRESERVE_PERMS);
        
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        
        if (result == 0) {
            printf("✓ Respaldo completado exitosamente\n");
            printf("⏱️  Tiempo de ejecución: %.4f segundos\n", elapsed);
        } else {
            perror("✗ Error durante el respaldo");
            return EXIT_FAILURE;
        }
        
        return EXIT_SUCCESS;
    }
    
    // Opción de backup con stdio
    if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--stdio") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: Uso correcto: %s -s origen destino\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        const char *src = argv[2];
        const char *dest = argv[3];
        struct stat st;
        
        printf("--- Iniciando respaldo (Stdio) ---\n");
        
        if (stat(src, &st) == -1) {
            perror("Error: Origen no encontrado");
            return EXIT_FAILURE;
        }
        
        struct timeval start, end;
        gettimeofday(&start, NULL);
        
        int result;
        if (S_ISDIR(st.st_mode)) {
            result = copy_directory_stdio(src, dest);
        } else {
            result = copy_file_stdio(src, dest);
        }
        
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        
        if (result == 0) {
            printf("✓ Respaldo completado exitosamente\n");
            printf("⏱️  Tiempo de ejecución: %.4f segundos\n", elapsed);
        } else {
            perror("✗ Error durante el respaldo");
            return EXIT_FAILURE;
        }
        
        return EXIT_SUCCESS;
    }
    
    fprintf(stderr, "Error: Opción no reconocida '%s'\n", argv[1]);
    print_help(argv[0]);
    return EXIT_FAILURE;
}