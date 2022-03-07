#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void binary(int*, int);

int main(int argc, char *argv[]){
	setbuf(stdout,0);

	if(argc!=2){
		fprintf(stderr,"Wrong paramiters\n");
		exit(1);
	}

	int n = atoi(argv[1]);
	int *vet = (int *) malloc(n*sizeof(int));

	if(vet == NULL){
		fprintf(stderr,"Allocation Error\n");
		exit(1);
	}
	
	fprintf(stderr,"Binary numbers:\n");
	binary(vet, n);
	free(vet);
	return 0;
}

void binary(int *vet, int n){
	int nChild=0;
	for(int i=0; i<n; i++){
		if(fork()){
			// father = 0
			nChild++; 
			vet[i] = 0;
		}else{
			// child = 1
			nChild=0;
			vet[i] = 1;
		}
	}

	char *vetS = (char *) malloc((n+1)*sizeof(char));
	int i;
	for(i=0;i<n;i++){
		sprintf(&vetS[i],"%d",vet[i]);
	}
	sprintf(&vetS[i],"\0");
	fprintf(stdout,"%s\n", vetS);

	for(i=0; i<nChild; i++){
		wait(NULL);
	}
}