#include "queue.h"
#include <unistd.h>
#define THEWALL(x) printf("---------------- %s ----------------\n", x); \
queuePrint(q); 


int main()
{
    Queue q = queueCreate(20);
    for(int i=1; i<=20; i++) {
        queuePush(q, i);
        sleep(1);
    }
    THEWALL("1.1 is full")
    printf("Q is supposed to be full: %d\n", queueIsFull(q));

    THEWALL("1.2 pop back")
    printf("Bring me 1-10:\n");
    for(int i=1; i<=10; i++) {
        queuePop(q);
    }

    THEWALL("1.3 remove specific 11")
    printf("remove 11:");
    queueRemove(q, 11);

    THEWALL("1.4 remove specific 15")
    printf("remove 15:");
    queueRemove(q, 15);

    THEWALL("1.5 remove specific 19")
    printf("remove 19:");
    queueRemove(q, 19);

    queueDestroy(q);

    return 0;
}