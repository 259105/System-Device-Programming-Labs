#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>

#define WAIT_TIME_1 1
#define WAIT_TIME_2 3
#define STR_SIZE 5
#define STR_NUM 3

void set_fcntl(int, int);
void clr_fcntl(int, int);

void set_fcntl(int fd, int flag){
	int val;
	if((val=fcntl(fd, F_GETFL, 0))==-1){
		fprintf(stderr,"Error getting flags of file descriptor %d",fd);
		return;
	}
	
	val |= flag;
	
	if(fcntl(fd, F_SETFL, val)){
		fprintf(stderr,"Error setting flags of file descriptor %d",fd);
		return;
	}
}

void clr_fcntl(int fd, int flag){
	int val;
	if((val=fcntl(fd, F_GETFL, 0))==-1){
		fprintf(stderr,"Error getting flags of file descriptor %d",fd);
		return;
	}
	
	val &= ~flag;
	
	if(fcntl(fd, F_SETFL, val)){
		fprintf(stderr,"Error setting flags of file descriptor %d",fd);
		return;
	}
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
		close(p1[1]); // child2 close writing p1
		
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
	
	set_fcntl(p1[0],O_NONBLOCK);
	set_fcntl(p2[0],O_NONBLOCK);
	
	int strSize1, strSize2;
	int k=0;
	int n=0, m=0;
	char *str1=NULL, *str2=NULL;
	
	while(k<STR_NUM*2){
	
		if(str1==NULL && read(p1[0],&strSize1,sizeof(int)) == sizeof(int)){
			str1 = (char *) malloc((strSize1+1)*sizeof(char));
		}
		
		if(str1!=NULL && read(p1[0],str1,(strSize1+1)*sizeof(char))==(strSize1+1)*sizeof(char)){
			for(int j=0;j<strlen(str1);j++){
				str1[j] = str1[j] - 'a' + 'A';
			}
			fprintf(stdout,"Child 1:%d: %s\n",n,str1);
			free(str1);
			str1=NULL;
			n++;
			k++;
		}
		
		if(str2==NULL && read(p2[0],&strSize2,sizeof(int)) == sizeof(int)){
			str2 = (char *) malloc((strSize2+1)*sizeof(char));
		}
		if(read(p2[0],str2,(strSize2+1)*sizeof(char))==(strSize2+1)*sizeof(char)){
			for(int j=0;j<strlen(str2);j++){
				str2[j] = str2[j] - 'a' + 'A';
			}
			fprintf(stdout,"Child 2:%d: %s\n",m,str2);
			free(str2);
			str2=NULL;
			m++;
			k++;
		}
				
	}
	
	wait(0);
	wait(0);
	
	close(p1[0]); // father close reading p1 at the end
	close(p2[0]); // father close reading p2 at the 
	return 0;
}
