#include "segel.h"
#include "request.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

#define DROP_PERCENT 0.3
#define FD_CLOSED -1
enum
{
    STAT_STATIC,
    STAT_DYNAMIC,
    STAT_TOTAL,
    STAT_OPTIONS,
};
typedef enum
{
    SCH_BLOCK,
    SCH_DT,
    SCH_DH,
    SCH_RANDOM,
} sched_alg_t;

pthread_mutex_t queue_lock;
pthread_cond_t thread_ready_cond;
pthread_cond_t req_ready_cond;
int ready_threads;
int **thread_stats;
Queue busy_q, wait_q;

void initThreadArrays(int threads_num, pthread_t **threads, int **thread_ids)
{
    *threads = malloc(threads_num * sizeof(**threads));
    if (!*threads)
    {
        fprintf(stderr, "Failed to allocate threads array");
        exit(1);
    }
    *thread_ids = malloc(threads_num * sizeof(**thread_ids));
    if (!*thread_ids)
    {
        free(*threads);
        fprintf(stderr, "Failed to allocate thread ids array");
        exit(1);
    }
    for (int i = 0; i < threads_num; i++)
    {
        *thread_ids[i] = i;
        if (pthread_create(*threads + i, NULL, serviceRequests, (void *)(*thread_ids + i)))
        {
            free(*threads);
            free(*thread_ids);
            fprintf(stderr, "Failed to create thread #%d", i);
            exit(1);
        }
    }
    thread_stats = malloc(threads_num * sizeof(*thread_stats));
    if (!thread_stats)
    {
        free(*threads);
        free(*thread_ids);
        // TODO Kill threads
        fprintf(stderr, "Failed to allocate STAT table");
        exit(1);
    }
    for (int i = 0; i < threads_num; i++)
    {
        // Allocate and initialize to zeros
        thread_stats[i] = calloc(STAT_OPTIONS, sizeof(**thread_stats));
        if (!thread_stats[i])
        {
            free(*threads);
            free(*thread_ids);
            // TODO Kill threads
            for (int j = 0; j < i; j++)
            {
                free(thread_stats[i]);
            }
            fprintf(stderr, "Failed to allocate STAT table entry");
            exit(1);
        }
    }
}

// Initializes queues.
// In case of failure returns 1
int initQueues(int threads_num, int queue_size)
{
    int busy_q_max_size = (threads_num > queue_size) ? queue_size : threads_num;
    busy_q = queueCreate(busy_q_max_size);
    wait_q = queueCreate(queue_size);
    return (busy_q == NULL || wait_q == NULL);
}

// Initializes locks and conditions.
// In case of failure return value will be Non-Zero
int initLocks()
{
    ready_threads = 0;
    return pthread_mutex_init(&queue_lock, NULL) +
           pthread_mutex_init(&queue_lock, NULL) +
           pthread_cond_init(&req_ready_cond, NULL);
}

// HW3: Parse the new arguments too
void getArgs(int *port_num, int *threads_num, int *queue_size, sched_alg_t *schedalg, int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port_num = atoi(argv[1]);
    *threads_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    char *alg = atoi(argv[4]);
    if (strcmp(alg, "block"))
        *schedalg = SCH_BLOCK;
    else if (strcmp(alg, "dt"))
        *schedalg = SCH_DT;
    else if (strcmp(alg, "dh"))
        *schedalg = SCH_DH;
    else if (strcmp(alg, "random"))
        *schedalg = SCH_RANDOM;
    else
    {
        fprintf(stderr, "%s is not a recognized sched alg\n", alg);
        exit(1);
    }
}

void *serviceRequests(void *thread_id_ptr)
{
    int thread_id = *(int *)thread_id_ptr;
    while (1)
    {
        // Critical part start
        pthread_mutex_lock(&queue_lock);
        ready_threads++;
        pthread_cond_signal(&thread_ready_cond);
        while (queueIsEmpty(wait_q))
        {
            pthread_cond_wait(&queue_lock, &req_ready_cond);
            // Critical part end + start
        }
        int connfd = queueGetBack(wait_q);
        queuePushBack(busy_q, connfd);
        ready_threads--;
        pthread_mutex_unlock(&queue_lock);
        // Critical part end

        requestHandle(connfd);
        Close(connfd);

        // Critical part start
        pthread_mutex_lock(&queue_lock);
        queueRemove(wait_q, connfd);
        pthread_mutex_unlock(&queue_lock);
        // Critical part end
    }
    return NULL;
}

int isFull(int queue_size)
{
    return (queueGetSize(busy_q) + queueGetSize(wait_q)) == queue_size;
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, threads_num, queue_size, clientlen;
    struct sockaddr_in clientaddr;
    sched_alg_t schedalg;
    pthread_t *threads;
    int *thread_ids;

    getArgs(&port, &threads_num, &queue_size, &schedalg, argc, argv);
    initThreadArrays(threads_num, &threads, &thread_ids);
    if (initQueues(threads_num, queue_size))
    {
        free(threads);
        free(thread_ids);
        // TODO Kill all threads
        fprintf(stderr, "Failed to Initialize queues");
        exit(1);
    }
    if (initLocks())
    {
        free(threads);
        free(thread_ids);
        // TODO Kill all threads
        queueDestroy(busy_q);
        queueDestroy(wait_q);
        fprintf(stderr, "Failed to Initialize locks and conditions");
        exit(1);
    }

    // Start listening to port
    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

        // Add connfd to wait_q
        // If there is available thread(?)- send signal
        // If queues are full- handel according to schedalg

        pthread_mutex_lock(&queue_lock);
        // assuming "block" for now:
        if (isFull(queue_size))
        {
            switch (schedalg)
            {
            case SCH_BLOCK:
                while (isFull(queue_size))
                {
                    pthread_cond_wait(&thread_ready_cond, &queue_lock);
                }
                break;
            case SCH_DT:
                Close(connfd);
                connfd = FD_CLOSED;
                break;
            case SCH_DH:
                if (!queueIsEmpty(wait_q))
                {
                    queuePop(wait_q);
                }
                break;
            case SCH_RANDOM:
                if (!queueIsEmpty(wait_q))
                {
                    // The amount to drop is based on ALL requests
                    // But- they are dropped only from wait_q
                    int total = queueGetSize(wait_q) + queueGetSize(busy_q);
                    int amount = total * DROP_PERCENT;
                    queueDropAmountRandomly(amount);
                }
                break;
            default:
                break;
            }
        }
        if (connfd != FD_CLOSED)
        {
            queuePushBack(wait_q, connfd);
            pthread_cond_signal(&req_ready_cond);
        }
    }
}
