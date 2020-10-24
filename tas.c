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

/**This structure is for TEST ONLY.
 * When you test it will store the data and tas lock and 
 * pass it to different threads.
 */
struct data {
    struct tas *tas;
    long *counter;
};

/**
 * @pram taslock, initialises the handle for the taslock
 */
void tas_init(struct tas *t) {
    t->lock = malloc(sizeof(int));
    *(t->lock) = 0;
}

/**
* Locks, operation should be successful if the lock is valid.
* If the lock is invalid, the lock operation returns a non-zero integer
* tas lock that is currently in a locked state will keep threads waiting. * @param taslock
* @return success
*/
int tas_lock(struct tas *t) {
    int expected = UNLOCKED;
    //Testing if lock is initialized.
    if(NULL == t) {
        perror("tas is not initialized\n");
        return -1;
    } 
    //This will loop until the lock resourse is unlocked.
    while(!__atomic_compare_exchange_n(t->lock, &expected,
        LOCKED, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            expected = UNLOCKED;  
    }
    //THis is to make sure that it leaved the funtion with the correct lock value.
    if(1 != *(t->lock)) {
        perror("Spin lock dident execute correctly\n");
        return -1;
    }
    return 0;
}


/**
* Unlocks, operation should be successful is the lock is valid.
* If the lock is invalid, unlock operation returns a non-zero integer. * @param taslock
* @return success
*/
int tas_unlock(struct tas *t) {
    //Testing if lock is initialized.
    if(NULL == t) {
        perror("tas is not initialized\n");
        return -1;
    } 
    __atomic_store_n(t->lock, UNLOCKED, __ATOMIC_SEQ_CST);
    return 0;
}

/**
 * Destroys the tas lock, puts it in an invalid state
 * @param taslock
*/
int tas_destory(struct tas *t) {
    free(t->lock);
    free(t);
    return 0;
}

//Testing Function
void* worker(void *args) {
    struct data *data = (struct data *)args;
    // if you change line 93 and 95 with 91 and 97 it still works.
    // The intention here was to give more stree to the lock and unlock function.
    // //tas_lock(data->tas);
    for(int i = 0; i < TEST_ITERATION; i++) {
        tas_lock(data->tas);
        *(data->counter) = *(data->counter) + 1;
        tas_unlock(data->tas);
    }
    // //tas_unlock(data->tas);
    // Without the lock specified it will created issues with unintended sharing.
}

//Testing Function
int main() {
    long counter = 0;
    struct tas* tas = malloc(sizeof(struct tas));
    tas_init(tas);
    struct data data = {.tas = tas, .counter = &counter};
    pthread_t threads[N_THREADS];

    //Passing the data value the the threads to change the value. 
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
