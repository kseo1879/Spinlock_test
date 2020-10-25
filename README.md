# TAS
This is the spinlock for HW7 SOFT3410 2020 Semester2

## Testing
```bash
#define TEST_ITERATION (1000000)
#define N_THREADS (4)
```
On line 10 and 11 there is the number of iteration and threads.

The expected Values should be TEST_ITERATION * N_THREADS.

When you run the program it will print out the expected value and the obtained value. 

```bash 
EXPECTED VALUE = 400000000
OBTAINED VALUE = 400000000
```

## tas_lock function (defensive coding)
It will test whether the value of the lock has sucessfully changed.
However it cannot check for the deadlock scenario because there is not point in using spin lock if it tried to prevent from deadlock happening. 
To be specific if you look at the code below,
```bash
    if(LOCKED == *(t->lock)) {
        perror("Deadlock");
        return -1;
    }
```
If we add this statement before we lock, than it will never reach the spin lock below, therefore there is no point of using it. 

Instead of doing this, I have added THIS CODE after spin lock, 
```bash
    while(!__atomic_compare_exchange_n(t->lock, &expected,
        LOCKED, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            expected = UNLOCKED;  
    }
    //THIS CODE
    if(1 != *(t->lock)) {
        perror("==UNDEFINED BEHAVIOR==\n"); 
        perror("Spin lock didn't execute correctly\n");
        perror("The lock is not locked correctly\n");
        perror("Or Multiple threads might be accessing the same lock and unlcocked it.\n");
        return -1;
    }
```
which will insure the value stays as locked after it executes the spin lock.
This will also ensure that other threads are not trying to access the same lock and unlocking it. 


## tas_unlock function (defensive coding)
```bash
    __atomic_store_n(t->lock, UNLOCKED, __ATOMIC_SEQ_CST);
```
Instead of unlocking the function with aboe sentense it will use 
```bash
    if(!__atomic_compare_exchange_n(t->lock, &expected,
        UNLOCKED, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {

            expected = LOCKED;
            perror("==UNDEFINED BEHAVIOR==\n");
            perror("It may be trying to double free and the lock value is not correct\n");
            perror("OR Multiple threads might be accessing the same lock\n");
            //This means theire is an error state where us user might be trying to free it twice
            return -1;
            
    }
```
This method CAS functionality which can ensure that is does not free the item twitce. When unlock function is called, the lock should be in UNLOCKED state otherwise this code will tell that that the lock is already unlocked..