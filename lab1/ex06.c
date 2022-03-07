#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter=0;
int isEnd=0;

void signalManager(int sig){
	if(sig==SIGUSR1){
		if(counter<0){
			counter=1;
			fprintf(stderr, "success\n");
		}else
			counter++;
	}else if(sig==SIGUSR2){
		if(counter>0){
			counter=-1;
			fprintf(stderr, "success\n");
		}else
			counter--;		
	}

	if(counter==2 || counter==-2)
		fprintf(stderr,"error\n");
	else if(counter==3 || counter==-3){
		fprintf(stderr,"terminate\n");
		isEnd = 1;
	}
}

int main(int argc, char *argv[]){
	signal(SIGUSR1,signalManager);
	signal(SIGUSR2,signalManager);
	
	while(1){
		pause();
		if(isEnd)
			exit(0);
	}
}