#include "myqueue.h"
#include <stdlib.h>

node_t* head = NULL;
node_t* tail = NULL;

void enqueue(int *connFd, char *root){
    node_t *newnode = malloc(sizeof(node_t));
    newnode->connFd = connFd;
    newnode->root = root;
    newnode->next = NULL;
    if(tail == NULL){
        head = newnode;
    }
    else{
        tail->next=newnode;
    }
    tail = newnode;
}


t_arg* dequeue(){
    if(head==NULL){
        return NULL;
    }else{
        //int *result = head->connFd;
        t_arg *args = malloc(sizeof(t_arg));
        args->connFd = head->connFd;
        args->root = head->root;
        node_t *temp = head;
        head = head->next;
        if(head == NULL) {tail = NULL;}
        free(temp);
        return args;
    }
}
