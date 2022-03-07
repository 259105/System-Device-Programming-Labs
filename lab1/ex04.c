#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){

	if(argc!=3){
		fprintf(stderr,"not enough parameters\n");
		exit(1);
	}
	
	setbuf(stdout,0);
	
	int h,n;
	h = atoi(argv[1]);
	n = atoi(argv[2]);
	
	for(int i=0; i<h; i++){
		for(int j=0; j<n-1; j++){ // n-1 because the father remains alive, so we create only n-1 processes
			if(!fork())
				break;
		}
	}
		
	fprintf(stdout, "My pid is %d\n", getpid());
	return 0;
}