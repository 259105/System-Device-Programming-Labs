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

int strSize1, strSize2;
int i1,i2, f1, f2;

void child1(int);
void child2(int);
void checkErr(int);

void signalHandler(int signo){
	if(signo == SIGUSR1){
		fprintf(stdout,"Receved SIGUSR1\n");
	}else if(signo == SIGUSR2){
		fprintf(stdout,"Receved SIGUSR2\n");
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
	
	struct aiocb *aio1, *aio2;
	aio1 = (struct aiocb *)malloc(sizeof(struct aiocb));
	aio2 = (struct aiocb *)malloc(sizeof(struct aiocb));

	aio1->aio_fildes = p1[0];
	aio1->aio_offset = 0;
	aio1->aio_buf = &strSize1;
	aio1->aio_nbytes = sizeof (int);
	aio1->aio_reqprio = 0;
	aio1->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	aio1->aio_sigevent.sigev_signo = SIGUSR1;

	aio2->aio_fildes = p2[0];
	aio2->aio_offset = 0;
	aio2->aio_buf = &strSize2;
	aio2->aio_nbytes = sizeof (int);
	aio2->aio_reqprio = 0;
	aio2->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	aio2->aio_sigevent.sigev_signo = SIGUSR2;

	i1 = i2 = f1 = f2= 0;
	while(i1<STR_NUM || i2<STR_NUM){
		if (i1<STR_NUM && f1 == 0) {

            if(aio_read(aio1) == -1){
				fprintf(stderr,"Error %d aio_read\n",errno);
				checkErr(aio_error(aio1));
			}

            f1 = 1;
        }
        if (i2<STR_NUM && f2 == 0) {

            if(aio_read(aio2) == -1){
				fprintf(stderr,"Error %d aio_read\n",errno);
				checkErr(aio_error(aio2));
			}

            f2 = 1;
        }

		pause();
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
		char *str = (char *) malloc((strSize+1)*sizeof(char));
		
		int j;
		for(j=0;j<strSize;j++)
			str[j] = (rand_r(&seed) % 26) + 'a';
		str[j] = '\0';
		
		fprintf(stdout,"child 1 has generated the string %d: %s\n",i,str);
		
		if( write(fd,&strSize,sizeof(int)) != sizeof(int) ){
			fprintf(stderr,"Error writing strSize %d: %d on pipe1\n",i,strSize);
		}
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
		char *str = (char *) malloc((strSize+1)*sizeof(char));
		
		int j;
		for(j=0;j<strSize;j++)
			str[j] = (rand_r(&seed) % 26) + 'a';
		str[j] = '\0';
		
		fprintf(stdout,"child 2 has generated the string %d: %s\n",i,str);
		
		if( write(fd,&strSize,sizeof(int)) != sizeof(int) ){
			fprintf(stderr,"Error writing strSize %d: %d on pipe2\n",i,strSize);
		}
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
