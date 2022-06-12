#include "queue.h"

struct node {
    time_t creation;
    int info;
    node next;
};

struct queue {
    int size;
    node first; 
    node last;
};



