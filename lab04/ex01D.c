#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <aiocb.h>
#include <pthread.h>
#include <signal.h>

#define WAIT_TIME_1 1
#define WAIT_TIME_2 3
#define STR_SIZE 5
#define STR_NUM 3

typedef struct{
	int fd;
	int strSize;
} thread_par;

void thread_getLenght(union sigval sv){
		threadp_par *tpar = (tpar *) sv.sival_ptr;
		int strSize = tpar->strSize;
		int fd = tpar->fd;		
		
		char *str = (char *) malloc((strSize+1)*sizeof(char));
		if(read(fd,str,(strSize+1)*sizeof(char))!=(strSize+1)*sizeof(char)){
			fprintf(stderr,"Error reading string %i, on pipe1\n",i);
		}else{
			for(int j=0;j<strlen(str);j++){
				str[j] = str[j] - 'a' + 'A';
			}
			fprintf(stdout,"Child 1:%d: %s\n",i,str);
		}
		free(str);
}

int main(int argc, char *argv[]){
	int p1[2],p2[2];
	
	if(pipe(p1)==-1){
		fprintf(stderr,"Error creating pipe 1\n");
		return 1;
	}
	
	pid_t pid1;
	if((pid1=fork())==-1){
		fprintf(stderr,"Error forking process for child 1\n");
		return 1;
	}else if(pid1==0){
		// CHILD 1 write things
		close(p1[0]); // child1 close reading p1
		
		// Generate STR_NUM strings and send them one at the time to the 
		unsigned int seed = WAIT_TIME_1;
		for(int i=0;i<STR_NUM;i++){
			sleep(WAIT_TIME_1);
			
			int strSize = (rand_r(&seed) % STR_SIZE) + 1;
			if( write(p1[1],&strSize,sizeof(int)) != sizeof(int) ){
				fprintf(stderr,"Error writing strSize %d: %d on pipe1\n",i,strSize);
			}
			
			char *str = (char *) malloc((strSize+1)*sizeof(char));
			
			int j;
			for(j=0;j<strSize;j++)
				str[j] = (rand_r(&seed) % 26) + 'a';
			str[j] = '\0';
			
			fprintf(stdout,"child 1 has generated the string %d: %s\n",i,str);
			
			if(write(p1[1],str,(strSize+1)*sizeof(char))!=(strSize+1)*sizeof(char)){
				fprintf(stderr,"Error writing string %d: %s, on pipe1\n",i,str);
			}
			
			free(str);
		}
		
		close(p1[1]); // child1 close writing p1 at the end
		exit(0);
	}
	close(p1[1]); // father close writing p1
	
	if(pipe(p2)==-1){
		fprintf(stderr,"Error creating pipe 2\n");
		return 1;
	}
	
	pid_t pid2;
	if((pid2=fork())==-1){
		fprintf(stderr,"Error forking process for child 2\n");
		return 1;
	}else if(pid2==0){
		// CHILD 2 write other things
		close(p2[0]); // child2 close reading p2
		close(p1[0]); // child2 close reading p1
		
		// Generate STR_NUM strings and send them one at the time to the 
		unsigned int seed = WAIT_TIME_2;
		for(int i=0;i<STR_NUM;i++){
			sleep(WAIT_TIME_2);
			
			int strSize = (rand_r(&seed) % STR_SIZE) + 1;
			if( write(p2[1],&strSize,sizeof(int)) != sizeof(int) ){
				fprintf(stderr,"Error writing strSize %d: %d on pipe2\n",i,strSize);
			}
			
			char *str = (char *) malloc((strSize+1)*sizeof(char));
			
			int j;
			for(j=0;j<strSize;j++)
				str[j] = (rand_r(&seed) % 26) + 'a';
			str[j] = '\0';
			
			fprintf(stdout,"child 2 has generated the string %d: %s\n",i,str);
			
			if(write(p2[1],str,(strSize+1)*sizeof(char))!=(strSize+1)*sizeof(char)){
				fprintf(stderr,"Error writing string %d: %s, on pipe2\n",i,str);
			}
			
			free(str);
		}
		
		close(p2[1]); // child2 close writing p2 at the 
		exit(0);
	}
	close(p2[1]); // father close writing p2
	
	
	
	union sigval sval; // no array because parameter by value
	struct sigevent sev; // no array because parameter by value
	struct aiocb **aio = (struct aiocb **)malloc(2*sizeod(struct aiocb *));
	thread_par **tpar = (threapd_par **)malloc(2*sizeof(thread_par *));
	for(int i=0;i<2;i++){
		tpar[i] = (thread_par *)malloc(STR_NUM*sizeof(thread_par));
		aio[i] = (struct aiocb *)malloc(STR_NUM*sizeof(struct aiocb));
	}

	for(int i=0;i<STR_NUM;i++){
	
		tpar[0][i].fd = p1[0];
		
		sval.sival_ptr = (void *) &tpar[0][i];
	
		sev.sigev_notify = SIGEV_THREAD;
		sev.sigev_value = sval;
		sev.sigev_notify_function = thread_getLenght;
		
		aio[0][i].aio_fildes = tpar[0][i].fd;
		aio[0][i].aio_buf = &(tpar[0][i].strSize);
		aio[0][i].aio_nbytes = sizeof(int);
		aio[0][i].aio_sigevent = sev;
		
		aio_read(&aio[0][i]);
		
		tpar[1][i].fd = p1[1];
		
		sval.sival_ptr = (void *) &tpar[1][i];
	
		sev.sigev_notify = SIGEV_THREAD;
		sev.sigev_value = sval;
		sev.sigev_notify_function = thread_getLenght;
		
		aio[1][i].aio_fildes = tpar[1][i].fd;
		aio[1][i].aio_buf = &(tpar[1][i].strSize);
		aio[1][i].aio_nbytes = sizeof(int);
		aio[1][i].aio_sigevent = sev;
		
		aio_read(&aio[1][i]);
		
	}
	
//	for(int i=0;i<2;i++){
//		free(tpar[i]);
//		free(aio[i]);
//	}
//	free(tpar);
//	free(aio);
	
	wait(0);
	wait(0);
	
	close(p1[0]); // father close reading p1 at the end
	close(p2[0]); // father close reading p2 at the 
	return 0;
}
