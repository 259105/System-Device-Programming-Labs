#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

void* thread_fn(void*);
void swap(int *,int *);
void heapify(int *, int , int );
void heapSort(int *, int );

typedef struct {
    pthread_t tid;
    char *filePath;
    int *v;
    int sizeV;
} pthread_pars;

void saveInFile(char *, int *, int);

// pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
    if(argc<3){
        fprintf(stderr,"Error paramiters: <srcFile> ... <dstFile>\n");
        exit(-1);
    }
    clock_t t_start, t_end;

    pthread_pars *thr_pars = (pthread_pars *)malloc((argc-2)*sizeof(pthread_pars));
    t_start = clock();
    for(int i=0;i<argc-2;i++){
        thr_pars[i].filePath=argv[i+1];
        thr_pars[i].v = NULL;
        pthread_create(&(thr_pars[i].tid),NULL,thread_fn,(void *)&(thr_pars[i]));
    }

    int *v;
    int size = 0;
    int *oldV;
    int oldSize = 0;
    for(int i=0;i<argc-2;i++){
        pthread_join(thr_pars[i].tid,0);

        // printf("OldV: ");
        // for(int j=0;j<oldSize;j++){
        //     printf("%d ",oldV[j]);
        // }
        // printf("\n");

        // printf("Curr: ");
        // for(int j=0;j<thr_pars[i].sizeV;j++){
        //     printf("%d ",thr_pars[i].v[j]);
        // }
        // printf("\n");

        // create the new array
        v = (int *) malloc( (thr_pars[i].sizeV+oldSize) *sizeof(int));
        size = thr_pars[i].sizeV + oldSize;

        // merge
        int n=0, m=0, k=0;
        // n => oldV ; m => currV
        while(k<size){
            if(n>=oldSize){
                v[k++] = thr_pars[i].v[m++];
                continue;
            }
            if(m>=thr_pars[i].sizeV){
                v[k++] = oldV[n++];
                continue;
            }
            if(oldV[n]<thr_pars[i].v[m]){
                v[k++] = oldV[n++];
                continue;
            }else{
                v[k++] = thr_pars[i].v[m++];
                continue;
            }
        }

        // printf("Res: ");
        // for(int j=0;j<size;j++){
        //     printf("%d ",v[j]);
        // }
        // printf("\n");
        
        if(oldSize!=0)
            free(oldV);

        oldV = v;
        oldSize = size;
    }

    t_end = clock();
    double time = (double)(t_end - t_start) / CLOCKS_PER_SEC;
    printf("%f\n",time);

    // saveInFile(argv[argc-1],v,size);

    for(int i=0;i<argc-2;i++)
        if(thr_pars[i].v!=NULL)
            free(thr_pars[i].v);
    free(thr_pars);

    return 0;
}

void* thread_fn(void* vpars){
    pthread_pars *pars = (pthread_pars *) vpars;
    int fd;
    if((fd=open(pars->filePath,O_RDONLY)) == -1){
        perror("Error opening file");
        exit(-1);
    }
    int rt;
    if((rt = read(fd,&(pars->sizeV),sizeof(int))) == -1 || rt!=sizeof(int)){
        perror("Error reading num elements from file");
        close(fd);
        exit(-1);
    }

    pars->v = (int *)malloc(pars->sizeV*sizeof(int));
    int n;
    for(int i=0;i<pars->sizeV;i++){
        if((rt = read(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
            perror("Error reading value");
            close(fd);
            exit(-1);
        }

        pars->v[i] = n;
    }
    close(fd);

    heapSort(pars->v,pars->sizeV);

    // pthread_mutex_lock(&mu);
    // printf("%d\n",pars->sizeV);
    // for(int i=0;i<pars->sizeV;i++){
    //     fprintf(stdout,"%d ",pars->v[i]);
    // }
    // printf("\n");
    // pthread_mutex_unlock(&mu);

    pthread_exit(NULL);
}

void saveInFile(char * filePath, int *v, int size){
    int fd;
    if((fd = open(filePath,O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU)) == -1){
        perror("Error opening file");
        exit(-1);
    }
    int rt;
    int n=size;
    if((rt=write(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
        perror("Error writing the file");
        close(fd);
        exit(-1);
    }

    for(int i=0; i<size; i++){
        n = v[i];
        if((rt=write(fd,&n,sizeof(int))) == -1 || rt!=sizeof(int)){
            perror("Error writing the file");
            close(fd);
            exit(-1);
        }
    }

    close(fd);
    fprintf(stdout,"TEST PRINT:\n");

    if((fd = open(filePath,O_RDONLY)) == -1){
        perror("Error opening file for testing it");
        exit(-1);
    }
    
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
    return;
}

// Function to swap the the position of two elements
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
  
void heapify(int *arr, int n, int i) {
    // Find largest among root, left child and right child
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    // Swap and continue heapifying if root is not largest
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }
}
  
// Main function to do heap sort
void heapSort(int *arr, int n) {
    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // Heap sort
    for (int i = n - 1; i >= 0; i--) {
        swap(&arr[0], &arr[i]);

        // Heapify root element to get highest element at root again
        heapify(arr, i, 0);
    }
}
