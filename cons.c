#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <math.h>
#include<fcntl.h>
#define BUF_SIZE 20
#define SHMKEY 4067
#define THNUM 3

struct buf_area{
    int rear;
    int head;
    int buffer[BUF_SIZE];
};


int lambda;
struct buf_area *buf_ptr = NULL;
sem_t* empty;
sem_t* mutex;
sem_t* full;

int *shm_ptr = NULL;

double NEGEXP_time(double lambda);
void* consume(void* argv);

int main(int argc, char*argv[]){
    if(argc < 2){
        printf("error: no argument\n");
        return -1;
    }
    empty = sem_open("prod_cons_empty", O_CREAT);
    full = sem_open("prod_cons_full", O_CREAT);
    mutex = sem_open("prod_cons_mutex", O_CREAT);

    int shmid;

    shmid = shmget((key_t)SHMKEY, sizeof(struct buf_area), IPC_CREAT|666);
    printf("%d?\n", shmid);
    if(shmid<0){
        perror("shmget error!");
        exit(1);
    }

    buf_ptr = (struct buf_area *)shmat(shmid, NULL, 0);   //获取共享内存
    printf("%d!!!!!\n", buf_ptr->buffer[0]);

    pthread_t tidp[THNUM];
    for(int k = 0; k < THNUM; k++) pthread_create(&tidp[k],NULL,consume,(void*)&k);

    for(int k = 0; k < THNUM; k++) pthread_join(tidp[k], NULL);

    sem_unlink("prod_cons_empty");
    sem_unlink("prod_cons_full");
    sem_unlink("prod_cons_mutex");

    shmdt(buf_ptr);
    shmctl(shmid, IPC_RMID, 0);
}

void* consume(void* argv){
    int id = *((int*)argv);
    while(1){
        usleep(1000000*NEGEXP_time(lambda));
        sem_wait(full);
        sem_wait(mutex);
        int data = buf_ptr->buffer[buf_ptr->head];
        buf_ptr->head = (buf_ptr->head+1)%BUF_SIZE;
        sem_post(mutex);
        sem_post(empty);
        printf("Thread %u of Process %d get data %d\n", (unsigned int)pthread_self(), getpid(), data);
    }
    pthread_exit(NULL);
}


double NEGEXP_time(double lambda){
    double x = 0;
    while(x == 0 || x == 1) x = (double)rand() / RAND_MAX;
    return -1/lambda*log(x);
}
