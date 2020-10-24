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
void* produce(void *argv);

int main(int argc, char*argv[]){
    if(argc < 2){
        printf("error: no argument\n");
        return -1;
    }
    lambda = atof(argv[1]);
    struct buf_area buf;

    memset(&buf, 0, sizeof(struct buf_area));


    /*初始化信号量*/
    empty = sem_open("prod_cons_empty", O_CREAT, 0666, 0);
    sem_init(empty, 1, BUF_SIZE);
    full = sem_open("prod_cons_full", O_CREAT, 0666, 0);
    sem_init(full, 1, 0);
    mutex = sem_open("prod_cons_mutex", O_CREAT, 0666, 0);
    sem_init(mutex, 1, 1);

    int shmid;
    shmctl(shmid, IPC_RMID, 0);
    shmid = shmget(SHMKEY, 0, 0);
    if (shmid != -1) shmctl(shmid, IPC_RMID, 0);

    shmid = shmget((key_t)SHMKEY, sizeof(struct buf_area), IPC_CREAT|0777);
    if(shmid<0){
        perror("shmget error!");
        exit(1);
    }

    buf_ptr = (struct buf_area *)shmat(shmid, NULL, 0);   //在任意适合地方创建可读写的共享内存。返回值为(void*)-1说明出错。

    pthread_t tidp[THNUM];

    for(int k = 0; k < THNUM; k++) pthread_create(&tidp[k],NULL,produce,(void*)&k);

    for(int k = 0; k < THNUM; k++) pthread_join(tidp[k], NULL);

    sem_unlink("prod_cons_empty");
    sem_unlink("prod_cons_full");
    sem_unlink("prod_cons_mutex");

    shmdt(buf_ptr);
    shmctl(shmid, IPC_RMID, 0);
}

void* produce(void *argv){
    int id = *(int*)argv;

    while(1){
    	double x = NEGEXP_time(lambda);
        usleep(1000000*x);
        int data = rand()%100;       //生成100内随机数
        sem_wait(empty);
        sem_wait(mutex);
        printf("Thread %u of Process %d write data %d\n", (unsigned int)pthread_self(), getpid(), data);
        buf_ptr->buffer[buf_ptr->rear] = data;
        buf_ptr->rear = (buf_ptr->rear+1) % BUF_SIZE;
        sem_post(mutex);
        sem_post(full);
    }
    pthread_exit(NULL);
}


double NEGEXP_time(double lambda){
    double x = 0;
    while(x == 0 || x == 1) x = (double)rand() / RAND_MAX;
    return -1/lambda*log(1-x);
}
