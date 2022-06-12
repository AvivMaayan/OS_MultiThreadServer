#include "queue.h"

struct Node {
    time_t time;
    int info;
    node next;
};

struct Queue {
    int size;
    int max; 
    Node last;
};

Node nodeCreate(int info) {
    Node node = (Node)malloc(sizeof(*node));
    node->time = time(NULL); 
    node->info = info;
    node->next = NULL;
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
}

Queue queueIsEmpty(Queue queue) {
    return queue->size == 0; 
}

Queue queueIsFull(Queue queue) {
    return queue->size == queue->max;
}

// removes the oldest node in the queue and returns its info
int queueGetBack(Queue queue) {
    if(queueIsEmpty(queue)) {
        return -1;
    }
    Node temp = queue->last;
    Node old_node = queue->last;
    temp = temp->getNext();
    while(temp != NULL) {
        if(temp->time < old_node->time) {
            old_node = temp;
        }
        temp = temp.getNext();
    }
    int result = old_node->info;
    queueRemove(queue, old_node);
    return result;
}

void queuePushBack(Queue queue, int info) {
    if(queueIsFull()) {
        return;
    }

    Node new_node = nodeCreate(info);
    if(queueIsEmpty()) {
        queue->last = new_node;
    }
    else {
        Node temp = queue->last;
        queue->last = new_node;
        setNext(queue->last, temp);
    }

    queue->size++;

}

void queueRemove(int to_remove) {
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
                setNext(previous, to_delete->getNext());
                break;
            }
            to_delete = to_delete.getNext();
            previous = previous.getNext();
        }
    }
    free(to_delete);
    queue->size--;
    printf("removed %d", to_remove);
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