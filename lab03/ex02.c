#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>


#define MAXCHAR 30
#define MAXMMAP (1024*1024*1024) // 1GB

typedef struct{
	int id;
	long num;
	char name[MAXCHAR+1];
	char surname[MAXCHAR+1];
	int mark;
} Student;

typedef struct{
    pthread_t tid;
    void *mem;
    int n;
} pthread_argv;

void *thread_changeRegNum(void *);
void *thread_changeMark(void *);

int main (int argc, char *argv[]){
    int fds, fdd;
    struct stat sbs;
    off_t currOff = 0;
    size_t copyLen;
    void *srcAddr, *dstAddr;

    // Check paramites
    if(argc != 3){
        fprintf(stderr,"Error: wrong paramiters");
        exit(1);
    }

    // Opening files
    if((fds = open(argv[1],O_RDONLY)) < 0){
        fprintf(stderr,"Error: open file %s\n",argv[1]);
        exit(1);
    }
    if((fdd = open(argv[2],O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IXUSR)) < 0){
        fprintf(stderr,"Error: open file %s\n",argv[2]);
        exit(1);
    }

    // Get the dim of a file input
    if(fstat(fds,&sbs) < 0){
        fprintf(stderr,"Error: get stats of file in\n");
        exit(1);
    }

    // Truncate the file out in the same dim of file in
    if(ftruncate(fdd,sbs.st_size) < 0){
        fprintf(stderr,"Error: truncate file out\n");
        exit(1);
    }

    // In case of big files we have to divide the process in chunks of MAXMMAP or less
    while(currOff < sbs.st_size){
        // WARNING !!! currOff must be a multiple of the page size sysconf(_SC_PAGE_SIZE), constaint of mmap()
        
        // take only chunks of MAXMMAP Byte
        // WARNING !!! MUST BE A MULTIPLE OF Student Struct FOR THIS EXERCISE !!
        if((sbs.st_size - currOff) > MAXMMAP){
            copyLen = MAXMMAP;
        }else{
            copyLen = (sbs.st_size - currOff);
        }
        copyLen -= copyLen%sizeof(Student); // take a multiple

        // debugging stuff
        fprintf(stdout,"Dimension: %ld\nRecords: %ld\nResidual: %ld\nCurrent Offset: %ld\nPageSize: %ld\nSizeStudent: %ld\n",copyLen,copyLen/sizeof(Student),copyLen%sizeof(Student),currOff,sysconf(_SC_PAGE_SIZE),sizeof(Student));

        // map the files
        if((srcAddr = mmap(0, copyLen, PROT_READ, MAP_SHARED, fds, currOff)) == MAP_FAILED){
            fprintf(stderr, "Error %d: map input file failed\n",errno);
            break;
        }
        if((dstAddr = mmap(0, copyLen, PROT_READ|PROT_WRITE, MAP_SHARED, fdd, currOff)) == MAP_FAILED){
            fprintf(stderr, "Error %d: map output file failed\n",errno);
            break;
        }

        // copy the bytes in dest mem
        memcpy(dstAddr, srcAddr, copyLen);

        // How to access to the memory by the addess of mmap;
        printf("\n");
        Student *si = dstAddr;  // by index
        Student *sp = dstAddr;  // by ponter
        for(int i=0;i<copyLen/sizeof(Student);i++){
            fprintf(stdout,"Student index %d: %d %ld %s %s %d\n",i,si[i].id,si[i].num,si[i].name,si[i].surname,si[i].mark);

            fprintf(stdout,"Student pointer %d: %d %ld %s %s %d\n",i,sp->id,sp->num,sp->name,sp->surname,sp->mark);

            fprintf(stdout,"Pointer: \t%p\nIndex: \t\t%p\nDifference: \t%ld\n",sp,&si[i],sp-&si[i]);

            printf("\n");

            sp++; // increment by one student (that is in bytes sizeof(Student))
        }

        // // throw threads to manage dstAddr and change the values
        pthread_argv *targv = (pthread_argv *)malloc(2 * sizeof(pthread_argv));
        targv[0].mem = dstAddr;     // first student
        targv[1].mem = dstAddr + copyLen - sizeof(Student); // last student
        targv[0].n = targv[1].n = copyLen/sizeof(Student);  // number of students in the chunk
        pthread_create(&(targv[0].tid),0,thread_changeRegNum,(void *) &targv[0]);
        pthread_create(&(targv[1].tid),0,thread_changeMark,(void *) &targv[1]);

        for(int i=0;i<2;i++){
            pthread_join(targv[i].tid,0);
        }

        // unmap the files
        munmap(srcAddr,copyLen);
        munmap(dstAddr,copyLen);
        // go forward
        currOff += copyLen;
        
        
    }

    // read the new file to check if the modification worked
    Student buf;
    while( (read(fdd,&buf,sizeof(Student))) == sizeof(Student)  ){
        fprintf(stdout,"%d %ld %s %s %d\n",buf.id,buf.num,buf.name,buf.surname,buf.mark);
    }

    close(fds); close(fdd);

    return 0;
}

void *thread_changeRegNum(void *vargv){
    pthread_argv *argv= (pthread_argv *)vargv;
    Student *s = argv->mem;     // think like in assembly
    int n = argv->n;

    for(int i=0;i<n;i++){
        // sleep(1);
        fprintf(stdout,"T0:Curr Student: %d %ld %s %s %d\n",s->id,s->num,s->name,s->surname,s->mark);

        s->num++;
        s++;    // go forward
    }

    return 0;
}

void *thread_changeMark(void *vargv){
    pthread_argv *argv= (pthread_argv *)vargv;
    Student *s = argv->mem;     // think like in assembly
    int n = argv->n;

    for(int i=0;i<n;i++){
        // sleep(2);
        fprintf(stdout,"T1:Curr Student: %d %ld %s %s %d\n",s->id,s->num,s->name,s->surname,s->mark);

        s->mark--;
        s--;    // go backward
    }

    return 0;
}