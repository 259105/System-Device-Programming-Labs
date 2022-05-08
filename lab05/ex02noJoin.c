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
    pthread_cond_t *cv;
    pthread_mutex_t *mu;
    int numReady;
}pthread_synch;

typedef struct {
    pthread_t tid;
    char *filePath;
    int *v;
    int sizeV;
    int status;     // {0, 1, 2} => { working, ready, done }
    pthread_synch *synch;
} pthread_pars;

void saveInFile(char *, int *, int);

int main(int argc, char *argv[]){
    setbuf(stdout, 0);
    if(argc<3){
        fprintf(stderr,"Error paramiters: <srcFile> ... <dstFile>\n");
        exit(-1);
    }
    clock_t t_start, t_end;

    pthread_pars *thr_pars = (pthread_pars *)malloc((argc-2)*sizeof(pthread_pars));
    pthread_synch *thr_synch = (pthread_synch *) malloc(sizeof(pthread_synch));
    thr_synch->cv = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    thr_synch->mu = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(thr_synch->mu, NULL);
    pthread_cond_init(thr_synch->cv, NULL);
    t_start = clock();
    for(int i=0;i<argc-2;i++){
        thr_pars[i].filePath=argv[i+1];
        thr_pars[i].v = NULL;
        thr_pars[i].status = 0;
        thr_pars[i].synch = thr_synch;
        pthread_create(&(thr_pars[i].tid),NULL,thread_fn,(void *)&(thr_pars[i]));
    }

    int done = 1; // surely you are merging more than 1 vectors
    int *v;
    int size = 0;
    int *oldV;
    int oldSize = 0;
    while(done != argc-2){
        pthread_mutex_lock(thr_synch->mu);
            // wait until the number of ready vector is at least 2
            while(thr_synch->numReady < 2) 
                pthread_cond_wait(thr_synch->cv,thr_synch->mu);

            size = oldSize;
            // array of index (avoid to cycle too times)
            int *indicesReadyVects = (int *) malloc(thr_synch->numReady * sizeof(int));
            int *indicesCurrPos = (int *) malloc((thr_synch->numReady+1) * sizeof(int));
            int k=0; // usefull for saving the indices of Vectors ready

            // loop on all thread and find who have finished, then take the dimension of the array and increment the newSize array
            for(int i=0; i<argc-2; i++){
                if(thr_pars[i].status == 1){
                    size += thr_pars[i].sizeV;
                    indicesReadyVects[k] = i;
                    indicesCurrPos[k++] = 0;
                }
            }  

#ifdef DEBG
            for(int i=0; i<thr_synch->numReady; i++){
                printf("%d: ", i);

                int printSize;
                int *printArray;
                // here I divide the case in which is present also the old vector
                if(oldSize != 0 && i == thr_synch->numReady-1){
                    printSize = oldSize;
                    printArray = oldV;
                }else{
                    printSize = thr_pars[indicesReadyVects[i]].sizeV;
                    printArray = thr_pars[indicesReadyVects[i]].v;
                }

                for(int j=0;j<printSize;j++){
                    printf("%d ", printArray[j]);
                }
                printf("\n");
            }
#endif

            // create the new array
            v = (int *) malloc( size *sizeof(int));

            // MERGE OF VECTORS
            for(int i=0; i<size; i++){
                v[i] = INT_MAX;
                int vectMin;
                for(int j=0; j<thr_synch->numReady; j++){
                    if(oldSize != 0 && j == thr_synch->numReady-1){
                        // in the old merged vector
                        if(indicesCurrPos[j] < oldSize
                            && oldV[indicesCurrPos[j]] < v[i]){

                            v[i] = oldV[indicesCurrPos[j]];
                            vectMin = j;
                        }
                    }else{
                        // in the sorted vectors
                        int readyThreadIndex = indicesReadyVects[j];
                        if(indicesCurrPos[j] < thr_pars[readyThreadIndex].sizeV
                            && thr_pars[readyThreadIndex].v[indicesCurrPos[j]] < v[i]){

                            v[i] = thr_pars[readyThreadIndex].v[indicesCurrPos[j]];
                            vectMin = j;
                        }
                    }
                }
                indicesCurrPos[vectMin]++;
            }

            free(indicesCurrPos);
            free(indicesReadyVects);
            

            // the first time we don't free the oldV, because it don't exist yet
            if(oldSize!=0)
                free(oldV);
            
            // the computed array now become the old, for the next cycle
            oldV = v;
            oldSize = size;

            // set the new status at the end of merging, in order to don't merge that array anymore
            for(int i=0;i<argc-2; i++)
                if(thr_pars[i].status == 1)
                    thr_pars[i].status = 2;

            // increment the total number of merged vectors
            done += (thr_synch->numReady-1);

            thr_synch->numReady = 1; // because after merging we have 1 array
            
        pthread_mutex_unlock(thr_synch->mu);

    }
    
    t_end = clock();
    double time = (double)(t_end - t_start) / CLOCKS_PER_SEC;
    printf("%f\n",time);

#ifdef DEBG
    printf("v: ");
    for(int j=0;j<size;j++){
        printf("%d ",v[j]);
    }
    printf("\n");
#endif

    saveInFile(argv[argc-1],v,size);

    free(v);

    pthread_cond_destroy(thr_synch->cv);
    pthread_mutex_destroy(thr_synch->mu);
    free(thr_synch);

    for(int i=0;i<argc-2;i++)
        if(thr_pars[i].v!=NULL)
            free(thr_pars[i].v);
    free(thr_pars);

    return 0;
}

void* thread_fn(void* vpars){
    pthread_pars *pars = (pthread_pars *) vpars;
    int fd;

    pthread_detach (pthread_self());

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

    // Unlock cond variable
    pthread_mutex_lock(pars->synch->mu);
        pars->synch->numReady++; // increment the counter of threads Ready
        pars->status = 1;   // Set the status to ready
        pthread_cond_signal(pars->synch->cv); // unlock the main thread
    pthread_mutex_unlock(pars->synch->mu);

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
