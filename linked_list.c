#include<stdio.h>
#include<stdlib.h>
#include "process_info.h"
 
struct node
{
    struct process_info data;
    struct node *next;
}*head;
 
 
 
void append(struct process_info num)
{
    struct node *temp,*right;
    temp= (struct node *)malloc(sizeof(struct node));
    temp->data=num;
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
 
void add(struct process_info num)
{
    struct node *temp;
    temp=(struct node *)malloc(sizeof(struct node));
    temp->data=num;
    if (head== NULL)
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
void addafter(struct process_info num, int loc)
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
    temp->data=num;
    left->next=temp;
    left=temp;
    left->next=right;
    return;
}
 
 
 
void insert(struct process_info num)
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
        if(temp->data.size < num.size)
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
}
 
 
 
struct process_info delete()
{
    struct node *temp, *prev;
    struct process_info process;
    process.process_id = head->data.process_id;
    process.size = head->data.size;
    temp = head;
    head=temp->next;
    free(temp);
    return process;
}
 
 
// void  display(struct node *r)
// {
//     r=head;
//     if(r==NULL)
//     {
//     return;
//     }
//     while(r!=NULL)
//     {
//     printf("%d ",r->data.size);
//     r=r->next;
//     }
//     printf("\n");
// }

// int main() {
//     struct node *n;
//     head=NULL;

//     struct process_info test;
//     test.process_id = 5;
//     test.size = 1;
    
//     struct process_info test2;
//     test2.process_id = 4;
//     test2.size = 6;

//     struct process_info test3;
//     test2.process_id = 3;
//     test3.size = 2;

//     struct process_info test4;
//     test2.process_id = 2;
//     test4.size = 10;
//     insert(test);
//     insert(test2);
//     insert(test3);
//     insert(test4);

//     display(n);
// }
 