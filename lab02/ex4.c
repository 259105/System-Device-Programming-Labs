#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>

void *threadFunct(void *);
int scanDirectory(char *, FILE *, int (*op)(struct dirent *, FILE *));
int printInfo(struct dirent *, FILE *);
void *outputThreadFunct(void *);

typedef struct{
    pthread_t tid;
    char *directory;
    int ret;
} ParThread;

sem_t outputSem, scanSem;
pthread_mutex_t muO = PTHREAD_MUTEX_INITIALIZER;
pthread_t currOut;
int nThr = 0;

int main(int argc, char *argv[]){
    if(argc == 1)
        return 0;
    if(mkdir("./.tempEx4",S_IRUSR|S_IWUSR|S_IXUSR)){
        fprintf(stderr,"Error creating temp dir\n");
        exit(1);
    }
    
    ParThread **parThreads = (ParThread **) malloc((argc-1)*sizeof(ParThread *));
    sem_init(&outputSem,0,0);
    sem_init(&scanSem,0,0);
    nThr = argc-1;

    for(int i=0;i<argc-1;i++){
        parThreads[i] = (ParThread *) malloc(sizeof(ParThread));
        parThreads[i]->directory = argv[i+1];
        if(pthread_create(&(parThreads[i]->tid),NULL,threadFunct,(void *)parThreads[i])){
            fprintf(stderr,"Error creating thread: %d\n",i);
            exit(1);
        }
    }

    pthread_t outputThreadTid;
    pthread_create(&outputThreadTid,NULL,outputThreadFunct,NULL);
    pthread_join(outputThreadTid,NULL);

    sem_destroy(&outputSem);
    sem_destroy(&scanSem);

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

    pthread_detach(pthread_self()); /* the thread don't need to be joined */

    // remove the last / if is present
    if(params->directory[strlen(params->directory)-1]=='/')
        params->directory[strlen(params->directory)-1]='\0';

    /* Open file */
    char tempFile[20+16+1]; // pthread_t + other chars + \0
    sprintf(tempFile,"./.tempEx4/%ld.temp",pthread_self());

    FILE *fp = fopen(tempFile,"a");
    if(fp == NULL){
        fprintf(stderr,"Error creating temp file: %s\n",tempFile);
        nThr--;
        pthread_exit(NULL);
    }

    params->ret = scanDirectory(params->directory,fp,printInfo);

    /* Close file */
    if(fclose(fp)){
        fprintf(stderr,"Error closing file: %s",tempFile);
        nThr--;
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&muO);   // enter in a CS because currOut can be accessed
        if(params->ret){
            nThr--;
            fprintf(stderr,"Error generic with thread: %ld\n",pthread_self());
            pthread_mutex_unlock(&muO);
            pthread_exit(NULL);
        }
        currOut = pthread_self();   // send to outputThread the tid
        sem_post(&outputSem);   // signal output thread
        sem_wait(&scanSem);     // wait the end of print
    pthread_mutex_unlock(&muO); // exit of the CS

    pthread_exit(NULL);
}

int scanDirectory(char *dir,FILE *fp,int (*op)(struct dirent *,FILE *)){
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
            if((*op)(entry, fp))
                return 1;
            scanDirectory(pathEntry, fp, (*op));
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

int printInfo(struct dirent *entry, FILE *fp){

    fprintf(fp,"%s\n",entry->d_name);

    return 0;
}

void* outputThreadFunct(void *pars){
    while(nThr){
        sem_wait(&outputSem);
            char tempFile[20+16+1]; // pthread_t + other chars + \0
            sprintf(tempFile,"./.tempEx4/%ld.temp",currOut);
            FILE *fp = fopen(tempFile,"r");
            if(fp == NULL){
                fprintf(stderr,"Error opening temp file: %s\n",tempFile);
                continue;
            }
            char nameDir[256+1]; //max name in linux
            while(fscanf(fp,"%s",nameDir)>0)
                fprintf(stdout,"%ld : %s\n",currOut,nameDir);
            
            if(fclose(fp)){
                fprintf(stderr,"Error closing temp file: %s\n",tempFile);
            }
            if(remove(tempFile)){
                fprintf(stderr,"Error removing temp file: %s\n",tempFile);
            }
        nThr--;
        sem_post(&scanSem);
    }
    pthread_exit(NULL);
}
