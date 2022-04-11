#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    pthread_mutex_t *me;
    sem_t *b1;
    sem_t *b2;
    int counter;
} BarrierSem;

typedef struct {
    pthread_t tid;
    int id;
    int *v;
    int n;
    BarrierSem *b;
} Thread_pars;

unsigned int pow2(int);
void* thread_fn(void *);

unsigned int pow2(int n){
    unsigned int r=1;
    for(int i=0;i<n;i++)
        r = r<<1;
    return r;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr,"Wrong number of input paramiters\n");
        exit(1);
    }
    unsigned int n = atoi(argv[1]);
    unsigned int pow2n = pow2(n);
    int *v = (int *)malloc(pow2n*sizeof(int));
    if(v == NULL){
        fprintf(stderr,"Error malloc\n");
        exit(1);
    }
    for(int i=0; i<pow2n ;i++)
        v[i] =(rand_r(&n) % 9) + 1;
    
    n = atoi(argv[1]);
    
    fprintf(stdout,"Before:\n");
    for(int i=0;i<pow2n;i++)
        fprintf(stdout,"%d ",v[i]);
    fprintf(stdout,"\n");

    // for(int i=0;i<n;i++){
    //     int pow2i = pow2(i);
    //     for(int j=pow2n-1;j>=pow2i;j--){
    //         v[j] += v[j-pow2i];
    //     }
    //     for(int i=0;i<pow2n;i++){
    //         fprintf(stdout,"%d ",v[i]);
    //     }
    //     fprintf(stdout,"\n");
    // }

    Thread_pars *thread_pars = (Thread_pars *)malloc((pow2n-1)*sizeof(Thread_pars));
    if(thread_pars == NULL){
        fprintf(stderr,"Error malloc\n");
        exit(1);
    } 

    BarrierSem *b = (BarrierSem *)malloc(sizeof(BarrierSem));
    b->me = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(b->me,NULL);
    b->b1 = (sem_t *)malloc(sizeof(sem_t));
    sem_init(b->b1,0,0);
    b->b2 = (sem_t *)malloc(sizeof(sem_t));
    sem_init(b->b2,0,1);
    b->counter = 0;


    for(int i=0;i<pow2n-1;i++){
        thread_pars[i].id = i+1;
        thread_pars[i].v = v;
        thread_pars[i].n = n;
        thread_pars[i].b = b;
        pthread_create(&(thread_pars[i].tid),NULL,thread_fn,(void *)&(thread_pars[i]));
    }

    fprintf(stdout,"After:\n");
    for(int i=0;i<pow2n-1;i++){
        fprintf(stdout,"%d ",v[i]);
        pthread_join(thread_pars[i].tid,NULL);
    }
    fprintf(stdout,"%d ",v[pow2n-1]);
    fprintf(stdout,"\n");

    sem_destroy(b->b1);
    sem_destroy(b->b2);
    pthread_mutex_destroy(b->me);
    free(b->b1);
    free(b->b2);
    free(b->me);
    free(b);
    free(thread_pars);
    free(v);
        
    return 0;
}

void* thread_fn(void *voidPar){
    Thread_pars *pars = (Thread_pars *) voidPar;
    int sum;
    for(int i = 0; i<pars->n;i++){
        int pow2i = pow2(i);
        int barrierSize = pow2(pars->n)-pow2i;

        // kill useless threads
        if(pars->id < pow2i)
            pthread_exit(0);   

        // do the sum
        sum = pars->v[pars->id] + pars->v[pars->id-pow2i];
        // prologue barrier 1
        pthread_mutex_lock(pars->b->me);
            pars->b->counter++;
            //printf("sum %d\n",pars->b->counter);
            if(pars->b->counter == barrierSize){
                sem_wait(pars->b->b2);
                sem_post(pars->b->b1);
            }
        pthread_mutex_unlock(pars->b->me);
        
        // Barrier 1
        sem_wait(pars->b->b1);
        sem_post(pars->b->b1);
        
        // save the result
        pars->v[pars->id] = sum;

        // prologue barrier 2
        pthread_mutex_lock(pars->b->me);
            pars->b->counter--;
            //printf("save %d\n",pars->b->counter);
            if(pars->b->counter == 0){
                sem_wait(pars->b->b1);
                sem_post(pars->b->b2);
            }
        pthread_mutex_unlock(pars->b->me);
        
        //Barrier 2
        sem_wait(pars->b->b2);
        sem_post(pars->b->b2);
    }
    pthread_exit(0);
}
