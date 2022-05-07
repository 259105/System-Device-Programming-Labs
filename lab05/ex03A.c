#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define PREC 1000000

extern int errno;
char err[64];

void product(double *, double **, double *, int , int );
void productVect(double *, double *, double *, int );
void *pthread_func(void *);

typedef struct{
    pthread_mutex_t *mu;
    int n;
} pthread_synch;

typedef struct{
    pthread_t tid;
    double *vr;
    double *vc;
    double **M;
    double *v;
    int n;
    int c;
    double *res;
    pthread_synch *synch;
} pthread_par;


int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "Error: insert the dimension n\n");
        exit(-1);
    }

    int n = atoi(argv[1]);
    unsigned int seed = 0 ;// = getpid();
    int r;

    // Allocation
    double *v1 = (double *) malloc(n*sizeof(double));
    double *v2 = (double *) malloc(n*sizeof(double));
    double *v = (double *) malloc(n*sizeof(double));
    double **M = (double **) malloc(n*sizeof(double *));
    double res = 0;
    for(int i=0; i<n; i++){
        M[i] = (double *) malloc(n*sizeof(double));
        
        // Generating random values
        r = (rand_r(&seed) % PREC);
        v1[i] = (double) r / PREC - 0.5;
        r = (rand_r(&seed) % PREC);
        v2[i] = (double) r / PREC - 0.5;
        for(int j=0; j<n; j++){
            r = (rand_r(&seed) % PREC);
            M[i][j] = (double) r / PREC - 0.5;
        }
    }

#ifdef DEBG
    printf("v1^T = [ ");
    for(int i=0; i<n; i++){
        printf("%f%c ",v1[i], i==n-1 ? ' ' : ',');
    }
    printf(" ]\n\n");

    printf("v2^T = [ ");
    for(int i=0; i<n; i++){
        printf("%f%c ",v2[i], i==n-1 ? ' ' : ',');
    }
    printf("]\n\n");

    printf("mat = [\t");
    for(int i=0; i<n; i++){
        printf("[ ");
        for(int j=0; j<n; j++){
            printf("%f%c",M[i][j], j==n-1 ? ' ' : ',' );
        }
        printf(" ]%c", i==n-1 ? ' ' : ',' );
    }
    printf("]\n\n");
#endif
    // Threads throw
    pthread_par *pars = (pthread_par *) malloc(n * sizeof(pthread_par));
    pthread_synch *synch = (pthread_synch *) malloc(sizeof(pthread_synch));
    synch->mu = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    synch->n = n;
    pthread_mutex_init(synch->mu , NULL);

    for(int i=0; i<n; i++){
        pars[i].vr = v1;
        pars[i].vc = v2;
        pars[i].M = M;
        pars[i].v = v;
        pars[i].n = n;
        pars[i].c = i;
        pars[i].res = &res;
        pars[i].synch = synch;
        pthread_create(&(pars[i].tid),NULL,pthread_func,(void *) &pars[i]);
    }

    // Threads join
    for(int i=0; i<n; i++){
        pthread_join(pars[i].tid, 0);
    }
    pthread_mutex_destroy(synch->mu);

#ifdef DEBG
    printf("v^T = [ ");
    for(int i=0; i<n; i++){
        printf("%f%c ",v[i], i==n-1 ? ' ' : ',');
    }
    printf("]\n\n");
#endif

    // Deallocation
    for(int i=0; i<n; i++){
        free(M[i]);
    }
    free(M);
    free(v2);
    free(v1);

    return 0;
}

void product(double *vr, double **M, double *v, int n, int c){
    v[c] = 0;
    for(int i=0; i<n; i++){
        v[c] += vr[i]*M[i][c];
    }
}

void productVect(double *vr, double *vc, double *res, int n){
    *res = 0;
    for(int i=0; i<n; i++){
        *res += vr[i]*vc[i];
    }
}

void *pthread_func(void *parV){
    pthread_par *pars = (pthread_par *) parV;

    double *vr = pars->vr;
    double *vc = pars->vc;
    double **M = pars->M;
    double *v = pars->v;
    int n = pars->n;
    int c = pars->c;
    double *res = pars->res;
    pthread_synch *synch = pars->synch;

    product(vr, M, v, n, c);

    pthread_mutex_lock(synch->mu);
        synch->n--;
        if(synch->n == 0){
            productVect(v, vc, res, n);
            fprintf(stdout,"%f\n", *res);
        }
    pthread_mutex_unlock(synch->mu);

    pthread_exit(0);
}