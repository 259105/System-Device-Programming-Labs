#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>

extern int errno;
char err[64];

void* th_reader(void*);
void* th_comparer(void*);

typedef struct{
    pthread_mutex_t *meR;
    sem_t *meC;
    sem_t *barrier;
    int count_thr,bar_thr,N;
    int end;
} pthread_synch;

typedef struct {
    pthread_t tid;
    char *path;
    char (*names)[NAME_MAX];
    int id;
    int deep;
    pthread_synch *synch;
} pthread_par;

int main(int argc, char **argv){
    if(argc < 3){
        fprintf(stderr,"Wrong paramiters: <dir1> ... <dirn>\n");
        exit(-1);
    }

    setbuf(stdout,0);

    struct stat sb;
    pthread_par *thrPar = (pthread_par *)malloc((argc)*sizeof(pthread_par));
    char (*names)[NAME_MAX] = (char (*)[NAME_MAX])malloc((argc-1)*sizeof(char[NAME_MAX]));
    pthread_synch *synch = (pthread_synch *)malloc(sizeof(pthread_synch));
    synch->meC = (sem_t *)malloc(sizeof(sem_t));
    synch->meR = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    synch->barrier = (sem_t *)malloc(sizeof(sem_t));
    sem_init(synch->meC,0,0);
    pthread_mutex_init(synch->meR,NULL);
    sem_init(synch->barrier,0,0);
    synch->count_thr = 0;
    synch->bar_thr = argc-1;
    synch->end = 0;
    synch->N = argc-1;

    for(int i=1;i<argc;i++){
        if(lstat(argv[i],&sb) == -1){
            sprintf(err,"Error opening lstat on paramiter %d",i);
            perror(err);
            exit(-1);
        }
        if(!S_ISDIR(sb.st_mode)){
            sprintf(err,"The paramiter %d is not a dir",i);
            perror(err);
            exit(-1);
        }
        thrPar[i].path = argv[i];
        thrPar[i].names = names;
        thrPar[i].synch = synch;
        thrPar[i].id = i-1;
        thrPar[i].deep = 0;
        pthread_create(&(thrPar[i].tid),NULL,th_reader,(void*)&(thrPar[i]));
    }
    thrPar[0].names = names;
    thrPar[0].synch = synch;
    pthread_create(&(thrPar[0].tid),NULL,th_comparer,(void*)&(thrPar[0]));

    for(int i=1;i<argc;i++){
        pthread_join(thrPar[i].tid,0);
    }
    
    if(synch->bar_thr==0){
        printf("They are equal\n");
    }else if(synch->bar_thr<synch->N){
        printf("They are semi-equal\n");
    }else if(synch->bar_thr==synch->N){
        printf("They are different\n");
    }
    pthread_join(thrPar[0].tid,0);

    free(names);
    sem_destroy(synch->meC);
    pthread_mutex_destroy(synch->meR);
    sem_destroy(synch->barrier);
    free(synch->meC);
    free(synch->meR);
    free(synch->barrier);
    free(synch);
    free(thrPar);
    return 0;
}

void* th_reader(void* vpar){
    pthread_par *pars = (pthread_par *) vpar;

    DIR *dp;
    if((dp = opendir(pars->path)) == NULL){
        sprintf(err,"Error opening directory %s",pars->path);
        perror(err);
        exit(-1);
    }

    if(pars->path[strlen(pars->path)-1] == '/'){    // remove the last /
        pars->path[strlen(pars->path)-1] = '\0';
    }

    struct dirent *dirent;
    struct stat sb;
    errno = 0;  // to validate the error on readdir
    while((dirent = readdir(dp)) != NULL){
        if(strcmp(dirent->d_name,".")==0 ||
            strcmp(dirent->d_name,"..")==0)
            continue;

        strcpy(pars->names[pars->id],dirent->d_name);

        #ifdef DEB
            pthread_mutex_lock(pars->synch->meR);
                printf("%s : %d\n",pars->names[pars->id],pars->id);
            pthread_mutex_unlock(pars->synch->meR);
        #endif
        // Barrier
        sem_post(pars->synch->meC);
        sem_wait(pars->synch->barrier);
        if(pars->synch->end)
            break;

        char *currPath = (char *)malloc((strlen(pars->path)+1+strlen(pars->names[pars->id])+1)*sizeof(char));
        sprintf(currPath,"%s/%s",pars->path,(char *)pars->names[pars->id]);

        strcpy(pars->names[pars->id],"\0");

        if(lstat(currPath,&sb) == -1){
            sprintf(err,"Error opening lstat on file %s",currPath);
            perror(err);
            exit(-1);
        }

        if(S_ISDIR(sb.st_mode)){
            pthread_par subpar;
            subpar.names = pars->names;
            subpar.path = currPath;
            subpar.synch = pars->synch;
            subpar.tid = pars->tid;
            subpar.id = pars->id;
            subpar.deep = pars->deep+1;
            th_reader((void*)&subpar);
        }
        
        free(currPath);
        errno = 0;  // to validate the error on readdir
    }
    if(dirent == NULL && errno !=0){
        sprintf(err,"Error entry dir %s", pars->path);
        perror(err);
        exit(-1);
    }
    closedir(dp);

    if(!pars->synch->end && pars->deep==0){
        pthread_mutex_lock(pars->synch->meR);
            printf("A\n");                  // WTF!!
            pars->synch->bar_thr--;
            sem_post(pars->synch->meC);
        pthread_mutex_unlock(pars->synch->meR);
    }

    return NULL;
}

void* th_comparer(void* vpar){
    pthread_par *pars = (pthread_par *) vpar;
    while(1){
        for(int i=0;i<pars->synch->bar_thr;i++){
            sem_wait(pars->synch->meC);
        }
        int diff=0;

        if(pars->synch->bar_thr==0){
            pars->synch->end=1;
            break;
        }else if(pars->synch->bar_thr<pars->synch->N){
            pars->synch->end=1;
        }else {
            for(int i=0;i<pars->synch->bar_thr-1;i++){
                #ifdef DEB
                    printf("%s\n",pars->names[i]);
                    printf("%s\n\n",pars->names[i+1]);
                #endif
                if(strcmp(pars->names[i],pars->names[i+1]) !=0){
                    diff++;
                }
            }
        }
        if(diff>0)
            pars->synch->end=1;
        
        for(int i=0;i<pars->synch->bar_thr;i++){
            sem_post(pars->synch->barrier);
        }
        
        if(diff >0 || pars->synch->bar_thr<pars->synch->N)
            break;
    }
    return NULL;
}
