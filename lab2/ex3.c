#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BLOCKSIZE 4096
#define MAXCHAR 1024 // incluse /0

int copyFile(char *,char *);
int wrapCopyFile(void *);
int scanDirCopy(char *absPathDirSrc, char *absPathDirDst, int (*operation)(void *pars) );

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Error not enough parameters\n");
        exit(1);
    }
    char src[MAXCHAR],dst[MAXCHAR];
    sprintf(src,"%s/%s",getcwd(src,MAXCHAR),argv[1]);
    sprintf(dst,"%s/%s",getcwd(dst,MAXCHAR),argv[2]);
    if(scanDirCopy(src,dst,wrapCopyFile))
        exit(1);
    return 0;
}

int copyFile(char *source, char *destination){
    int fds = open(source, O_RDONLY);
    int fdd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fds < 0){
        fprintf(stderr,"Error opening source file: %s\n",destination);
        return fds;
    }
    if(fdd < 0){
        fprintf(stderr,"Error opening dest file: %s\n",destination);
        return fdd;
    }

    struct stat sourceStats;
    if(fstat(fds,&sourceStats) == -1){
        fprintf(stderr, "Error retrieving stats of: %s\n", source);
        return -1;
    }
    FILE *fp = fopen(destination, "w");
    if (fp == NULL){
        fprintf(stderr, "Error opening dest file : %s\n",destination);
    }
    char *name;
    for(int i=0;source[i]!='\0';i++)
        if(source[i]=='/') name = &source[i+1];
    char addingFields[MAXCHAR];
    sprintf(addingFields,"%s %ld\n",name,sourceStats.st_size);
    fprintf(fp,"%s",addingFields);
    fclose(fp);
    lseek(fdd,sizeof(addingFields),SEEK_SET);

    int rb,wb;
    char buf[4096];

    while((rb = read(fds,buf,BLOCKSIZE)) > 0){
        wb = write(fdd,buf,rb);
        if(rb != wb){
            fprintf(stderr,"File coping Error: %s\n", source);
            return -1;
        }
    }
    if(rb == -1){
        fprintf(stderr,"Error reading src file: %s\n",source);
        return -1;
    }
    if(close(fds) < 0) {
        fprintf(stderr, "Error closing src file: %s\n",source);
        return -1;
    }
    if(close(fdd) < 0){
        fprintf(stderr, "Error closing dest file: %s\n",destination);
        return -1;
    }
    return 1;
}

int wrapCopyFile(void *pars){
    char **params = (char **) pars;
    return copyFile(params[0],params[1]);
}

int scanDirCopy(char *absPathDirSrc, char *absPathDirDst, int (*operation)(void *pars) ){
    DIR *dp  = opendir(absPathDirSrc);
    if(dp == NULL){
        fprintf(stderr,"Error opening directory: %s\n",absPathDirSrc);
        return -1;
    }
    char *nameDirDst;
    for(int i=0;absPathDirSrc[i]!='\0';i++)
        if(absPathDirSrc[i]=='/') nameDirDst = &absPathDirSrc[i+1];
    //fprintf(stdout,"Name new dir: %s\n",nameDirDst);
    char pathNewDir[MAXCHAR];
    sprintf(pathNewDir,"%s/%s/",absPathDirDst,nameDirDst);
    if(mkdir(pathNewDir,S_IRUSR | S_IWUSR) <0){
        fprintf(stderr, "Error creating sub directory: %s\n", pathNewDir);
        return -1;
    }
    //fprintf(stdout,"Created directory: %s\n",pathNewDir);
    struct dirent *entry;
    struct stat entryStat;
    char absPathEntry[MAXCHAR];
    char absPathFileDst[MAXCHAR];
    while( (entry = readdir(dp)) != NULL){
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
            continue;
        sprintf(absPathEntry, "%s/%s", absPathDirSrc, entry->d_name);
        sprintf(absPathFileDst, "%s/%s",pathNewDir,entry->d_name);
        if(lstat(absPathEntry,&entryStat) < 0 ){
            fprintf(stderr, "Error retrieving stats of: %s\n", absPathEntry);
            return -1;
        }
        if(S_ISREG(entryStat.st_mode)){
            // is a file
            char **pars = (char **)malloc(2*sizeof(char *));
            pars[0]=absPathEntry;
            pars[1]=absPathFileDst;
            //fprintf(stdout,"Coping from %s to %s\n",absPathEntry,absPathFileDst);
            (*operation)((void *) pars);
        }else if(S_ISDIR(entryStat.st_mode)){
            // is a directory
            scanDirCopy(absPathEntry,pathNewDir,(*operation));
        }
    }
    if(closedir(dp) == -1){
        fprintf(stderr, "Error closing directory: %s\n", absPathDirSrc);
        return -1;
    }
    return 1;
}