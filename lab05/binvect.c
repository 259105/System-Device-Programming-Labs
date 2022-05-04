#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#define MAX 0x04000000

extern int errno;

int main(int argc, char *argv[]){
    if(argc < 1){
        fprintf(stderr,"Error wrong paramiters: <fileDest>\n");
        exit(-1);
    }

    int fd;
    if((fd = open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU)) == -1){
        perror("Error opening file");
        exit(-1);
    }
    int rt;
    int n=MAX;
    if((rt=write(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
        perror("Error writing the file");
        close(fd);
        exit(-1);
    }

    int wrongNumbers=0;
    unsigned int pid = getpid();
    for(int i=0;i<MAX;i++){
        n = rand_r(&pid) % MAX;
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
    /*
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
    */

    return 0;
}