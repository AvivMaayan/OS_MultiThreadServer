#include "../queue.h"
#include <unistd.h>
#define THEWALL(x) printf("---------------- %s ----------------\n", x); \
queuePrint(q); \
printf("\n"); 


int main()
{
    Queue q = queueCreate(20);
    for(int i=1; i<=10; i++) {
        queuePush(q, i);
        // sleep(1);
    }
    THEWALL("1.1 is full")
    printf("Q is supposed to be not full: %d\n", queueIsFull(q));

    THEWALL("1.2 pop randomly")
    queueDropAmountRandomly(q, 3);
    THEWALL("1.3 remove specific 11")
    printf("remove 11:");
    queueRemove(q, 11);

    THEWALL("1.4 top")
    printf("top is:");
    printf("%d",queueGetFD(q));

    THEWALL("1.5 remove top")
    queuePop(q);

    THEWALL("1.6 top2")
    printf("top is:");
    printf("%d",queueGetFD(q));

    THEWALL("1.7 remove top2")
    queuePop(q);

    THEWALL("")
    queueDestroy(q);

    return 0;
}