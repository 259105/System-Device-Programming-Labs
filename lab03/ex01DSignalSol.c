/*
 *  StQ 24.03.2022
 *  Parent process with two children connected with 2 pipes
 *  Children write; parent read
 *  Asynchronous I/O
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <aio.h>

#define L         100
#define STR_SIZE   5
#define STR_NUM     3
#define WAIT_TIME_1 1
#define WAIT_TIME_2 3

void parent ();
void child1 ();
void child2 ();
void generate (int *, char []);
void convert (char []);
void get_time (char []);
void my_read1 (int);
void my_read2 (int);

int i1, i2, f1, f2, n1, n2;
int fd1[2], fd2[2];

int main (int argc, char ** argv) {
    pid_t pid1, pid2;

    // no buffering
    setbuf (stdout, 0);

    // Create pipe
    pipe(fd1);
    pipe(fd2);

    // Signal handlers for aysnchronous I/O
    signal (SIGUSR1, my_read1);
    signal (SIGUSR2, my_read2);

    pid1 = fork();
    if (pid1==0) {
        // Child 1
        srand (getpid());
        child1();
    } else {
        pid2 = fork();
        if (pid2==0) {
            // Child 2
            srand (getpid());
            child2();
        } else {
            // Parent
            close (fd1[1]);
            close (fd2[1]);
            parent ();
        }
    }

    wait (NULL);
    wait (NULL);

    close (fd1[0]);
    close (fd1[0]);

    return (0);
}

void parent () {
    struct aiocb aios1, aios2;

    aios1.aio_fildes = fd1[0];
    aios1.aio_offset = 0;
    aios1.aio_buf = &n1;
    aios1.aio_nbytes = sizeof (int);
    aios1.aio_reqprio = 0;
    aios1.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aios1.aio_sigevent.sigev_signo = SIGUSR1;
    //aios1.aio_sigevent.sigev_value = ;
    //aios1.aio_sigevent.sigval = ;
    //aios1.aio_sigevent.sigev_notify_attributes = ;
    //aios1.aio_lio_opcode = ;

    aios2.aio_fildes = fd2[0];
    aios2.aio_offset = 0;
    aios2.aio_buf = &n2;
    aios2.aio_nbytes = sizeof (int);
    aios2.aio_reqprio = 0;
    aios2.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aios2.aio_sigevent.sigev_signo = SIGUSR2;
    //aios2.aio_sigevent.sigev_value = ;
    //aios2.aio_sigevent.sigval = ;
    //aios2.aio_sigevent.sigev_notify_attributes = ;
    //aios2.aio_lio_opcode = ;

    i1 = i2 = f1 = f2 = 0;
    while (i1<STR_NUM || i2<STR_NUM) {
        if (i1<STR_NUM && f1 == 0) {
            aio_read(&aios1);
            f1 = 1;
        }
        if (i2<STR_NUM && f2 == 0) {
            aio_read(&aios2);
            f2 = 1;
        }

        // Do something else ... nothing to do, then pause
        pause();
    }

    return;
}

void my_read1 (int sig) {
    char str[L], my_time[L];

    read(fd1[0], str, n1);
    get_time(my_time);
    convert(str);
    fprintf(stdout, "From Child1 R [%d][%s]: %d %s\n", i1 + 1, my_time, n1, str);
    i1++;
    f1 = 0;

    return;
}

void my_read2 (int sig) {
    char str[L], my_time[L];

    read(fd2[0], str, n2);
    get_time(my_time);
    convert(str);
    fprintf(stdout, "\t\t\t\t\t\tFrom Child2 R [%d][%s]: %d %s\n", i2 + 1, my_time, n2, str);
    i2++;
    f2 = 0;

    return;
}

void child1 () {
    int i, l;
    char str[L], my_time[L];

    close (fd1[0]);
    for (i=0; i<STR_NUM; i++) {
        sleep (WAIT_TIME_1);
        generate (&l, str);
        get_time (my_time);
        fprintf(stdout, "---- Child1 W [%d][%s]: %d %s\n", i+1, my_time, l, str);
        write (fd1[1], &l, sizeof(int));
        write (fd1[1], str, l);
    }
    return;
}

void child2 () {
    int i, l;
    char str[L], my_time[L];

    close(fd2[0]);
    for (i = 0; i < STR_NUM; i++) {
        sleep(WAIT_TIME_2);
        generate(&l, str);
        get_time(my_time);
        fprintf(stdout, "\t\t\t\t\t\t---- Child2 W [%d][%s]: %d %s\n", i + 1, my_time, l, str);
        write(fd2[1], &l, sizeof(int));
        write(fd2[1], str, l);
    }
    return;
}

void generate (int *l, char str[]) {
    int i;
    *l = 1 + rand() % STR_SIZE;
    for (i = 0; i < *l; i++) {
        str[i] = (char) (((int) 'a') + rand() % 26);
    }
    str[i] = '\0';
    *l = *l + 1;
    return;
}

void convert (char str[]) {
    int i;
    for (i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
    return;
}

void get_time (char my_time[L]) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(my_time, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return;
}
