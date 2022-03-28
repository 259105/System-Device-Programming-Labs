#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#define ROWS 3
#define COLUMS 2
#define X 3

void *computeElement(void *);
void mat_mul(int **, int **, int, int, int, int **);
void printMatrix(int **, int, int);

// public parameters for mat_mul
int **globA; // left matrix;
int **globB; // right matrix;
int **globC; // result matrix;
int globr, globx, globc; // dimensions 

typedef struct computeElement_pars {
    pthread_t tid;
    int pos;    // position of cells to compute
} ComputeElement_pars;

int main(int argc, char *argv[]){
    // ----- The matrixes can be optained in different way -----

    // int A[][X] = {{1,2,3},{3,2,1},{1,1,1}};
    // int B[][COLUMS] = {{1,2},{2,2},{3,2}};
    // int C[ROWS][COLUMS];
    
    int **A = (int **) malloc(ROWS*sizeof(int *));
    int **B = (int **) malloc(X*sizeof(int *));
    int **C = (int **) malloc(ROWS*sizeof(int *));
    for(int i=0;i<ROWS;i++){
        A[i] = (int *) malloc(X*sizeof(int));
        C[i] = (int *) malloc(COLUMS*sizeof(int));
    }
    for(int i=0;i<X;i++)
        B[i] = (int *) malloc(COLUMS*sizeof(int));

    int k=1;
    for(int i=0;i<ROWS;i++)
        for(int j=0;j<X;j++)
            A[i][j]=k++;

    for(int i=0;i<X;i++)
        for(int j=0;j<COLUMS;j++)
            B[i][j]=k++;

    // printMatrix(A,ROWS,X);
    // printMatrix(B,X,COLUMS);

    // ---------------------------------------------------------

    mat_mul(A,B,ROWS,X,COLUMS,C);

    // -- print in stdout the matrix ---
    printMatrix(C, ROWS,COLUMS);

    return 0;
}

void printMatrix(int **M, int row, int colum){
    for(int i=0;i<row;i++){
        for(int j=0;j<colum;j++)
            fprintf(stdout,"%d ",M[i][j]);
        fprintf(stdout,"\n");
    }
    fprintf(stdout,"\n");
}

void *computeElement(void * par){
    ComputeElement_pars *parameters = (ComputeElement_pars *)par;
    int ri=parameters->pos/globc;
    int ci=parameters->pos%globc;
    // printf("calculcate(%d, %d) : %d\n",ri,ci,parameters->pos);
    globC[ri][ci] = 0;
    for(int i=0;i<globx;i++)
        globC[ri][ci]+=globA[ri][i]*globB[i][ci];
    pthread_exit(0);
}

void mat_mul(int **A, int **B, int r, int x, int c, int **C){
    int rc;
    ComputeElement_pars **parameter = (ComputeElement_pars **)malloc((r*c) * sizeof(ComputeElement_pars *));
    if(parameter == NULL){
        fprintf(stderr,"Error malloc array of parameters\n");
        exit(1);
    }
    globA=A;
    globB=B;
    globC=C;
    globr=r;
    globc=c;
    globx=x;
    for(int i=0;i<r*c;i++){
        parameter[i] = (ComputeElement_pars *)malloc(sizeof(ComputeElement_pars));
        if(parameter[i]==NULL){
            fprintf(stderr,"Error malloc params Thread %d\n",i);
            exit(1);
        }
        parameter[i]->pos=i;
        rc = pthread_create(&(parameter[i]->tid),NULL,computeElement,parameter[i]);
        if(rc){
            fprintf(stderr,"Error creation Thread %d\n", i);
            exit(1);
        }
    }

    for(int i=0;i<r*c;i++){
        pthread_join(parameter[i]->tid,NULL);
        free(parameter[i]);
    }
    free(parameter);
}