#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "linked_list.c"

key_t keyLinkedList;
int shmidFour;
struct node *sharedLinkedList;

int main()
{
    keyLinkedList = 3940;
	if ((shmidFour = shmget(keyLinkedList, sizeof(struct node)*25, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

    if ((sharedLinkedList = shmat(shmidFour, NULL, 0)) == (struct node *)-1)
    {
        perror("shmat");
		exit(1);
    }

    int i;
    int flag = 0;
    
    struct node *head = sharedLinkedList;
    for (i = 0; i < 2; i++)
    {
        struct node *temp;
        temp=(struct node *)malloc(sizeof(struct node));

        temp->job_size = -1;
        temp->process_id = -1;
        temp->next = NULL;
        sharedLinkedList->next = temp;
        sharedLinkedList = temp;
    }

    struct node* r=head;

    struct node hello;
    hello.job_size = -2;
    hello.process_id = -2;

    if(fork() == 0) {
        insert(hello);
        exit(0);
    }

    wait(NULL);

    int j = 0;
    while (r != NULL)
    {
    printf("%d ", r->job_size);
    r=r->next;
    j++;
    }
    printf("\n");
    return 0;
}