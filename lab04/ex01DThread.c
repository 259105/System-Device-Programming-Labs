#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <aio.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define WAIT_TIME_1 1
#define WAIT_TIME_2 3
#define STR_SIZE 5
#define STR_NUM 3

typedef struct{
	int fd;
	int strSize;
	int n;
	char *str;
} thread_par;

pthread_mutex_t *mu;
pthread_mutex_t *muS;

void child1(int);
void child2(int);
void thread_getLenght(union sigval);
void thread_getString(union sigval);
void signalHandler(int);
void checkErr(int);

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
		
		//child2(p2[1]);
		
		close(p2[1]); // child2 close writing p2 at the 
		exit(0);
	}
	close(p2[1]); // father close writing p2
	
	// signal(SIGUSR1, signalHandler);
	// signal(SIGUSR2, signalHandler);
	
	struct aiocb **aio = (struct aiocb **)malloc(2*sizeof(struct aiocb *));
	thread_par **tpar = (thread_par **)malloc(2*sizeof(thread_par *));
	for(int i=0;i<2;i++){
		tpar[i] = (thread_par *)malloc(STR_NUM*sizeof(thread_par));
		aio[i] = (struct aiocb *)malloc(STR_NUM*sizeof(struct aiocb));
	}

	mu = (pthread_mutex_t *)malloc(2*sizeof(pthread_mutex_t));
	muS = (pthread_mutex_t *) malloc(2*sizeof(pthread_mutex_t));

	for(int i=0;i<2;i++){
		pthread_mutex_init(&mu[i],NULL);
		pthread_mutex_init(&muS[i],NULL);
		pthread_mutex_lock(&muS[i]);
	}

	int p[2]; p[0]=p1[0]; p[1]=p2[0]; // It's ugly, to be change

	for(int i=0;i<STR_NUM;i++){
		for(int k=0;k<1;k++){ // pipe1 and pipe2

			pthread_mutex_lock(&mu[k]);

			tpar[k][i].fd = p[k];
			tpar[k][i].n = k;
			
			aio[k][i].aio_fildes = tpar[k][i].fd;
			aio[k][i].aio_buf = &(tpar[k][i].strSize);
			aio[k][i].aio_nbytes = sizeof(int);
			aio[k][i].aio_sigevent.sigev_notify= SIGEV_THREAD;
			aio[k][i].aio_sigevent.sigev_value.sival_ptr = (void *) &tpar[k][i];
			aio[k][i].aio_sigevent.sigev_notify_function = thread_getLenght;

			//checkErr(aio_error(&aio[k][i]));

			if(aio_read(&aio[k][i]) == -1){
				fprintf(stderr,"Error %d aio_read strSize\n",errno);
			}

			//checkErr(aio_error(&aio[k][i]));
		}
	}
	for(int i=0;i<2;i++){
		pthread_mutex_destroy(&mu[i]);
		pthread_mutex_destroy(&muS[i]);
	}
	free(mu);
	free(muS);
	
	wait(0);
	wait(0);

	for(int i=0;i<2;i++){
		free(tpar[i]);
		free(aio[i]);
	}
	free(tpar);
	free(aio);
	
	close(p1[0]); // father close reading p1 at the end
	close(p2[0]); // father close reading p2 at the 
	return 0;
}

void thread_getLenght(union sigval sv){
		thread_par *tpar = (thread_par *) sv.sival_ptr;
		int strSize = tpar->strSize;
		int fd = tpar->fd;
		int n = tpar->n;
		fprintf(stdout,"ThreadChild n: %d; strSize: %d; fd: %d\n",n, strSize, fd);		

		struct aiocb *aio = (struct aiocb*) malloc(sizeof(struct aiocb));

		tpar->str = (char *) malloc((strSize+1)*sizeof(char));

		aio->aio_fildes = fd;
		aio->aio_buf = tpar->str;
		aio->aio_nbytes = (strSize+1)*sizeof(char);
		aio->aio_sigevent.sigev_notify = SIGEV_THREAD;
		aio->aio_sigevent.sigev_value.sival_ptr = (void *) tpar;
		aio->aio_sigevent.sigev_notify_function = thread_getString;

		if(aio_read(aio) == -1){
			fprintf(stderr,"Error %d aio_read str\n",errno);
			checkErr(aio_error(aio));
		}

		pthread_mutex_lock(&muS[n]);

		free(tpar->str);
		free(aio);

		pthread_mutex_unlock(&mu[n]);

		pthread_exit(0);
}

void thread_getString(union sigval sv){
	thread_par *tpar = (thread_par *)sv.sival_ptr;
	int n = tpar->n;
	char *str = tpar->str;

	for(int j=0;j<strlen(str);j++){
		str[j] = str[j] - 'a' + 'A';
	}
	fprintf(stdout,"Child:%d: %s\n",n+1,str);

	pthread_mutex_unlock(&muS[n]);

	pthread_exit(0);
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

void signalHandler(int signo){
	if(signo == SIGUSR1){
		fprintf(stdout,"Receved SIGUSR1\n");
	}else if(signo == SIGUSR2){
		fprintf(stdout,"Receved SIGUSR2\n");
	}
}

void checkErr(int err){
	//printf("In error checker\n");
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
