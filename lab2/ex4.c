#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

void *threadFunct(void *);
int scanDirectory(char *,int (*op)(struct dirent *));
int printInfo(struct dirent *);

typedef struct{
    pthread_t tid;
    char *directory;
    int ret;
} ParThread;

int main(int argc, char *argv[]){
    if(argc == 1)
        return 0;
    if(mkdir("./.tempEx4",S_IRUSR|S_IWUSR|S_IXUSR)){
        fprintf(stderr,"Error creating temp dir\n");
        exit(1);
    }
    ParThread **parThreads = (ParThread **) malloc((argc-1)*sizeof(ParThread *));
    for(int i=0;i<argc-1;i++){
        parThreads[i] = (ParThread *) malloc(sizeof(ParThread));
        parThreads[i]->directory = argv[i+1];
        if(pthread_create(&(parThreads[i]->tid),NULL,threadFunct,parThreads[i])){
            fprintf(stderr,"Error creating thread: %d\n",i);
            exit(1);
        }
    }
    for(int i=0;i<argc-1;i++){
        pthread_join(parThreads[i]->tid,NULL);
        if(parThreads[i]->ret){
            fprintf(stderr,"Error generic with thread: %ld\n",parThreads[i]->tid);
        }
        char tempFile[20+16+1]; // pthread_t + other chars + \0
        sprintf(tempFile,"./.tempEx4/%ld.temp",parThreads[i]->tid);
        FILE *fp = fopen(tempFile,"r");
        if(fp == NULL){
            fprintf(stderr,"Error opening temp file: %s\n",tempFile);
            continue;
        }
        char nameDir[256+1]; //max name in linux
        while(fscanf(fp,"%s",nameDir)>0)
            fprintf(stdout,"%ld : %s\n",parThreads[i]->tid,nameDir);
        
        if(fclose(fp)){
            fprintf(stderr,"Error closing temp file: %s\n",tempFile);
        }
        if(remove(tempFile)){
            fprintf(stderr,"Error removing temp file: %s\n",tempFile);
        }
    }
    for(int i=0;i<argc-1;i++)
        free(parThreads[i]);
    free(parThreads);
    
    if(rmdir("./.tempEx4")){
        fprintf(stderr,"Error removing temp directory\n");
        exit(1);
    }
    return 0;
}

void *threadFunct(void * pars){
    ParThread *params = (ParThread *) pars;
    // remove the last / if is present
    if(params->directory[strlen(params->directory)-1]=='/')
        params->directory[strlen(params->directory)-1]='\0';

    params->ret = scanDirectory(params->directory,printInfo);
    pthread_exit(NULL);
}

int scanDirectory(char *dir,int (*op)(struct dirent *entry)){
    struct stat buf;
    // check if the path is really a directory
    if(lstat(dir,&buf)==-1){
        fprintf(stderr,"Error reading info about: %s\n",dir);
        return 1; 
    }
    if(!S_ISDIR(buf.st_mode)){
        fprintf(stderr,"The path %s is not a directory\n", dir);
        return 1;
    }
    // open dir
    DIR *dr = opendir(dir);
    if(dr == NULL){
        fprintf(stderr,"Error opening directory: %s\n",dir);
        return 1;
    }
    // loop in entry of dir
    struct dirent *entry;
    while((entry = readdir(dr)) != NULL){
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
            continue;
        // abs path of entry
        char *pathEntry = (char *) malloc((strlen(dir)+1+strlen(entry->d_name)+1)*sizeof(char));
        sprintf(pathEntry,"%s/%s",dir,entry->d_name);
        // check if it is a dir or file
        if(lstat(pathEntry,&buf)==-1){
            fprintf(stderr,"Error reading info about: %s\n",pathEntry);
            return 1; 
        }
        if(S_ISDIR(buf.st_mode)){
            if((*op)(entry))
                return 1;
            scanDirectory(pathEntry,(*op));
        }else if(S_ISREG(buf.st_mode)){
            //(*op)(entry);
        }
        free(pathEntry);
    }
    // close dir
    if(closedir(dr) == -1){
        fprintf(stderr,"Error closing dir: %s\n",dir);
        return 1;
    }
    return 0;
}

int printInfo(struct dirent *entry){

    char tempFile[20+16+1]; // pthread_t + other chars + \0
    sprintf(tempFile,"./.tempEx4/%ld.temp",pthread_self());

    FILE *fp = fopen(tempFile,"a");
    if(fp == NULL){
        fprintf(stderr,"Error creating temp file: %s\n",tempFile);
        return 1;
    }
    fprintf(fp,"%s\n",entry->d_name);
    if(fclose(fp)){
        fprintf(stderr,"Error closing file: %s",tempFile);
        return 1;
    }
    return 0;
}