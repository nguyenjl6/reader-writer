#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "linked_list.c"
#define SIZE 25
#define SHMSZ 300 

typedef struct node buffer_t;

key_t keyBuffer;
key_t keyFullSem;
key_t keyEmptySem;
key_t keyLinkedList;

int shmidOne;
int shmidTwo;
int shmidThree;
int shmidFour;

struct cs_semaphore {
    int val;
    sem_t wait;
    sem_t mutex;
};

pthread_mutex_t *buffer_mutex_pointer;

struct cs_semaphore* full_sem_pointer;
struct cs_semaphore* empty_sem_pointer;
struct node *sharedLinkedList;
struct node *head;

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
    if (count() <= 25) {
        insert(value);
        display(head);
    }
    else
    {
        printf("%d ", count());
        printf("Buffer overflow\n");
    }
}
 
buffer_t dequeuebuffer() {
    struct node value;
    if (count() > 0)
    {
        value = delete();        
        return value;
    }
    else
    {
        printf("List is empty\n");
    }
    struct node empty_info;
    empty_info.process_id = -1;
    empty_info.job_size = -1;
    return empty_info;
}

void *producer() {
    //int thread_numb = *(int *)thread_n;
    int i=0;
    struct node test_info;
    // int numLoops = (rand() % 25) + 1;
    int numLoops = 3;
    while (i++ < numLoops)
    {
        sleep(rand() % 10);

        test_info.process_id = getpid();
        srand(getpid());
        test_info.job_size = (rand() % 900) + 100;

        my_sem_wait(full_sem_pointer); 
        pthread_mutex_lock(buffer_mutex_pointer); 
        insertbuffer(test_info);
        pthread_mutex_unlock(buffer_mutex_pointer);
        my_sem_post(empty_sem_pointer); 
        //printf("Producer <%d> added <%d> to buffer\n", thread_numb + 1, test_info.size);
        printf("Producer <%d> added <%d> to buffer\n", getpid(), test_info.job_size);

    }
}

void *consumer(void *thread_n) {
    int thread_numb = *(int *)thread_n;
    buffer_t value;
    while (1) {
        my_sem_wait(empty_sem_pointer);
        pthread_mutex_lock(buffer_mutex_pointer);
        value = dequeuebuffer(); // TODO
        pthread_mutex_unlock(buffer_mutex_pointer);
        my_sem_post(full_sem_pointer); 
        printf("Consumer <%d> dequeue <%d, %d> from buffer\n", thread_numb + 1, value.process_id, value.job_size);
   }
}
 
int main(int argc, char **argv) {

    int processes = atoi(argv[1]);
    int threads = atoi(argv[2]);

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
    
	keyEmptySem = 4911;
	if ((shmidTwo = shmget(keyEmptySem, sizeof(struct cs_semaphore), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	if ((empty_sem_pointer = shmat(shmidTwo, NULL, 0)) == (struct cs_semaphore*) -1) {
		perror("shmat");
		exit(1);
	}
	
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


	keyLinkedList = 4913;
	if ((shmidFour = shmget(keyLinkedList, sizeof(struct node)*25, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

    struct node* initial_element;
    initial_element=(struct node *)malloc(sizeof(struct node));
    initial_element->job_size = -1;
    initial_element->process_id = -1;
    initial_element->next = NULL;

    if ((sharedLinkedList = shmat(shmidFour, NULL, 0)) == (struct node *)-1)
    {
        perror("shmat");
		exit(1);
    }

    sharedLinkedList = initial_element;


    pthread_t thread[threads];
    int thread_numb[threads];
    int i;
    int flag = 0;
    
    head = sharedLinkedList;
    for (i = 0; i < 24; i++)
    {
        struct node *temp;
        temp=(struct node *)malloc(sizeof(struct node));

        temp->job_size = -1;
        temp->process_id = -1;
        temp->next = NULL;
        sharedLinkedList->next = temp;
        sharedLinkedList = temp;
    }

    for(i = 0; i < processes + 1; i++) {
         if (fork() == 0) {
             producer();
             exit(0);
         }
    }

    for (i = 0; i < processes + 1; i++) {
        wait(NULL);
        flag = 1;
    }

    for (i = 0; i < threads; i++)
    {
        thread_numb[i] = i;
        pthread_create(&thread[i], NULL, consumer, &thread_numb[i]);
    }

    while(!(flag && (empty_sem_pointer->val == 0)));

    for (i = 0; i < threads; i++)
        pthread_cancel(thread_numb[i]);

    for (i = 0; i < threads; i++)
        pthread_join(thread[i], NULL);
 
    pthread_mutex_destroy(buffer_mutex_pointer);
    my_sem_destroy(full_sem_pointer);
    my_sem_destroy(empty_sem_pointer);
 
    return 0;
}