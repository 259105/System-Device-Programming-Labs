#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

extern int errno;

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr,"Error wrong paramiters: <fileDest> [elements, ...]\n");
        exit(-1);
    }

    int fd;
    if((fd = open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU)) == -1){
        perror("Error opening file");
        exit(-1);
    }
    int rt;
    int n=argc-2;
    if((rt=write(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
        perror("Error writing the file");
        close(fd);
        exit(-1);
    }

    int wrongNumbers=0;
    for(int i=2;i<argc;i++){
        n = atol(argv[i]);
        if(n==0 && argv[i][0] != 48){
            fprintf(stderr,"%d ",i-2);
            wrongNumbers++;
            n = argc-2-wrongNumbers;
            lseek(fd,0,SEEK_SET);
            if((rt=write(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
                perror("Error writing the file");
                close(fd);
                exit(-1);
            }
            lseek(fd,(i-wrongNumbers)*sizeof(int),SEEK_SET);
            continue;
        }
        if((rt=write(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
            perror("Error writing the file");
            close(fd);
            exit(-1);
        }
    }

    if(wrongNumbers>0){
        fprintf(stderr,"%s not a number\n",wrongNumbers == 1 ? "is":"are");
    }

    close(fd);
    fprintf(stdout,"TEST PRINT:\n");

    if((fd = open(argv[1],O_RDONLY)) == -1){
        perror("Error opening file for testing it");
        exit(-1);
    }
    int size;
    if((rt=read(fd,&size,sizeof(int))) == -1 || rt!=sizeof(int)){
        perror("Error reading the file");
        close(fd);
        exit(-1);
    }
    fprintf(stdout,"%d ",size);
    for(int i=0;i<size;i++){
        if((rt=read(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
            perror("Error reading the file");
            close(fd);
            exit(-1);
        }
        fprintf(stdout,"%d ",n);
    }
    fprintf(stdout,"\n");

    close(fd);

    return 0;
}