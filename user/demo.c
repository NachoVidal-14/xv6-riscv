// user/demo.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NPROC 10
#define WORK_TIME 1000000  // Cantidad de trabajo a realizar

void
do_work(int iterations)
{
  int i, j;
  volatile int dummy = 0;
  
  for(i = 0; i < iterations; i++) {
    for(j = 0; j < 1000; j++) {
      dummy += i * j;
    }
  }
}

int
main(int argc, char *argv[])
{
  int i, pid;
  int tickets;
  
  printf("Starting Lottery Scheduling Demo with %d processes\n\n", NPROC);
  
  for(i = 0; i < NPROC; i++) {
    pid = fork();
    
    if(pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }
    
    if(pid == 0) {
      // Proceso hijo
      tickets = 50 * (i + 1);  // 50, 100, 150, ..., 500
      settickets(tickets);
      
      printf("Process %d: PID=%d, Tickets=%d - Starting work\n", 
             i, getpid(), tickets);
      
      // Realizar trabajo
      do_work(WORK_TIME);
      
      printf("Process %d: PID=%d, Tickets=%d - Finished work\n", 
             i, getpid(), tickets);
      
      exit(0);
    }
  }
  
  // Proceso padre espera a todos los hijos
  for(i = 0; i < NPROC; i++) {
    wait(0);
  }
  
  printf("\nAll processes completed!\n");
  printf("Check kernel logs for run_slices statistics\n");
  
  exit(0);
}