#ifndef MYQUEUE_H_
#define MYQUEUE_H_

struct thread_args{
    int *connFd;
    char *root;
};
typedef struct thread_args t_arg;

struct node{
    struct node* next;
    int *connFd;
    char *root;
};
typedef struct node node_t;


void enqueue(int* connFd, char *root);
t_arg* dequeue();

#endif