#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "tas.h"

#define LOCKED (1)
#define UNLOCKED (0)

#define TEST_ITERATION (1000000)
#define N_THREADS (4)

struct tas {
    int *lock;
};

struct data {
    struct tas *tas;
    long *counter;
};

//This is initializing the state to the unlock state
void tas_init(struct tas *t) {
    t->lock = malloc(sizeof(int));
    *(t->lock) = 0;
}

int tas_lock(struct tas *t) {
    int expected = UNLOCKED;
    if(NULL == t) {
        perror("tas is not initialized\n");
        return -1;
    } 
    while(!__atomic_compare_exchange_n(t->lock, &expected,
        LOCKED, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            expected = UNLOCKED;  
    }
    if(1 != *(t->lock)) {
        perror("Spin lock dident execute correctly\n");
        return -1;
    }
    return 0;
}

int tas_unlock(struct tas *t) {
    //Testing if the structure t is valid
    if(NULL == t) {
        perror("tas is not initialized\n");
        return -1;
    } 
    __atomic_store_n(t->lock, UNLOCKED, __ATOMIC_SEQ_CST);
    return 0;
}

//This will free the memory that was created 
int tas_destory(struct tas *t) {
    free(t->lock);
    free(t);
    return 0;
}

//Testing Function
void* worker(void *args) {
    struct data *data = (struct data *)args;
    // tas_lock(data->tas);
    for(int i = 0; i < TEST_ITERATION; i++) {
        tas_lock(data->tas);
        *(data->counter) = *(data->counter) + 1;
        tas_unlock(data->tas);
    }
    // tas_unlock(data->tas);

}

//Testing Function
int main() {
    long counter = 0;
    struct tas* tas = malloc(sizeof(struct tas));
    tas_init(tas);
    struct data data = {.tas = tas, .counter = &counter};
    pthread_t threads[N_THREADS];

    for(int i = 0; i < N_THREADS; i ++) {
        pthread_create(threads+i, NULL, worker, &data);
    }

    for(int i = 0; i < N_THREADS; i ++) {
        pthread_join(threads[i], NULL);
    }

    tas_destory(tas);

    printf("EXPECTED VALUE = %d\n", TEST_ITERATION * N_THREADS);
    printf("OBTAINED VALUE = %ld\n", *(data.counter));

    return 0; 
}
