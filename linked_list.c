#include<stdio.h>
#include<stdlib.h>
 
struct node
{
    int job_size;
    int process_id;
    struct node *next;
}*head;
 
 
 
void append(struct node num)
{
    struct node *temp,*right;
    temp= (struct node *)malloc(sizeof(struct node));
    temp->job_size=num.job_size;
    temp->process_id = num.process_id;
    right=(struct node *)head;
    while(right->next != NULL)
    right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}

int count()
{
    struct node *n;
    int c=0;
    n=head;
    while(n!=NULL)
    {
    n=n->next;
    c++;
    }
    return c;
}
 
void add(struct node num)
{
    struct node *temp;
    temp=(struct node *)malloc(sizeof(struct node));
    temp->job_size=num.job_size;
    temp->process_id = num.process_id;
    if (head == NULL)
    {
    head=temp;
    head->next=NULL;
    }
    else
    {
    temp->next=head;
    head=temp;
    }
}
void addafter(struct node num, int loc)
{
    int i;
    struct node *temp,*left,*right;
    right=head;
    for(i=1;i<loc;i++)
    {
    left=right;
    right=right->next;
    }
    temp=(struct node *)malloc(sizeof(struct node));
    temp->job_size=num.job_size;
    temp->process_id = num.process_id;
    left->next=temp;
    left=temp;
    left->next=right;
    return;
}
 
 
 
void insert(struct node num)
{
    int c=0;
    struct node *temp;
    temp=head;
    if(temp==NULL)
    {
    add(num);
    }
    else
    {
    while(temp!=NULL)
    {
        if(temp->job_size < num.job_size)
        c++;
        temp=temp->next;
    }
    if(c==0)
        add(num);
    else if(c<count())
        addafter(num,++c);
    else
        append(num);
    }

    // struct node *findNode = head;
    // if(findNode->job_size == -1) {
    //     findNode->job_size = num.job_size;
    //     findNode->process_id = num.process_id;
    // } else {
    //     while(findNode != NULL) {
    //         if(findNode->job_size < num.job_size) {
    //             findNode->job_size = num.job_size;
    //             findNode->process_id = num.process_id;
    //             return;
    //         }
    //         findNode = findNode->next;
    //     }
    // }
}

struct node delete()
{
    struct node *temp, *prev;
    struct node process;
    process.process_id = head->process_id;
    process.job_size = head->job_size;
    temp = head;
    head=temp->next;
    free(temp);
    return process;
}
 
 
void  display(struct node *r)
{
    r=head;
    if(r==NULL)
    {
    return;
    }
    while(r!=NULL)
    {
    printf("%d ",r->job_size);
    r=r->next;
    }
    printf("\n");
}

// int main() {
//     struct node *n;
//     head=NULL;

//     struct node test;
//     test.process_id = 5;
//     test.job_size = 1;
    
//     struct node test2;
//     test2.process_id = 4;
//     test2.job_size = 6;

//     struct node test3;
//     test2.process_id = 3;
//     test3.job_size = 2;

//     struct node test4;
//     test2.process_id = 2;
//     test4.job_size = 10;
//     insert(test);
//     insert(test2);
//     insert(test3);
//     insert(test4);

//     display(n);
// }
 