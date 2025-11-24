#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
main(void) 
{
  printf("=== Tests de Validación para mrdprotect ===\n\n");
  
  char *addr = sbrk(0);
  sbrk(4096); // Reservar una página
  
  // Test 1: Dirección no alineada a página
  printf("Test 1: Dirección no alineada a página\n");
  if (mrdprotect(addr + 10, 1) == -1) {
    printf("✓ Rechazado correctamente (dirección no alineada)\n\n");
  } else {
    printf("✗ FALLO: debería rechazar dirección no alineada\n\n");
  }
  
  // Test 2: len <= 0
  printf("Test 2: len <= 0\n");
  if (mrdprotect(addr, 0) == -1) {
    printf("✓ Rechazado correctamente (len = 0)\n");
  } else {
    printf("✗ FALLO: debería rechazar len = 0\n");
  }
  
  if (mrdprotect(addr, -5) == -1) {
    printf("✓ Rechazado correctamente (len negativo)\n\n");
  } else {
    printf("✗ FALLO: debería rechazar len negativo\n\n");
  }
  
  // Test 3: Dirección fuera del heap del proceso
  printf("Test 3: Dirección fuera del espacio del proceso\n");
  if (mrdprotect((void*)0x9999999999, 1) == -1) {
    printf("✓ Rechazado correctamente (dirección inválida)\n\n");
  } else {
    printf("✗ FALLO: debería rechazar dirección fuera del espacio\n\n");
  }
  
  // Test 4: Protección y desprotección exitosa de una página
  printf("Test 4: Protección y desprotección exitosa (1 página)\n");
  if (mrdprotect(addr, 1) == 0) {
    printf("✓ Protección exitosa\n");
    if (munrdprotect(addr, 1) == 0) {
      printf("✓ Desprotección exitosa\n");
      // Verificar que podemos leer después de desproteger
      addr[0] = 'X';
      char test = addr[0];
      if (test == 'X') {
        printf("✓ Lectura funciona después de desproteger\n\n");
      }
    } else {
      printf("✗ FALLO: munrdprotect falló\n\n");
    }
  } else {
    printf("✗ FALLO: mrdprotect falló\n\n");
  }
  
  // Test 5: Múltiples páginas
  printf("Test 5: Protección de múltiples páginas\n");
  sbrk(8192); // Reservar 2 páginas más (total 3 páginas)
  if (mrdprotect(addr, 3) == 0) {
    printf("✓ Protección de 3 páginas exitosa\n");
    if (munrdprotect(addr, 3) == 0) {
      printf("✓ Desprotección de 3 páginas exitosa\n\n");
    } else {
      printf("✗ FALLO: desprotección de múltiples páginas falló\n\n");
    }
  } else {
    printf("✗ FALLO: protección de múltiples páginas falló\n\n");
  }
  
  // Test 6: Intentar desproteger sin haber protegido
  printf("Test 6: Desproteger página no protegida\n");
  if (munrdprotect(addr, 1) == 0) {
    printf("✓ Desprotección de página no protegida exitosa\n");
    printf("  (esto es OK, simplemente activa el bit R)\n\n");
  }
  
  printf("=== Todos los tests de validación completados ===\n");
  printf("Los tests que requieren page fault deben ejecutarse con rdprotect_test\n");
  
  exit(0);
}