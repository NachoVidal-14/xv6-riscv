# Tarea 3: Protección de Lectura en XV6

**Grupo:** E  
**Integrantes:** Ignacio Vidal / Felipe Céspedes  
**Fecha:** Noviembre 2024  
**Repositorio:** https://github.com/NachoVidal-14/xv6-riscv/tree/GrupoE_Tarea3

---

## 📋 Descripción

Implementación de un mecanismo de protección de memoria que permite **escritura pero bloquea lectura**. Esto es útil para proteger datos sensibles como claves criptográficas, contraseñas y tokens de autenticación, donde necesitamos escribir información pero no queremos que pueda ser leída accidentalmente (o maliciosamente).

---

## 🔧 System Calls Implementadas

### `int mrdprotect(void *addr, int len)`

Protege `len` páginas comenzando en la dirección `addr`, removiendo el permiso de lectura pero manteniendo el permiso de escritura.

**Parámetros:**
- `addr`: Dirección inicial (debe estar alineada a página, 4096 bytes)
- `len`: Número de páginas a proteger

**Retorna:**
- `0` en éxito
- `-1` en error

**Errores posibles:**
- `addr` no está alineada a página
- `len <= 0`
- Alguna dirección no pertenece al espacio de usuario
- Alguna página no está mapeada (no tiene PTE_V)

---

### `int munrdprotect(void *addr, int len)`

Restaura el permiso de lectura en las páginas previamente protegidas.

**Parámetros:**
- `addr`: Dirección inicial (debe estar alineada a página)
- `len`: Número de páginas a desproteger

**Retorna:**
- `0` en éxito
- `-1` en error

---

## 🏗️ Implementación

### Archivos Modificados

| Archivo | Descripción |
|---------|-------------|
| `kernel/syscall.h` | Definición de números de syscall (23 y 24) |
| `kernel/syscall.c` | Registro de syscalls en el array de syscalls |
| `kernel/defs.h` | Declaración de prototipos de funciones |
| `kernel/sysproc.c` | Wrappers que extraen argumentos del trapframe |
| `kernel/vm.c` | Implementación principal de las funciones |
| `user/usys.pl` | Generación de stubs de usuario |
| `user/user.h` | Interfaz pública para programas de usuario |
| `user/rdprotect_test.c` | Test principal (debe crashear) |
| `user/rdprotect_validation_test.c` | Tests de validación |
| `Makefile` | Compilación de los programas de test |

---

## 💡 Lógica de Implementación

### Función `mrdprotect(void *addr, int len)`

```
1. Obtener proceso actual con myproc()
2. Validar que addr esté alineada a página (múltiplo de 4096)
3. Validar que len > 0
4. Validar que addr esté en espacio de usuario (< MAXVA)
5. Validar que todo el rango esté dentro del heap del proceso
6. Para cada página en el rango [addr, addr + len*PGSIZE):
   a. Obtener PTE usando walk(pagetable, va, 0)
   b. Verificar que PTE existe (no es NULL)
   c. Verificar que sea válido (PTE_V) y de usuario (PTE_U)
   d. Limpiar bit PTE_R: *pte = *pte & ~PTE_R
7. Retornar 0 si todo OK, -1 si algún error
```

### Función `munrdprotect(void *addr, int len)`

```
1. Mismas validaciones que mrdprotect (pasos 1-5)
2. Para cada página en el rango:
   a. Obtener PTE usando walk(pagetable, va, 0)
   b. Verificar que PTE existe
   c. Verificar validez (PTE_V y PTE_U)
   d. Activar bit PTE_R: *pte = *pte | PTE_R
3. Retornar 0 si todo OK, -1 si algún error
```

---

## 🔑 Operaciones de Bits Críticas

```c
// Limpiar bit PTE_R (quitar permiso de lectura)
*pte = *pte & ~PTE_R;

// Activar bit PTE_R (restaurar permiso de lectura)  
*pte = *pte | PTE_R;
```

**Explicación:**
- `PTE_R` es una máscara con el bit de lectura en 1 (definido en `kernel/riscv.h`)
- `~PTE_R` invierte los bits, dejando el bit R en 0 y todos los demás en 1
- `& ~PTE_R` hace AND con la máscara invertida, limpiando solo el bit R
- `| PTE_R` hace OR con la máscara original, activando el bit R

---

## 🧪 Pruebas

### Test Principal: `rdprotect_test`

```bash
$ rdprotect_test
```

**Comportamiento esperado:**

```
Valor inicial escrito: Z

Protegiendo página contra lectura...
✓ Página protegida exitosamente

Intentando escribir (debería funcionar)...
✓ Escritura exitosa

Intentando leer (debería causar page fault)...
Si ves este mensaje y luego un crash, ¡LA PROTECCIÓN FUNCIONA!
usertrap(): unexpected scause 0x000000000000000d pid=4
            sepc=0x0000000000000XXX stval=0x0000000000004000
```

**⚠️ IMPORTANTE:** El crash con page fault **NO ES UN ERROR**. Es la prueba de que la protección está funcionando correctamente. Si el programa imprime `"❌ FALLO: Valor leído..."`, significa que la protección NO funciona.

---

### Test de Validaciones: `rdprotect_validation_test`

```bash
$ rdprotect_validation_test
```

**Resultado esperado:**

```
=== Tests de Validación para mrdprotect ===

Test 1: Dirección no alineada a página
✓ Rechazado correctamente (dirección no alineada)

Test 2: len <= 0
✓ Rechazado correctamente (len = 0)
✓ Rechazado correctamente (len negativo)

Test 3: Dirección fuera del espacio del proceso
✓ Rechazado correctamente (dirección inválida)

Test 4: Protección y desprotección exitosa (1 página)
✓ Protección exitosa
✓ Desprotección exitosa
✓ Lectura funciona después de desproteger

Test 5: Protección de múltiples páginas
✓ Protección de 3 páginas exitosa
✓ Desprotección de 3 páginas exitosa

Test 6: Desproteger página no protegida
✓ Desprotección de página no protegida exitosa
  (esto es OK, simplemente activa el bit R)

=== Todos los tests de validación completados ===
```

---

## 🚀 Instrucciones de Compilación

```bash
# Limpiar compilaciones anteriores
make clean

# Compilar y ejecutar xv6
make qemu

# Dentro de xv6, ejecutar los tests:
$ rdprotect_test
$ rdprotect_validation_test

# Salir de xv6:
Ctrl+A, luego X
```

---

## 🐛 Desafíos Encontrados

### 1. **Entender la estructura de page tables en RISC-V**

**Problema:** No estaba claro cómo navegar la jerarquía de page tables de 3 niveles en RISC-V.

**Solución:** Estudiamos la función `walk()` en `kernel/vm.c`, que ya implementa el recorrido de la tabla de páginas. Usamos esta función existente en lugar de implementar nuestra propia navegación.

---

### 2. **Manipulación correcta de bits PTE**

**Problema:** Los PTEs contienen múltiples bits (V, R, W, X, U, etc.) y debíamos modificar solo el bit R sin alterar los demás.

**Solución:** Revisamos `kernel/riscv.h` donde están definidas todas las máscaras de bits. Usamos operaciones bitwise AND con `~PTE_R` para limpiar y OR con `PTE_R` para activar.

```c
// Estructura de un PTE en RISC-V:
// [63:54] Reserved
// [53:10] PPN (Physical Page Number)
// [9:8]   RSW (Reserved for Software)
// [7]     D (Dirty)
// [6]     A (Accessed)
// [5]     G (Global)
// [4]     U (User)
// [3]     X (eXecute)
// [2]     W (Write)
// [1]     R (Read)
// [0]     V (Valid)
```

---

### 3. **Page fault es el comportamiento CORRECTO**

**Problema:** Inicialmente pensamos que el programa debía ejecutarse completamente sin errores.

**Solución:** Entendimos que cuando `rdprotect_test` intenta leer memoria protegida y el kernel genera un page fault (`scause 0xd` = Load Page Fault), esto **demuestra que la protección funciona**. Es exactamente el comportamiento deseado.

---

### 4. **Validación del rango de memoria**

**Problema:** Debíamos asegurarnos de que todas las páginas en el rango `[addr, addr + len*PGSIZE)` fueran válidas.

**Solución:** Agregamos validación para verificar que `va + (len * PGSIZE) <= p->sz`, asegurando que todo el rango esté dentro del heap del proceso.

---

## 🎯 Casos de Uso

Esta implementación es útil para:

1. **Claves Criptográficas:** Escribir claves privadas en memoria sin riesgo de lectura accidental por bugs o side-channel attacks.

2. **Credenciales de Autenticación:** Almacenar passwords o tokens que deben escribirse pero no leerse directamente.

3. **Protección contra Memory Dumps:** Si un atacante logra hacer un dump de memoria, no podrá leer las regiones protegidas.

4. **Write-Only Logs:** Logs de auditoría que pueden escribirse pero no leerse por el proceso (solo por el sistema).

---

## ⚠️ Limitaciones

1. **Granularidad de página completa:** La protección es a nivel de página (4KB), no de bytes individuales.

2. **No protege contra escritura maliciosa:** Un atacante puede sobrescribir datos protegidos.

3. **Requiere privilegios de kernel:** Un atacante con acceso al kernel puede cambiar los bits PTE directamente.

4. **No persiste:** La protección se pierde cuando el proceso termina o el sistema reinicia.

5. **Performance:** Llamar estas syscalls tiene overhead. No usar en hot paths.

---

## 📊 Análisis de Performance

| Operación | Costo |
|-----------|-------|
| `mrdprotect(addr, 1)` | O(1) - Una página |
| `mrdprotect(addr, n)` | O(n) - n páginas |
| `walk()` por página | O(1) amortizado (si ya está en TLB) |
| Page fault | ~100-1000 ciclos |

**Nota:** El TLB (Translation Lookaside Buffer) cachea las traducciones de direcciones virtuales a físicas. Después de modificar un PTE, el hardware de RISC-V invalida automáticamente la entrada del TLB cuando ocurre un page fault.

---

## 🔒 Consideraciones de Seguridad

### Ataques Mitigados:
- ✅ Lectura accidental por bugs del programa
- ✅ Memory dumps parciales
- ✅ Algunos side-channel attacks basados en lectura

### Ataques NO Mitigados:
- ❌ Escritura maliciosa (puede sobrescribir datos)
- ❌ Acceso con privilegios de kernel
- ❌ Spectre/Meltdown (requieren mitigaciones a nivel hardware)
- ❌ DMA attacks (acceso directo a memoria física)

---

## 📚 Referencias Técnicas

- **RISC-V Privileged Specification v1.10:** Documentación oficial de page tables y permisos
- **XV6 Book (RISC-V edition):** Capítulo 3 - Page Tables
- **MIT 6.S081 Lab:** Virtual Memory assignments
- **walk() function:** `kernel/vm.c:81` - Navegación de page tables

---

## ✅ Conclusión

Implementamos exitosamente un mecanismo de protección de memoria "solo escritura" en XV6. Las syscalls `mrdprotect()` y `munrdprotect()` permiten control fino sobre permisos de lectura en páginas de memoria, creando una herramienta útil para proteger datos sensibles en aplicaciones criptográficas.

El test principal (`rdprotect_test`) demuestra que intentar leer memoria protegida resulta en un page fault, confirmando que la protección funciona correctamente. El test de validaciones confirma que todas las validaciones de errores funcionan como se espera.

---

## 👥 Contribuciones

- **Ignacio Vidal:** Implementación de syscalls y funciones del kernel, debugging
- **Felipe Céspedes:** Tests de validación, documentación, análisis de seguridad

---

## 📝 Historial de Commits

```bash
git log --oneline
```

1. Add mrdprotect and munrdprotect syscall declarations
2. Implement mrdprotect and munrdprotect in vm.c
3. Add test programs for read protection
4. Add comprehensive documentation and README