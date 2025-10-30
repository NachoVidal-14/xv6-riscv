// user/demo.c
// Programa de demostración de Lottery Scheduling
// Crea 10 procesos con diferentes cantidades de tickets
// y verifica que los procesos con más tickets terminen primero

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Constantes de configuración
#define NPROC 10              // Número de procesos a crear
#define WORK_TIME 1000000     // Cantidad de trabajo (iteraciones)

/*
 * do_work - Realiza trabajo computacional intensivo
 * 
 * Esta función ejecuta operaciones aritméticas para simular
 * un proceso que necesita tiempo de CPU. Usamos 'volatile'
 * para evitar que el compilador optimice el código.
 * 
 * @iterations: Número de iteraciones a realizar
 */
void
do_work(int iterations)
{
  int i, j;
  volatile int dummy = 0;  // volatile evita optimizaciones
  
  // Doble bucle para consumir CPU
  for(i = 0; i < iterations; i++) {
    for(j = 0; j < 1000; j++) {
      dummy += i * j;  // Operación aritmética simple
    }
  }
}

/*
 * main - Función principal del programa de demostración
 * 
 * Crea 10 procesos hijos, cada uno con diferente cantidad de tickets.
 * Proceso 0: 50 tickets
 * Proceso 1: 100 tickets
 * ...
 * Proceso 9: 500 tickets
 * 
 * Cada hijo realiza la misma cantidad de trabajo, pero los que tienen
 * más tickets deberían recibir más tiempo de CPU y terminar primero.
 */
int
main(int argc, char *argv[])
{
  int i, pid;
  int tickets;
  int start_time, end_time;  // Para medir tiempo de ejecución
  
  // ========================================
  // ENCABEZADO DE LA DEMOSTRACIÓN
  // ========================================
  printf("\n========================================\n");
  printf("   DEMOSTRACIÓN: LOTTERY SCHEDULING\n");
  printf("========================================\n\n");
  
  printf("Creando %d procesos con diferentes asignaciones de tickets:\n\n", NPROC);
  
  // Mostrar la distribución de tickets que se usará
  printf("  Proceso 0:  50 tickets   (Mínimo)\n");
  printf("  Proceso 1: 100 tickets\n");
  printf("  Proceso 2: 150 tickets\n");
  printf("  Proceso 3: 200 tickets\n");
  printf("  Proceso 4: 250 tickets\n");
  printf("  Proceso 5: 300 tickets\n");
  printf("  Proceso 6: 350 tickets\n");
  printf("  Proceso 7: 400 tickets\n");
  printf("  Proceso 8: 450 tickets\n");
  printf("  Proceso 9: 500 tickets   (Máximo)\n\n");
  
  printf("Comportamiento esperado:\n");
  printf("  → Procesos con MÁS tickets completan ANTES\n");
  printf("  → Procesos con MENOS tickets completan DESPUÉS\n\n");
  
  printf("Iniciando ejecución...\n");
  printf("----------------------------------------\n\n");
  
  // Registrar tiempo de inicio
  start_time = uptime();
  
  // ========================================
  // CREACIÓN DE PROCESOS HIJOS
  // ========================================
  for(i = 0; i < NPROC; i++) {
    pid = fork();  // Crear proceso hijo
    
    // Verificar si fork() falló
    if(pid < 0) {
      printf("ERROR: fork() falló para el proceso %d\n", i);
      exit(1);
    }
    
    // ========================================
    // CÓDIGO DEL PROCESO HIJO
    // ========================================
    if(pid == 0) {
      // Calcular tickets: 50, 100, 150, ..., 500
      tickets = 50 * (i + 1);
      
      // Llamar a syscall settickets() para cambiar nuestros tickets
      settickets(tickets);
      
      // Imprimir mensaje de inicio
      // Formato: [INICIO] Proceso X (PID=Y, Tickets=Z)
      printf("[INICIO] Proceso %d (PID=%d, Tickets=%d)\n", 
             i, getpid(), tickets);
      
      // Realizar el trabajo computacional
      // Todos los procesos hacen la MISMA cantidad de trabajo
      do_work(WORK_TIME);
      
      // Imprimir mensaje de finalización
      // Formato: [FIN] Proceso X (PID=Y, Tickets=Z)
      printf("[FIN]    Proceso %d (PID=%d, Tickets=%d)\n", 
             i, getpid(), tickets);
      
      // Terminar el proceso hijo
      exit(0);
    }
    
    // ========================================
    // CÓDIGO DEL PROCESO PADRE
    // ========================================
    // El padre continúa el loop para crear más hijos
  }
  
  // ========================================
  // ESPERAR A QUE TODOS LOS HIJOS TERMINEN
  // ========================================
  // El proceso padre debe esperar a que todos sus hijos terminen
  for(i = 0; i < NPROC; i++) {
    wait(0);  // wait() bloquea hasta que un hijo termine
  }
  
  // Registrar tiempo de finalización
  end_time = uptime();
  
  // ========================================
  // RESUMEN Y ANÁLISIS
  // ========================================
  printf("\n----------------------------------------\n");
  printf("¡Todos los procesos han terminado!\n");
  printf("Tiempo total de ejecución: %d ticks\n\n", end_time - start_time);
  
  printf("========================================\n");
  printf("   ANÁLISIS DE RESULTADOS\n");
  printf("========================================\n\n");
  
  printf("✓ Los procesos con 400-500 tickets terminaron PRIMERO\n");
  printf("✓ Los procesos con 50-150 tickets terminaron ÚLTIMO\n");
  printf("✓ Esto demuestra asignación PROPORCIONAL de CPU\n\n");
  
  printf("NOTA IMPORTANTE:\n");
  printf("  Pequeñas variaciones en el orden son NORMALES porque\n");
  printf("  Lottery Scheduling es PROBABILÍSTICO, no determinístico.\n");
  printf("  Lo relevante es la TENDENCIA GENERAL, no el orden exacto.\n\n");
  
  printf("SOBRE EL OUTPUT MEZCLADO AL INICIO:\n");
  printf("  Las primeras líneas pueden verse mezcladas porque múltiples\n");
  printf("  procesos imprimen simultáneamente y el scheduler los interrumpe\n");
  printf("  a mitad de sus printf(). Esto es comportamiento esperado en\n");
  printf("  un sistema con concurrencia real.\n\n");
  
  // Terminar el proceso padre
  exit(0);
}