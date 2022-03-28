#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAXCHAR 30

int lock_region(int fd, int cmd, short type, short whence, off_t start, off_t len);

typedef struct{
	int id;
	long num;
	char name[MAXCHAR+1];
	char surname[MAXCHAR+1];
	int mark;
} Student;

int lock_region(int fd, int cmd, short type, short whence, off_t start, off_t len){
	struct flock flockS;
	flockS.l_type = type;
	flockS.l_whence = whence;
	flockS.l_start = start;
	flockS.l_len = len;
	return fcntl(fd,cmd,&flockS);
}

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr,"Not enough paramiters\n");
		exit(1);
	}
	int fd = open(argv[1],O_RDWR);
	char cmd;
	int n;
	int rb;
	Student *s = (Student *) malloc(sizeof(Student));

	if (fd == -1){
		fprintf(stderr,"Error open binary file\n");
		exit(1);
	}
	
	while(1){
		fprintf(stdout, "Insert command:\n");
		if(fscanf(stdin,"%c %d%*c",&cmd, &n)<=0){
			fprintf(stderr,"Error: wrong option\n");
			continue;
		}
		if(n == 0) continue;
		lseek(fd,(n-1)*sizeof(Student),SEEK_SET);
		switch(cmd){
		case 'R':
			lock_region(fd,F_SETLKW,F_RDLCK,SEEK_SET,(n-1)*sizeof(Student),sizeof(Student));
			rb = read(fd,s,sizeof(Student));
			lock_region(fd,F_SETLKW,F_UNLCK,SEEK_SET,(n-1)*sizeof(Student),sizeof(Student));
			if(rb != sizeof(Student) || s->id==0 ){
				fprintf(stderr,"Error reading the binary file or student doesn't exist\n");
				break;
			}
			fprintf(stdout,"%d %li %s %s %d\n",s->id,s->num,s->name,s->surname,s->mark);
			break;
		case 'W':
			fprintf(stdout,"Insert data: <matricola> <name> <surname> <mark>\n");
			s->id = n;
			fscanf(stdin,"%li %s %s %d%*c",&s->num,s->name,s->surname,&s->mark);
			lock_region(fd,F_SETLKW,F_WRLCK,SEEK_SET,(n-1)*sizeof(Student),sizeof(Student));
			rb = write(fd,s,sizeof(Student));
			lock_region(fd,F_SETLKW,F_UNLCK,SEEK_SET,(n-1)*sizeof(Student),sizeof(Student));
			if(rb != sizeof(Student) ){
				fprintf(stderr,"Error writing in binary file: %d\n",rb);
				exit(1);
			}
			break;
		case 'E':
			return 0;
		}	
	}
}