#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct argt {
    pthread_t tid;  // current tid
    int pos;        // position in the recursion, starts from h
    int i;          // position in the tree starts from 0;
    int n;          // degree of children generation
    pthread_t *parents;   // array tid of parents
} Argt;

void *childGeneator(void *);

int main(int argc, char *argv[]){
    //setbuf(stdout,0);
    if(argc != 3){
        fprintf(stderr, "Wrong paramers\n");
        exit(1);
    }

    int h = atoi(argv[1]); // height
    int n = atoi(argv[2]); // grade
    // init of params
    Argt *params = (Argt *)malloc(sizeof(Argt));
    params->i=0;
    params->n=n;
    params->pos=h;
    params->parents = (pthread_t *)malloc(h*sizeof(pthread_t));
    for(int i=0;i<h;i++){
        params->parents[i]=0;
    }
    
    childGeneator(params);
}

void *childGeneator(void *par){
    Argt *pars = (Argt *)par;
    pthread_t currTid =pthread_self();

    //termination condition
    if(pars->pos==0){
        for (int i=0;i<pars->i;i++){
            pthread_t tid = pars->parents[i];
            fprintf(stdout,"%li ",tid);
        }
        fprintf(stdout,"\n");
        pthread_exit(0);
    }

    //fprintf(stdout,"current Tid : %li\n",currTid);

    // array of params of created children, useful for join
    Argt **params = (Argt **)malloc(pars->n*sizeof(Argt *));
    for(int i=0;i<pars->n;i++){
        params[i] = (Argt *)malloc(sizeof(Argt));
        params[i]->i=pars->i+1;
        params[i]->n=pars->n;
        params[i]->pos=pars->pos-1;
        int h=params[i]->pos+params[i]->i;
        params[i]->parents=(pthread_t *)malloc(h*sizeof(pthread_t));

        for(int j=0;j<h;j++){
            params[i]->parents[j]=pars->parents[j];
        }

        params[i]->parents[pars->i]=currTid;
        int rc;
        rc = pthread_create(&(params[i]->tid),NULL,childGeneator,params[i]);
        if (rc) {
            fprintf(stderr, "Errore creation tread\n");
            exit(1);
        }
    }

    for(int i=0;i<pars->n;i++){
        int rc;
        rc = pthread_join(params[i]->tid,NULL);
        if (rc) {
            fprintf(stderr, "Errore creation tread\n");
            exit(1);
        }
    }

    pthread_exit(0);
}
