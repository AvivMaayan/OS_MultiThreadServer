#ifndef QUEUE_H 
#define QUEUE_H

// #include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct Queue *Queue;
typedef struct Node *Node; 

// This is Node's methods right there:

Node nodeCreate(int info, struct timeval _time);

int getInfo(Node orig);

struct timeval getTime(Node orig);

Node setNext(Node orig, Node next);

Node getNext(Node orig);

// Those are Queue's methods mortyyyyy

Queue queueCreate(int max_size);

bool queueIsEmpty(Queue queue);

int queueGetSize(Queue queue);

bool queueIsFull(Queue queue);

int queuePop(Queue queue);

int queueGetFD(Queue queue);

struct timeval queueGetTime(Queue queue);

void queuePush(Queue queue, int info);

void queueRemove(Queue queue, int to_remove);

void queueDestroy(Queue queue);

void queuePrint(Queue queue);

void queueDropAmountRandomly(Queue queue, int amount);

#endif //QUEUE_H