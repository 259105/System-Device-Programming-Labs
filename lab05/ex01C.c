#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>

#define  MAXLEN 20
#define  SHM_SIZE 1024
#define  PROJID 7

extern int errno;
char err[64];

int P1(int , int shmid);
int P2(int , int shmid);
char* toUpper(char *);
void generateText(char *, int, unsigned int);

typedef struct mymsg {
    long mtype;       /* Message type. */
    // char mtext[N_BYTES]
    int mtext[1];    /* Message text. */
    //  data portion that holds the data bytes of the message
    // can be whatever you whant
} MyMsg;

int main(int argc, char **argv){
    key_t key;
    int shmid;
    int msgid;

    system("touch ex01.txt");
    if((key = ftok("ex01.txt", PROJID)) == -1){
        perror("ftok");
        exit(1);
    }
    if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
        perror("msgget");
        exit(1);
    }
    if((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1){
        perror("shmget");
        exit(1);
    }

    if(fork()){
        P1(msgid, shmid);
    }else{
        P2(msgid, shmid);
    }
    // removing the queue from the system
    if(shmctl(shmid, IPC_RMID, NULL)){
        perror("shmctl");
        exit(1);
    }
    if(msgctl(msgid, IPC_RMID, NULL)){
        perror("msgctl");
        exit(1);
    }
    system("rm -rf ex01.txt");
    return 0;
}

int P1(int msgid, int shmid){
    unsigned int seed = getpid();
    int n;
    char *data;
    MyMsg msg;

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
        
        msg.mtype = 1; // fifo1
        *msg.mtext = n; // send the dimention of generated string

        if(msgsnd(msgid, (void *)&msg, 1*sizeof(int), 0) == -1){
            perror("msgsnd");
            exit(1);
        }
        if(n == 0){
            fprintf(stdout,"P1 is closing\n");
            break;
        }

        if(msgrcv(msgid, (void *)&msg, 1*sizeof(int), 2, 0) == -1){
            // the type is 2 => fifo2
            perror("msgrcv");
            exit(1);
        }

        n = *msg.mtext; // take the dimention

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

int P2(int msgid, int shmid){
    unsigned int seed = getpid();
    int n;
    char *data;
    MyMsg msg;

    // Attach
    data = shmat(shmid, NULL, 0);
    if(data == (char *)(-1)){
        perror("shmat");
        exit(1);
    }

    while(1){

        if(msgrcv(msgid, (void *)&msg, 1*sizeof(int), 1, 0) == -1){
            // the type is 1 => fifo1
            perror("msgrcv");
            exit(1);
        }

        n = *msg.mtext; // take the dimention

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

        msg.mtype = 2; // fifo2
        *msg.mtext = n;

        if(msgsnd(msgid, (void *)&msg, 1*sizeof(int), 0) == -1){
            perror("msgsnd");
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
