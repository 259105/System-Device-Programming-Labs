#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    pthread_barrier_t *b1;
    pthread_barrier_t *b2;
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
    return 1<<n;
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

    // ------ NORMAL SOLUTION ------    
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

    BarrierSem *b = (BarrierSem *)malloc(sizeof(BarrierSem));
    b->b1 = (pthread_barrier_t *)malloc(sizeof(pthread_barrier_t));
    pthread_barrier_init(b->b1,NULL,pow2n-1);
    b->b2 = (pthread_barrier_t *)malloc(sizeof(pthread_barrier_t));
    pthread_barrier_init(b->b2,NULL,pow2n-1);


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

    pthread_barrier_destroy(b->b1);
    free(b->b1);
    pthread_barrier_destroy(b->b2);
    free(b->b2);
    free(b);
    free(thread_pars);
    free(v);
        
    return 0;
}

void* thread_fn(void *voidPar){
    Thread_pars *pars = (Thread_pars *) voidPar;
    int sum;
    for(int i = 0; i<pars->n;i++){
        int barrierSize = pow2(pars->n)-pow2(i);

        // kill useless threads
        if(pars->id < pow2(i))
            pthread_exit(0);
        if(pars->id == pow2(i)){ // I have to choose a thread that must do the reinitialization of the next barrier, in this case is done by the first thread that will have to die in the next cicle
            pthread_barrier_destroy(pars->b->b2);
            if(pthread_barrier_init(pars->b->b2,NULL,barrierSize) != 0){
                fprintf(stderr,"Error: init barrier 2\n");
                exit(-1);
            }
        }

        // do the sum
        sum = pars->v[pars->id] + pars->v[pars->id-pow2(i)];
        
        pthread_barrier_wait(pars->b->b1);

        if(pars->id == pow2(i)){ // the same thread as before, have to init barr 1 to the next dimension barr, (calculated on pow2(i+1))
            barrierSize = pow2(pars->n)-pow2(i+1);
            pthread_barrier_destroy(pars->b->b1);
            if(barrierSize !=0 && pthread_barrier_init(pars->b->b1,NULL,barrierSize) != 0){ // the next cicle barrier
                fprintf(stderr,"Error: init barrier 1\n");
                exit(-1);
            }
        }

        // save the result
        pars->v[pars->id] = sum;

        // Barrier 2
        pthread_barrier_wait(pars->b->b2);
    }
    pthread_exit(0);
}
