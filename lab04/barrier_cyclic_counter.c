#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "semaphore.h"

typedef struct barrier_s {
  sem_t sem1;
  sem_t sem2;
  int count;
  pthread_mutex_t mutex;
} barrier_t;

barrier_t *barrier_d;
int num_threads;

static void *code (void *arg) {
  int i, j, r;
	
  pthread_detach (pthread_self ());
  for (i=0;i<10;i++){
  r =	random() % 5;
  sleep(r);
  printf("%ld slept %d  sec in phase 1\n", pthread_self(), r);
  pthread_mutex_lock (&barrier_d->mutex);
    barrier_d->count++;
    if (barrier_d->count == num_threads)
    	for (j=0;j<num_threads;j++)
    	  sem_post(&barrier_d->sem1);
  pthread_mutex_unlock (&barrier_d->mutex);
    
  sem_wait(&barrier_d->sem1);	
  
  printf("%ld phase 2\n", pthread_self());
  
  pthread_mutex_lock (&barrier_d->mutex);
    barrier_d->count--;
    if (barrier_d->count == 0) 
    	for (j=0;j<num_threads;j++)
    	  sem_post(&barrier_d->sem2);
  pthread_mutex_unlock (&barrier_d->mutex);
  
  sem_wait(&barrier_d->sem2);
}
  return  0;
}  

int
main (int argc, char ** argv)
{
  pthread_t *th;

  int i;
  
  if (argc != 2) {
    printf ("Syntax: %s num_threads\n", argv[0]);
    return (1);
  }
  num_threads = atoi(argv[1]);
  th = (pthread_t *) malloc (num_threads * sizeof(pthread_t));
  barrier_d = (barrier_t *) malloc (sizeof(barrier_t));
  barrier_d->count = 0;
  pthread_mutex_init (&barrier_d->mutex, NULL);
  //barrier_d->sem1 = (sem_t *) malloc(sizeof(sem_t));
  sem_init (&barrier_d->sem1, 0, 0);
  //barrier_d->sem2 = (sem_t *) malloc(sizeof(sem_t));
  sem_init (&barrier_d->sem2, 0, 0);
  
    /* Create the threads */
  for (i=0;i< num_threads; i++)
    pthread_create (&th[i], NULL, code, NULL);
  
  pthread_exit ((void *) pthread_self());
    
  return 0;
}
