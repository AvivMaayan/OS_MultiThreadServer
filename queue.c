#include "queue.h"
#include <time.h>

struct Node {
    time_t time;
    int info;
    Node next;
};

struct Queue {
    int size;
    int max; 
    Node last;
};

Node nodeCreate(int info) {
    Node node = (Node)malloc(sizeof(*node));
    time(&node->time); 
    node->info = info;
    node->next = NULL;
    printf("Created node %d\n", info);
    return node;
}

int getInfo(Node orig) {
    return orig->info;
}

int getTime(Node orig) {
    return orig->time;
}

// replaces orig's next with the next provided
// as an argument. 
// returns the removed node.
Node setNext(Node orig, Node next) {
    Node removed = orig->next;
    orig->next = next;
    return removed;
}

Node getNext(Node orig) {
    return orig->next;
}

Queue queueCreate(int max_size) {
    Queue queue = (Queue)malloc(sizeof(*queue));
    queue->size = 0;
    queue->max = max_size;
    queue->last = NULL;
    printf("Created queue with max size %d\n", max_size);
    return queue;
}

bool queueIsEmpty(Queue queue) {
    return queue->size == 0; 
}

bool queueIsFull(Queue queue) {
    return queue->size == queue->max;
}

// removes the oldest node in the queue and returns its info
int queuePop(Queue queue) {
    if(queueIsEmpty(queue)) {
        return -1;
    }
    Node temp = queue->last;
    Node old_node = queue->last;
    temp = getNext(temp);
    while(temp != NULL) {
        if(temp->time < old_node->time) {
            old_node = temp;
        }
        temp = getNext(temp);
    }
    int result = old_node->info;
    queueRemove(queue, result);
    return result;
}

void queuePush(Queue queue, int info) {
    if(queueIsFull(queue)) {
        return;
    }

    Node new_node = nodeCreate(info);
    if(queueIsEmpty(queue)) {
        queue->last = new_node;
    }
    else {
        Node temp = queue->last;
        queue->last = new_node;
        setNext(queue->last, temp);
    }
    printf("Queue added %d\n", info);
    queue->size++;

}

void queueRemove(Queue queue, int to_remove) {
    if(queueIsEmpty(queue)) {
        return;
    }
    Node previous = queue->last;
    Node to_delete = queue->last;
    if(to_delete->info == to_remove) {
        // the last is the one to remove. 
        queue->last = getNext(queue->last);
    }
    else {
        to_delete = getNext(to_delete);
        while(to_delete != NULL) {
            if(to_delete->info == to_remove) {
                setNext(previous, getNext(to_delete));
                break;
            }
            to_delete = getNext(to_delete);
            previous = getNext(previous);
        }
    }
    free(to_delete);
    queue->size--;
    printf("Queue removed %d\n", to_remove);
}

void queueDestroy(Queue queue) {
    Node temp = queue->last;
    Node next = NULL;
    while(temp != NULL) {
        next = temp->next;
        free(temp);
        temp = next;
    }
    free(queue);
}

void queuePrint(Queue queue) {
    Node temp = queue->last;
    while(temp != NULL) {
        printf("%d, ", temp->info);
        temp = getNext(temp);
    }
}