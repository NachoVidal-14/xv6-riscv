# Tarea 2: Lottery Scheduling en XV6

**Grupo:** E  
**Integrantes:** Ignacio Vidal / Felipe Céspedes
**Fecha:** 29 de Octubre de 2025  
**Repositorio:** https://github.com/NachoVidal-14/xv6-riscv/tree/GrupoE_Tarea2

---

## 1. Funcionamiento y Lógica de la Implementación

### ¿Qué es Lottery Scheduling?

Lottery Scheduling es un algoritmo de planificación probabilístico donde cada proceso tiene "tickets de lotería". El scheduler hace un sorteo en cada ciclo, y el proceso ganador obtiene tiempo de CPU. Más tickets = más probabilidad de ganar = más tiempo de CPU.

### Lógica del Algoritmo

Nuestro scheduler funciona en 4 pasos simples:

**1. Contar tickets totales:**
```c
for cada proceso RUNNABLE:
    total_tickets += proceso.tickets
```

**2. Sortear un ganador:**
```c
ganador = numero_aleatorio(1, total_tickets)
```

**3. Buscar al ganador:**
```c
acumulador = 0
for cada proceso RUNNABLE:
    acumulador += proceso.tickets
    if acumulador >= ganador:
        este_proceso_gana()
        break
```

**4. Ejecutar y contar:**
```c
proceso.run_slices++  // Contabilidad
ejecutar(proceso)
```

### Ejemplo con Números

Si tenemos 3 procesos:
- Proceso A: 10 tickets
- Proceso B: 20 tickets  
- Proceso C: 70 tickets

Total = 100 tickets

Sorteo genera número 65:
- A: tickets 1-10 (no gana)
- B: tickets 11-30 (no gana)
- C: tickets 31-100 (¡GANA! porque 65 está en su rango)

Probabilidad de C = 70/100 = 70% → Recibirá ~70% del CPU en el largo plazo.

### Resultado del Programa de Prueba

```
$ demo
Starting Lottery Scheduling Demo with 10 processes

[Output mezclado al inicio - normal por concurrencia]

Process 9: PID=13, Tickets=500 - Finished work
Process 7: PID=11, Tickets=400 - Finished work
Process 8: PID=12, Tickets=450 - Finished work
Process 5: PID=9, Tickets=300 - Finished work
Process 6: PID=10, Tickets=350 - Finished work
Process 3: PID=7, Tickets=200 - Finished work
Process 4: PID=8, Tickets=250 - Finished work
Process 1: PID=5, Tickets=100 - Finished work
Process 2: PID=6, Tickets=150 - Finished work
Process 0: PID=4, Tickets=50 - Finished work

All processes completed!
```

**Análisis:**
- ✅ Procesos con 400-500 tickets terminan PRIMERO
- ✅ Procesos con 50-150 tickets terminan ÚLTIMO
- ✅ Esto demuestra que más tickets = más CPU

**Nota:** Pequeñas variaciones en el orden son normales (ej: Process 5 antes que Process 6) porque el algoritmo es probabilístico, no determinístico. Lo importante es la tendencia general.

---

## 2. Modificaciones Realizadas

### Archivos Modificados

#### **kernel/proc.h** - Definición de la estructura del proceso
```c
struct proc {
  // ... campos existentes ...
  
  int tickets;      // Número de tickets del proceso
  int run_slices;   // Contador de ejecuciones
};
```
**Razón:** Necesitamos almacenar los tickets y llevar contabilidad de cuántas veces se ejecuta cada proceso.

---

#### **kernel/proc.c** - Implementación del scheduler

**Cambio 1: Generador de números aleatorios**
```c
static unsigned long randseed = 1;

unsigned long random(void)
{
  randseed = randseed * 1103515245 + 12345;
  return (randseed / 65536) % 32768;
}
```
**Razón:** XV6 no tiene `rand()`, necesitamos nuestra propia función para generar números pseudo-aleatorios.

**Cambio 2: Inicialización en `allocproc()`**
```c
p->tickets = 100;      // Default: 100 tickets
p->run_slices = 0;     // Contador empieza en 0
```
**Razón:** Todos los procesos nuevos deben tener tickets (mínimo 1, por defecto 100).

**Cambio 3: Nueva función `scheduler()`**
```c
void scheduler(void)
{
  // 1. Calcular total de tickets
  int total_tickets = 0;
  for(p = proc; p < &proc[NPROC]; p++) {
    if(p->state == RUNNABLE)
      total_tickets += p->tickets;
  }
  
  // 2. Si no hay tickets, continuar (robustez)
  if(total_tickets == 0)
    continue;
  
  // 3. Generar ganador
  int winner = (random() % total_tickets) + 1;
  
  // 4. Buscar ganador y ejecutar
  int accumulator = 0;
  for(p = proc; p < &proc[NPROC]; p++) {
    if(p->state == RUNNABLE) {
      accumulator += p->tickets;
      if(accumulator >= winner) {
        p->state = RUNNING;
        p->run_slices++;  // Contabilidad
        swtch(...);       // Ejecutar
        break;
      }
    }
  }
}
```
**Razón:** Reemplaza el Round-Robin anterior. Ahora la selección es por lotería en lugar de secuencial.

---

#### **kernel/sysproc.c** - System call settickets
```c
uint64 sys_settickets(void)
{
  int n;
  argint(0, &n);
  
  if(n < 1)
    n = 1;  // Robustez: mínimo 1 ticket
  
  struct proc *p = myproc();
  p->tickets = n;
  
  return 0;
}
```
**Razón:** Permite a los procesos cambiar su cantidad de tickets. Valida que sea al menos 1.

---

#### **kernel/syscall.h** - Número de la syscall
```c
#define SYS_settickets 22
```
**Razón:** Cada syscall necesita un número único. 22 era el siguiente disponible.

---

#### **kernel/syscall.c** - Registro de la syscall
```c
extern uint64 sys_settickets(void);  // Declaración

static uint64 (*syscalls[])(void) = {
  // ... otras syscalls ...
  [SYS_settickets] sys_settickets,   // Agregar al array
};
```
**Razón:** El kernel necesita saber qué función llamar cuando se invoca `settickets()`.

---

#### **user/usys.pl** - Stub en ensamblador
```perl
entry("settickets");
```
**Razón:** Genera el código ensamblador que permite llamar la syscall desde user space.

---

#### **user/user.h** - Prototipo para usuario
```c
int settickets(int);
```
**Razón:** Los programas de usuario necesitan saber que existe `settickets()` y cómo usarla.

---

#### **user/demo.c** - Programa de prueba (NUEVO)
```c
int main() {
  for(i = 0; i < 10; i++) {
    if(fork() == 0) {
      settickets(50 * (i + 1));  // 50, 100, 150, ..., 500
      do_work();
      exit(0);
    }
  }
  // Padre espera a todos
  for(i = 0; i < 10; i++)
    wait(0);
}
```
**Razón:** Demuestra que el scheduler funciona: procesos con más tickets terminan primero.

---

#### **Makefile** - Compilar demo
```makefile
UPROGS=\
  # ... otros programas ...
  $U/_demo\
```
**Razón:** Hace que `demo` se compile automáticamente con `make qemu`.

---

## 3. Dificultades Encontradas y Soluciones

### Dificultad 1: Comprensión inicial del algoritmo
**Problema:** Al inicio no teníamos claro cómo implementar exactamente la lógica del sorteo y la selección del ganador.

**Solución:** Investigamos en internet y con ayuda de IA logramos entender el concepto de "rangos acumulativos de tickets" y cómo recorrer los procesos sumando tickets hasta encontrar el ganador. Una vez comprendido, la implementación fue directa.

### Dificultad 2: Output mezclado en la salida
**Problema:** Al ejecutar `demo`, las primeras líneas aparecen completamente mezcladas e ilegibles:
```
PrPocrPeProcrPrsePros 8: PcIeDPsPoroococesesccess...
```

**Solución:** Investigamos y entendimos que esto NO es un error, sino comportamiento esperado en un sistema con concurrencia real. Los 10 procesos intentan imprimir simultáneamente y sus salidas se entrelazan porque no hay sincronización entre los `printf()`. Los procesos se interrumpen entre sí antes de completar sus prints. Lo importante es verificar que las líneas de "Finished work" muestran la tendencia correcta (más tickets → termina antes).

### Dificultad 3: Archivo README vs README.md
**Problema:** XV6 necesita un archivo `README` (sin extensión) para construir el sistema de archivos. Al reemplazarlo con `README.md`, make qemu fallaba.

**Solución:** Mantuvimos el `README` original de XV6 y creamos nuestro informe como `INFORME.md` para evitar conflictos.

### Dificultad 4: Ubicación del struct proc
**Problema:** Inicialmente buscamos `struct proc` en `proc.c` pero no lo encontramos.

**Solución:** Descubrimos que está en `proc.h`, no en `proc.c`. Una vez identificado el archivo correcto, agregamos los campos sin problemas.

---

## 4. Posibles Problemas de Lottery Scheduling

### 4.1 Starvation (Inanición)
Un proceso con muy pocos tickets puede esperar mucho tiempo antes de ejecutarse. Si hay 10 procesos con 1000 tickets y 1 proceso con 1 ticket, este último tiene solo 1/10001 ≈ 0.01% de probabilidad en cada sorteo.

**Ejemplo real:** En nuestro demo, Process 0 (50 tickets) tiene que competir contra Process 9 (500 tickets). Process 0 tiene 10 veces menos probabilidad de ganar, lo que se traduce en tiempos de espera mucho mayores.

### 4.2 Falta de Predictibilidad
Al ser probabilístico, no hay garantías de CUÁNDO se ejecutará un proceso. Un proceso con 50% de tickets podría, por mala suerte, no ejecutarse en varios ciclos consecutivos.

**Impacto:** No es adecuado para sistemas de tiempo real que necesitan garantías determinísticas. Un proceso crítico podría perderse un deadline por mala suerte en los sorteos.

### 4.3 Overhead Computacional
En cada ciclo del scheduler:
1. Recorremos TODOS los procesos para sumar tickets (O(n))
2. Generamos número aleatorio
3. Recorremos NUEVAMENTE para encontrar ganador (O(n))

Con muchos procesos, este doble recorrido puede ser costoso. Round-Robin solo necesita avanzar al siguiente (O(1)).

### 4.4 Injusticia a Corto Plazo
Aunque es justo en el largo plazo (las proporciones convergen), puede ser muy injusto a corto plazo. En nuestro output vimos que Process 5 (300 tickets) terminó antes que Process 6 (350 tickets), aunque la diferencia no es grande.

**Contraste:** Round-Robin garantiza que todos los procesos RUNNABLE se ejecuten dentro de un período acotado (n × quantum).

### 4.5 Dificultad de Control
Es difícil traducir requisitos reales ("necesito 30% de CPU") a un número de tickets, especialmente cuando el total de tickets cambia dinámicamente al crearse/terminarse procesos.

### 4.6 Vulnerabilidad a Manipulación
Si los procesos pueden cambiar sus propios tickets, uno malicioso podría hacer `settickets(1000000)` y monopolizar la CPU. Se necesitan:
- Control de privilegios (solo root puede aumentar tickets)
- Límites máximos por proceso
- Auditoría de cambios sospechosos

---

## 5. Instrucciones de Compilación y Ejecución

```bash
# Compilar XV6
make clean
make qemu

# Dentro de XV6, ejecutar:
$ demo

# Salir de QEMU:
Ctrl+A, luego X
```

---

## 6. Conclusión

La implementación de Lottery Scheduling fue exitosa. El algoritmo proporciona una asignación proporcional de CPU basada en tickets, como lo demuestra nuestro programa de prueba donde los procesos con más tickets consistentemente terminan antes.

Sin embargo, este enfoque tiene limitaciones importantes: no es predecible, tiene overhead mayor que Round-Robin, y puede causar starvation en procesos con pocos tickets. Es útil para sistemas de propósito general que necesitan control proporcional flexible, pero no para sistemas de tiempo real.