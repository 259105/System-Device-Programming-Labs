#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <aio.h>
#include <signal.h>
#include <errno.h>

#define WAIT_TIME_1 1
#define WAIT_TIME_2 3
#define STR_SIZE 5
#define STR_NUM 3

int strSize;
int n;
char *str;

void child1(int);
void child2(int);
void checkErr(int);

void signalHandler(int signo){
	if(signo == SIGUSR1){
		fprintf(stdout,"Receved SIGUSR1\n");
	}else if(signo == SIGUSR2){
		fprintf(stdout,"Receved SIGUSR2\n");

		for(int j=0;j<strlen(str);j++){
			str[j] = str[j] - 'a' + 'A';
		}
		fprintf(stdout,"Child:%d: %s\n",n+1,str);
	}
}

int main(int argc, char *argv[]){
	int p1[2],p2[2];

	if(signal(SIGUSR1, signalHandler) == SIG_ERR || signal(SIGUSR2, signalHandler) == SIG_ERR){
		fprintf(stderr, "Error creation signal\n");
		return 1;
	}
	
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
		
		child1(p1[1]);
		
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
		
		child2(p2[1]);
		
		close(p2[1]); // child2 close writing p2 at the 
		exit(0);
	}
	close(p2[1]); // father close writing p2
	
	struct aiocb aio;

	int p[2]; p[0]=p1[0]; p[1]=p2[0]; // It's ugly, to be change

	for(int i=0;i<STR_NUM;i++){
		for(int k=0;k<2;k++){ // pipe1 and pipe2
			
			aio.aio_fildes = p[k];
			aio.aio_offset = 0;
			aio.aio_buf = &(strSize);
			aio.aio_nbytes = sizeof(int);
			aio.aio_reqprio = 0;
			aio.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
			aio.aio_sigevent.sigev_signo = SIGUSR1;
			aio.aio_sigevent.sigev_value.sival_ptr = (void *) &aio;

			if(aio_read(&aio) == -1){
				fprintf(stderr,"Error %d aio_read\n",errno);
				checkErr(aio_error(&aio));
			}

			pause();

			str = (char *) malloc(strSize * sizeof(char));
			n=i;

			aio.aio_fildes = p[k];
			aio.aio_offset = 0;
			aio.aio_buf = str;
			aio.aio_nbytes = strSize*sizeof(char);
			aio.aio_reqprio = 0;
			aio.aio_sigevent.sigev_notify= SIGEV_SIGNAL;
			aio.aio_sigevent.sigev_signo = SIGUSR2;
			aio.aio_sigevent.sigev_value.sival_ptr = (void *) &aio;

			if(aio_read(&aio) == -1){
				fprintf(stderr,"Error aio_read\n");
				checkErr(aio_error(&aio));
			}

			pause();

			free(str);
		}
	}
	
	wait(0);
	wait(0);
	
	close(p1[0]); // father close reading p1 at the end
	close(p2[0]); // father close reading p2 at the 
	return 0;
}

void child1(int fd){
	// Generate STR_NUM strings and send them one at the time to the 
	unsigned int seed = WAIT_TIME_1;
	for(int i=0;i<STR_NUM;i++){
		sleep(WAIT_TIME_1);
		
		int strSize = (rand_r(&seed) % STR_SIZE) + 1;
		//fprintf(stdout,"child 1 writing the strSize: %d\n", strSize);
		if( write(fd,&strSize,sizeof(int)) != sizeof(int) ){
			fprintf(stderr,"Error writing strSize %d: %d on pipe1\n",i,strSize);
		}
		
		char *str = (char *) malloc((strSize+1)*sizeof(char));
		
		int j;
		for(j=0;j<strSize;j++)
			str[j] = (rand_r(&seed) % 26) + 'a';
		str[j] = '\0';
		
		fprintf(stdout,"child 1 has generated the string %d: %s\n",i,str);
		
		if(write(fd,str,(strSize+1)*sizeof(char))!=(strSize+1)*sizeof(char)){
			fprintf(stderr,"Error writing string %d: %s, on pipe1\n",i,str);
		}
		
		free(str);
	}
}

void child2(int fd){
	// Generate STR_NUM strings and send them one at the time to the 
	unsigned int seed = WAIT_TIME_2;
	for(int i=0;i<STR_NUM;i++){
		sleep(WAIT_TIME_2);
		
		int strSize = (rand_r(&seed) % STR_SIZE) + 1;
		//fprintf(stdout,"child 2 writing the strSize: %d\n", strSize);
		if( write(fd,&strSize,sizeof(int)) != sizeof(int) ){
			fprintf(stderr,"Error writing strSize %d: %d on pipe2\n",i,strSize);
		}
		
		char *str = (char *) malloc((strSize+1)*sizeof(char));
		
		int j;
		for(j=0;j<strSize;j++)
			str[j] = (rand_r(&seed) % 26) + 'a';
		str[j] = '\0';
		
		fprintf(stdout,"child 2 has generated the string %d: %s\n",i,str);
		
		if(write(fd,str,(strSize+1)*sizeof(char))!=(strSize+1)*sizeof(char)){
			fprintf(stderr,"Error writing string %d: %s, on pipe2\n",i,str);
		}
		
		free(str);
	}
}

void checkErr(int err){
	if( err == EINPROGRESS ){
		fprintf(stderr,"The request has not been completed yet\n");
	}else if(err == ECANCELED ){
		fprintf(stderr,"The request was canceled\n");
	}else if(err == 0){
		fprintf(stderr,"The request completed successfully\n");
	}else if(err == EINVAL){
		fprintf(stderr,"aiocbp does not point at a control block for an asynchronous I/O request of which the return status (see aio_return(3)) has not been retrieved yet\n");
	}else{
		fprintf(stderr,"Fallied error: %d\n",err);
	}
}
