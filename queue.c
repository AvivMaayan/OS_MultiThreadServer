#include "queue.h"
#include <time.h>
#include <sys/time.h>

struct Node
{
    int info;
    Node next;
    struct timeval time;
};

struct Queue
{
    int size;
    int max;
    Node first;
    Node last;
};

Node nodeCreate(int info, struct timeval _time)
{
    Node node = (Node)malloc(sizeof(*node));
    node->info = info;
    node->next = NULL;
    node->time = _time;
    DEBUG_PRINTF("Created node %d\n", info);
    return node;
}

int getInfo(Node orig)
{
    return orig->info;
}

struct timeval getTime(Node orig)
{
    return orig->time;
}

// replaces orig's next with the next provided
// as an argument.
// returns the removed node.
Node setNext(Node orig, Node next)
{
    Node removed = orig->next;
    orig->next = next;
    return removed;
}

Node getNext(Node orig)
{
    return orig->next;
}

Queue queueCreate(int max_size)
{
    Queue queue = (Queue)malloc(sizeof(*queue));
    queue->size = 0;
    queue->max = max_size;
    queue->last = NULL;
    queue->first = NULL;
    return queue;
}

bool queueIsEmpty(Queue queue)
{
    return queue->size == 0;
}

int queueGetSize(Queue queue)
{
    return queue->size;
}

bool queueIsFull(Queue queue)
{
    if (queue->size == queue->max)
        return true;
    else
        return false;
}

// removes the oldest node in the queue and returns its info
int queuePop(Queue queue)
{
    if (queueIsEmpty(queue))
    {
        return -1;
    }
    Node temp = queue->first->next;
    int result = queue->first->info;
    free(queue->first);
    if (temp == NULL)
    {
        queue->first = NULL;
        queue->last = NULL;
    }
    else
    {
        queue->first = temp;
    }
    return result;
}

int queueGetFD(Queue queue)
{
    if (queueIsEmpty(queue))
    {
        return -1;
    }
    return queue->first->info;
}

struct timeval queueGetTime(Queue queue)
{
    if (queueIsEmpty(queue))
    {
        return (struct timeval){0};
    }
    return queue->first->time;
}

void queuePush(Queue queue, int info, struct timeval arrival)
{
    if (queueIsFull(queue))
    {
        return;
    }

    Node new_node = nodeCreate(info, arrival);
    if (queueIsEmpty(queue))
    {
        queue->first = new_node;
        queue->last = new_node;
    }
    else
    {
        queue->last->next = new_node;
        queue->last = new_node;
    }
    queue->size++;
}

/*
void queueRemove(Queue queue, int to_remove)
{
    if (queueIsEmpty(queue))
    {
        return;
    }
    Node previous = queue->last;
    Node to_delete = queue->last;
    if (to_delete->info == to_remove)
    {
        // the last is the one to remove.
        queue->last = getNext(queue->last);
        if (queue->first->info == to_remove)
        { // this is also the first node!
            queue->first = NULL;
        }
    }
    else
    {
        to_delete = getNext(to_delete);
        while (to_delete != NULL)
        {
            if (to_delete->info == to_remove)
            {
                setNext(previous, getNext(to_delete));
                if (queue->first == to_delete)
                {
                    // this is the head of the queue
                    queue->first = previous;
                }
                break;
            }
            to_delete = getNext(to_delete);
            previous = getNext(previous);
        }
    }
    free(to_delete);
    queue->size--;
    DEBUG_PRINTF("Queue removed %d\n", to_remove);
}
*/

int queueIndex(Queue queue, int info)
{
    if (queueIsEmpty(queue))
    {
        return -1;
    }
    Node temp = queue->first;
    int index = 0;
    while (temp)
    {
        if (info == temp->info)
        {
            return index;
        }
        index++;
        temp = temp->next;
    }
    return -1;
}

void queueDestroy(Queue queue)
{
    Node temp = queue->last;
    Node next = NULL;
    while (temp != NULL)
    {
        next = temp->next;
        free(temp);
        temp = next;
    }
    free(queue);
}

void queuePrint(Queue queue)
{
    Node temp = queue->last;
    while (temp != NULL)
    {
        DEBUG_PRINTF("%d, ", temp->info);
        temp = getNext(temp);
    }
}

int queueRemoveByPlace(Queue queue, int place)
{
    if (queueIsEmpty(queue) || queue->size < place || place < 0)
    {
        return -1;
    }
    Node previous = NULL;
    Node to_delete = queue->first;
    int counter = 0;
    if (place == counter)
    {
        return queuePop(queue);
    }
    else
    {
        for (int i = 0; i < place; i++)
        {
            previous = to_delete;
            to_delete = to_delete->next;
        }
    }
    int info = to_delete->info;
    previous->next = to_delete->next;
    free(to_delete);
    if(index == queue->size - 1){
        queue->last = previous;
    }
    queue->size--;
    return info;
}

void queueDropAmountRandomly(Queue queue, int amount)
{
    if (amount > queue->size)
    {
        amount = queue->size;
        DEBUG_PRINTF("PROBLEM! Trying to delete too many!");
    }
    for (int i = 0; i < amount; i++)
    {
        int rand_loc = rand() % (queue->size) + 1;
        queueRemoveByPlace(queue, rand_loc);
    }
}