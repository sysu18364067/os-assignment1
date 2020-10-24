#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void* philo(void*argv);
void pickup_forks(int i);
void return_forks(int i);
void tryeat(int i);
void printStatus();

enum {THINKING, HUNGRY, EATING} state[5];
pthread_cond_t self[5];
pthread_mutex_t mutex[5];
pthread_t tid[5];

void* philo(void*argv){
    //sleep(1);
    int id = *((int*)argv);
    //printf("philosopher %d sit down\n", id);
    while(1){
        pickup_forks(id);
        //printf("philosopher %d is eating\n", id);
        sleep(rand()%3+1);
        return_forks(id);
        //printf("philosopher %d is thinking\n", id);
        sleep(rand()%3+1);
    }
    pthread_exit(NULL);
}

void pickup_forks(int i){
    state[i] = HUNGRY;
    printStatus();
    tryeat(i);
    pthread_mutex_lock(&mutex[i]);
    while(state[i] != EATING){
        pthread_cond_wait(&self[i], &mutex[i]);
    }
    pthread_mutex_unlock(&mutex[i]);
}

void return_forks(int i){
    state[i] = THINKING;
    printStatus();
    //printf("philosopher %d says he has finished dinner\n", i);
    tryeat((i+1)%5);
    tryeat((i+4)%5);
}

void tryeat(int i){
    //printf("philosopher %d try to eat\n", i);
    if(state[i] == HUNGRY && (state[(i+4)%5] != EATING) && state[(i+1)%5] != EATING) {
        pthread_mutex_lock(&mutex[i]);
        state[i] = EATING;
        printStatus();
        pthread_cond_signal(&self[i]);
        pthread_mutex_unlock(&mutex[i]);
    }
    //else printf("philosopher %d fail to eat\n", i);
}

int main(int argc, char*argv[]){
    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    int id[] = {0, 1, 2, 3, 4};

    for(int k = 0; k < 5; ++k){
        state[k] = THINKING;
        pthread_mutex_init(&mutex[k], &ma);
        pthread_cond_init(&self[k], NULL);
    }
    for(int k = 0; k < 5; ++k){
        //printf("inviting philosopher %d\n", k);
        fflush(stdout);
        pthread_create(&tid[k],NULL,philo,(void*)&id[k]);
    }
    for(int k = 0; k < 5; ++k) pthread_join(tid[k], NULL);
}

void printStatus(){
    printf("===============\n");
    for(int i = 0; i < 5; i++){
    	switch (state[i]){
    	case HUNGRY:
    	    printf("philo %d: hungry\n", i);break;
    	case THINKING:
    	    printf("philo %d: thinking\n", i);break;
    	case EATING:
    	    printf("philo %d: eating\n", i);break;
    	default:
    	    break;
    	}
    }
}
