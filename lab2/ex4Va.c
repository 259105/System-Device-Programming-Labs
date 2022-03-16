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
} ParThread;

int main(int argc, char *argv[]){
    if(argc == 1)
        return 0;
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
        long int ret;
        pthread_join(parThreads[i]->tid,(void **)&ret);
        if(ret==1){
            fprintf(stderr,"Error generic with thread: %d\n",i);
            exit(1);
        }
    }
    return 0;
}

void *threadFunct(void * pars){
    ParThread *params = (ParThread *) pars;
    int ret=0;
    // remove the last / if is present
    if(params->directory[strlen(params->directory)-1]=='/')
        params->directory[strlen(params->directory)-1]='\0';
    if( (ret=scanDirectory(params->directory,printInfo)) == 1)
        pthread_exit((void *)&ret);
    pthread_exit((void *)&ret);
}

int scanDirectory(char *dir,int (*op)(struct dirent *entry)){
    struct stat buf;
    if(lstat(dir,&buf)==-1){
        fprintf(stderr,"Error reading info about: %s\n",dir);
        return 1; 
    }
    if(!S_ISDIR(buf.st_mode)){
        fprintf(stderr,"The path %s is not a directory\n", dir);
        return 1;
    }
    DIR *dr = opendir(dir);
    if(dr == NULL){
        fprintf(stderr,"Error opening directory: %s\n",dir);
        return 1;
    }
    struct dirent *entry;
    while((entry = readdir(dr)) != NULL){
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
            continue;
        char *pathEntry = (char *) malloc((strlen(dir)+1+strlen(entry->d_name)+1)*sizeof(char));
        sprintf(pathEntry,"%s/%s",dir,entry->d_name);
        if(lstat(pathEntry,&buf)==-1){
            fprintf(stderr,"Error reading info about: %s\n",pathEntry);
            return 1; 
        }
        if(S_ISDIR(buf.st_mode)){
            (*op)(entry);
            scanDirectory(pathEntry,(*op));
        }else if(S_ISREG(buf.st_mode)){
            //(*op)(entry);
        }
        free(pathEntry);
    }
    if(closedir(dr) == -1){
        fprintf(stderr,"Error closing dir: %s\n",dir);
        return 1;
    }
    return 0;
}

int printInfo(struct dirent *entry){
    fprintf(stdout,"%ld : %s\n",pthread_self(),entry->d_name);
    return 0;
}