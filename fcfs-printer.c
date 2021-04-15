#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "linked_list.c"
#define SIZE 5
#define NUMB_THREADS 6
#define PRODUCER_LOOPS 2

#define SHMSZ 300 

typedef struct process_info buffer_t;
buffer_t buffer[SIZE];
int buffer_index;

key_t keyBuffer;
key_t keyFullSem;
key_t keyEmptySem;

int shmidOne;
int shmidTwo;
int shmidThree;

struct cs_semaphore {
    int val;
    sem_t wait;
    sem_t mutex;
};

// pthread_mutex_t buffer_mutex;
pthread_mutex_t *buffer_mutex_pointer;
/* initially buffer will be empty.  full_sem
   will be initialized to buffer SIZE, which means
   SIZE number of producer threads can write to it.
   And empty_sem will be initialized to 0, so no
   consumer can read from buffer until a producer
   thread posts to empty_sem */
// struct cs_semaphore full_sem;  
// struct cs_semaphore empty_sem; 
struct cs_semaphore* full_sem_pointer;
struct cs_semaphore* empty_sem_pointer;

void my_sem_init(struct cs_semaphore* cs, int K) {
    cs->val = K;
    if(cs->val > 0) {
        sem_init(&cs->wait, 0, 1);
    } else {
        sem_init(&cs->wait, 0, 0);
    }
    sem_init(&cs->mutex, 0, 1);
}

void my_sem_wait(struct cs_semaphore* cs) {
    sem_wait(&cs->wait);
    sem_wait(&cs->mutex);
    cs->val = cs->val - 1;
    if (cs->val > 0) {
        sem_post(&cs->wait);
    } 
    sem_post(&cs->mutex);
}

void my_sem_post(struct cs_semaphore* cs) {
    sem_wait(&cs->mutex);
    cs->val = cs->val + 1;
    if(cs->val == 1) {
        sem_post(&cs->wait);
    }
    sem_post(&cs->mutex);
}

void my_sem_destroy(struct cs_semaphore* cs) {
    sem_destroy(&cs->mutex);
    sem_destroy(&cs->wait);
}
 
void insertbuffer(buffer_t value) {
    if (buffer_index < SIZE) {
        buffer[buffer_index++] = value;
    } else {
        printf("Buffer overflow\n");
    }
}
 
buffer_t dequeuebuffer() {
    if (buffer_index > 0) {
        return buffer[--buffer_index]; // buffer_index-- would be error!
    } else {
        printf("Buffer underflow\n");
    }
    struct process_info empty_info;
    empty_info.process_id = 0;
    empty_info.size = 0;
    return empty_info;
}

void *producer(void *thread_n) {
    int thread_numb = *(int *)thread_n;
    buffer_t value;
    int i=0;
    while (i++ < PRODUCER_LOOPS) {
        sleep(rand() % 10);

        struct process_info test_info;
        test_info.process_id = 20;
        test_info.size = 500;
        
        my_sem_wait(full_sem_pointer); // sem=0: wait. sem>0: go and decrement it
        /* possible race condition here. After this thread wakes up,
           another thread could aqcuire mutex before this one, and add to list.
           Then the list would be full again
           and when this thread tried to insert to buffer there would be
           a buffer overflow error */
        pthread_mutex_lock(buffer_mutex_pointer); /* protecting critical section */
        insertbuffer(test_info);
        pthread_mutex_unlock(buffer_mutex_pointer);
        my_sem_post(empty_sem_pointer); // post (increment) emptybuffer semaphore
        printf("Producer %d added %d to buffer\n", thread_numb, value);
    }
    pthread_exit(0);
}
 
void *consumer(void *thread_n) {
    int thread_numb = *(int *)thread_n;
    buffer_t value;
    int i=0;
    while (i++ < PRODUCER_LOOPS) {
        my_sem_wait(empty_sem_pointer);
        /* there could be race condition here, that could cause
           buffer underflow error */
        pthread_mutex_lock(buffer_mutex_pointer);
        value = dequeuebuffer(value);
        pthread_mutex_unlock(buffer_mutex_pointer);
        my_sem_post(full_sem_pointer); // post (increment) fullbuffer semaphore
        printf("Consumer %d dequeue %d from buffer\n", thread_numb, value);
   }
    pthread_exit(0);
}
 
int main(int argc, int **argv) {
    // Initialize shared memory variables 
	keyBuffer = 4910;
	if ((shmidOne = shmget(keyBuffer, sizeof(pthread_mutex_t), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	if ((buffer_mutex_pointer = shmat(shmidOne, NULL, 0)) == (pthread_mutex_t*) -1) {
		perror("shmat");
		exit(1);
	}

    pthread_mutex_init(buffer_mutex_pointer, NULL); 

    buffer_index = 0;
    
	// empty_sem_pointer = &empty_sem;
	keyEmptySem = 4911;
	if ((shmidTwo = shmget(keyEmptySem, sizeof(struct cs_semaphore), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	if ((empty_sem_pointer = shmat(shmidTwo, NULL, 0)) == (struct cs_semaphore*) -1) {
		perror("shmat");
		exit(1);
	}
	

	// full_sem_pointer = &full_sem;
	keyFullSem = 4912;
	if ((shmidThree = shmget(keyFullSem, sizeof(struct cs_semaphore), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	if ((full_sem_pointer = shmat(shmidThree, NULL, 0)) == (struct cs_semaphore*) -1) {
		perror("shmat");
		exit(1);
	}

    my_sem_init(full_sem_pointer, SIZE); 
    my_sem_init(empty_sem_pointer, 0);

    head = NULL;
    struct node *n;

    pthread_t thread[NUMB_THREADS];
    int thread_numb[NUMB_THREADS];
    int i;
    for (i = 0; i < NUMB_THREADS; ) {
        thread_numb[i] = i;
        pthread_create(thread + i, // pthread_t *t
                       NULL, // const pthread_attr_t *attr
                       producer, // void *(*start_routine) (void *)
                       thread_numb + i);  // void *arg
        i++;
        thread_numb[i] = i;
        // playing a bit with thread and thread_numb pointers...
        pthread_create(&thread[i], // pthread_t *t
                       NULL, // const pthread_attr_t *attr
                       consumer, // void *(*start_routine) (void *)
                       &thread_numb[i]);  // void *arg
        i++;
    }
 
    for (i = 0; i < NUMB_THREADS; i++)
        pthread_join(thread[i], NULL);
 
    pthread_mutex_destroy(buffer_mutex_pointer);
    my_sem_destroy(full_sem_pointer);
    my_sem_destroy(empty_sem_pointer);
 
    return 0;
}