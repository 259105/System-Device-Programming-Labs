#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXCHAR 30

typedef struct{
	int id;
	long num;
	char name[MAXCHAR+1];
	char surname[MAXCHAR+1];
	int mark;
} Student;

int copyFileToBin(char *fileName, char *nameNewFile);

int copyBinToFile(char *fileName, char *nameNewFile);


int main(int argc,char *argv[]){
	if(argc != 4){
		fprintf(stderr,"Wrong parameters\n");
		exit(1);	
	}
	
	if(copyFileToBin(argv[1],argv[2]) == -1 ){
		fprintf(stderr, "Error writing on bin file\n");
		exit(1);
	}

	if(copyBinToFile(argv[2],argv[3]) == -1){
		fprintf(stderr,"copy file Error\n");
		exit(1);
	}

	return 0;
}

int copyFileToBin(char *fileName, char *nameNewFile){
	int fd = open(nameNewFile,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
	if (fd == -1) 
		return -1;

	FILE *fp = fopen(fileName,"r");
	if(fp == NULL){
		fprintf(stderr,"File Error\n");
		exit(1);
	}

	Student *currS = (Student *)malloc(sizeof(Student));
	if(currS == NULL)
		return -1;

	while( fscanf(fp,"%d %li %s %s %d",&currS->id, &currS->num, currS->name, currS->surname, &currS->mark) != -1)	{
		//fprintf(stdout, "%d %li %s %s %d\n", currS->id, currS->num, currS->name, currS->surname, currS->mark);
		if(write(fd,currS, sizeof(Student)) == -1)
			return -1;
	}
	
	fclose(fp);
	close(fd);
	return 0;
}

int copyBinToFile(char *fileName, char *nameNewFile){
	FILE *fp = fopen(nameNewFile, "w");
	if (fp == NULL)
		return -1;

	int fd = open(fileName,O_RDONLY);
	if (fd == -1) 
		return -1;
	
	Student *currS ;

	int byteRead=0;
	while( (byteRead = read(fd, currS, sizeof(Student)) ) > 0){
		//fprintf(stdout, "%d %li %s %s %d\n", currS->id, currS->num, currS->name, currS->surname, currS->mark);
		fprintf(fp, "%d %li %s %s %d\n", currS->id, currS->num, currS->name, currS->surname, currS->mark);
	}
	
	if(byteRead==-1)
		return -1;
	
	fclose(fp);
	close(fd);
	return 0;
}