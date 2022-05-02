#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>

#define  MAXLEN 20
#define  SHM_SIZE 1024
#define  PROJID 7

extern int errno;
char err[64];

int P1(int *,int *, int shmid);
int P2(int *,int *, int shmid);
char* toUpper(char *);
void generateText(char *, int, unsigned int);

int main(int argc, char **argv){
    int p1[2],p2[2];
    key_t key;
    int shmid;

    if(pipe(p1) || pipe(p2)){
        sprintf(err,"Error opening pipes");
        perror(err);
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
        P1(p1,p2, shmid);
    }else{
        P2(p1,p2, shmid);
    }
    
    if(shmctl(shmid, IPC_RMID, NULL)){
        perror("shmctl");
        exit(1);
    }
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    system("rm -rf ex01.txt");
    return 0;
}

int P1(int *p1,int *p2, int shmid){
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
        
        if(write(p1[1], &n, sizeof(int)) != sizeof(int)){
            sprintf(err, "P1: Error write from pipe");
            perror(err);
            exit(1);
        }
        if(n == 0){
            fprintf(stdout,"P1 is closing\n");
            break;
        }

        if(read(p2[0], &n, sizeof(int)) != sizeof(int)){
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

int P2(int *p1,int *p2, int shmid){
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
        if(read(p1[0], &n, sizeof(int)) != sizeof(int)){
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

        if(write(p2[1], &n, sizeof(int)) != sizeof(int)){
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
