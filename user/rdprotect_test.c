#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
main(void) 
{
  char *addr = sbrk(0); // Dirección actual del heap
  sbrk(4096);           // Reservar una página (4096 bytes = PGSIZE)
  
  // Escribir valor inicial para verificar que la página funciona
  addr[0] = 'Z';
  printf("Valor inicial escrito: %c\n", addr[0]);
  
  // Proteger contra lectura
  printf("\nProtegiendo página contra lectura...\n");
  if (mrdprotect(addr, 1) < 0) {
    printf("ERROR: mrdprotect falló\n");
    exit(1);
  }
  printf("✓ Página protegida exitosamente\n");
  
  // Escritura aún debe estar permitida
  printf("\nIntentando escribir (debería funcionar)...\n");
  addr[0] = 'A';
  printf("✓ Escritura exitosa\n");
  
  // Intento de lectura debería provocar page fault
  printf("\nIntentando leer (debería causar page fault)...\n");
  printf("Si ves este mensaje y luego un crash, ¡LA PROTECCIÓN FUNCIONA!\n");
  
  char c = addr[0];  // ← Esta línea DEBE causar un page fault
  
  // Si llegamos aquí, la protección NO está funcionando
  printf("\n❌ FALLO: Valor leído: %c\n", c);
  printf("❌ La protección NO está funcionando correctamente\n");
  
  // Este código nunca debería ejecutarse si la protección funciona
  if (munrdprotect(addr, 1) < 0) {
    printf("ERROR: munrdprotect falló\n");
    exit(1);
  }
  printf("Protección revertida correctamente.\n");
  
  exit(0);
}