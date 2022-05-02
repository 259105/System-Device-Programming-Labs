#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#define  MAXLEN 20
#define  SHM_SIZE 1024
#define  PROJID 7

extern int errno;
char err[64];

int P1(int ,int , int shmid);
int P2(int ,int , int shmid);
char* toUpper(char *);
void generateText(char *, int, unsigned int);

int main(int argc, char **argv){
    char *fifo1 = "./tmp/fifo1";
    char *fifo2 = "./tmp/fifo2";
    int ff1, ff2;
    key_t key;
    int shmid;


    if(mkfifo(fifo1, 0666) || mkfifo(fifo2, 0666)){
        sprintf(err,"mkfifo1 %d",errno);
        perror(err);
        exit(1);
    }
    if((ff1 = open(fifo1,O_RDWR)) == -1 || (ff2 = open(fifo2, O_RDWR)) == -1){
        perror("open");
        exit(1);
    }

    system("touch ex01.txt");
    if((key = ftok("ex01.txt", PROJID)) == -1){
        perror("ftok");
        exit(1);
    }
    if((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1){
        perror("shmget");
        exit(1);
    }

    if(fork()){
        P1(ff1,ff2, shmid);
    }else{
        P2(ff1,ff2, shmid);
    }

    close(ff1);
    close(ff2);

    system("rm -rf ex01.txt");
    system("rm -rf tmp");
    return 0;
}

int P1(int ff1,int ff2, int shmid){
    unsigned int seed = getpid();
    int n;
    char *data;

    // Attach
    data = shmat(shmid, NULL, 0);
    if(data == (char *)(-1)){
        perror("shmat");
        exit(1);
    }

    while(1){
        n = rand_r(&seed) % MAXLEN;
        // randomly generates text in the data addr of shared mem
        generateText(data, n, seed);
        printf("P1 Generated n=%d:\n%s\n", n, data);
        
        if(write(ff1, &n, sizeof(int)) != sizeof(int)){
            sprintf(err, "P1: Error write from pipe");
            perror(err);
            exit(1);
        }
        if(n == 0){
            fprintf(stdout,"P1 is closing\n");
            break;
        }

        if(read(ff2, &n, sizeof(int)) != sizeof(int)){
            sprintf(err, "P1: Error read from pipe");
            perror(err);
            exit(1);
        }
        if(n == 0){
            fprintf(stdout,"P1 is closing\n");
            break;
        }
        // get the text generated from the shared memory
        // print the text in upper case
        fprintf(stdout, "P1 Getted n=%d:\n%s\n",n, toUpper(data));
    }  

    if(shmdt(data) == -1){
        perror("shmdt");
        exit(1);
    }

    return 0;
}

int P2(int ff1,int ff2, int shmid){
    unsigned int seed = getpid();
    int n;
    char *data;

    // Attach
    data = shmat(shmid, NULL, 0);
    if(data == (char *)(-1)){
        perror("shmat");
        exit(1);
    }

    while(1){
        if(read(ff1, &n, sizeof(int)) != sizeof(int)){
            sprintf(err, "P2: Error read from pipe");
            perror(err);
            exit(1);
        }
        if(n == 0){
            fprintf(stdout,"P2 is closing\n");
            break;
        }
        // get the text generated from the shared memory
        // print the text in upper case
        fprintf(stdout, "P2 Getted n=%d:\n%s\n",n, toUpper(data));

        n = rand_r(&seed) % MAXLEN;
        // randomly generates text in the data addr of shared mem
        generateText(data, n, seed);
        printf("P2 Generated n=%d:\n%s\n", n, data);

        if(write(ff2, &n, sizeof(int)) != sizeof(int)){
            sprintf(err, "P2: Error write from pipe");
            perror(err);
            exit(1);
        }
        if(n == 0){
            fprintf(stdout,"P2 is closing\n");
            break;
        }
    }

    if(shmdt(data) == -1){
        perror("shmdt");
        exit(1);
    }

    return 0;
}

char* toUpper(char *str){
    for(int i=0; i<strlen(str); i++)
        if(str[i] >= 'a' && str[i] <= 'z')
            str[i] = str[i] - 'a' + 'A';
    return str;
}

void generateText(char *str, int n,unsigned int seed){
    int r;
    int i;
    for(i=0; i<n; i++){
        r = rand_r(&seed) % 28; // 26 downcase letter spaces and newlines
        if(r == 26)
            str[i] = ' ';
        else if(r == 27)
            str[i] = '\n';
        else
            str[i] = r + 'a';
    }
    str[i] = '\0';
}
