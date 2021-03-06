#include "segel.h"
#include "request.h"
#include "queue.h"
#include <sys/time.h>
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
int **thread_stats;
Queue busy_q, wait_q;

void *serviceRequests(void *thread_id_ptr);

// Initializes queues.
// In case of failure returns 1
int initThreadArrays(int threads_num, pthread_t **threads, int **thread_ids)
{
    *threads = (pthread_t *)malloc(threads_num * sizeof(**threads));
    if (!*threads)
    {
        fprintf(stderr, "Failed to allocate threads array");
        return 1;
    }
    *thread_ids = (int *)malloc(threads_num * sizeof(**thread_ids));
    if (!*thread_ids)
    {
        free(*threads);
        fprintf(stderr, "Failed to allocate thread ids array");
        return 1;
    }
    for (int i = 0; i < threads_num; i++)
    {
        (*thread_ids)[i] = i;
        if (pthread_create(*threads + i, NULL, serviceRequests, (void *)(*thread_ids + i)))
        {
            free(*threads);
            free(*thread_ids);
            fprintf(stderr, "Failed to create thread #%d", i);
            return 1;
        }
    }
    thread_stats = malloc(threads_num * sizeof(*thread_stats));
    if (!thread_stats)
    {
        free(*threads);
        free(*thread_ids);
        // TODO Kill threads
        fprintf(stderr, "Failed to allocate STAT table");
        return 1;
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
            return 1;
        }
    }
    return 0;
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
    return pthread_mutex_init(&queue_lock, NULL) +
           pthread_cond_init(&thread_ready_cond, NULL) +
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
    char *alg = argv[4];
    if (!strcmp(alg, "block"))
        *schedalg = SCH_BLOCK;
    else if (!strcmp(alg, "dt"))
        *schedalg = SCH_DT;
    else if (!strcmp(alg, "dh"))
        *schedalg = SCH_DH;
    else if (!strcmp(alg, "random"))
        *schedalg = SCH_RANDOM;
    else
    {
        fprintf(stderr, "%s is not a recognized sched alg\n", alg);
        exit(1);
    }
    DEBUG_PRINTF("argv[4] is: %s, Alg is: %d\n", argv[4], *schedalg);
}

void *serviceRequests(void *thread_id_ptr)
{
    int thread_id = *(int *)thread_id_ptr;
    while (1)
    {
        struct stat_s stats;
        stats.handler_thread_id = thread_id;
        // Critical part start
        pthread_mutex_lock(&queue_lock);
        while (queueIsEmpty(wait_q))
        {
            pthread_cond_wait(&req_ready_cond, &queue_lock);
            // Critical part end + start
        }
        DEBUG_PRINTF("thread: got signal\n");
        /*
        int connfd = queueGetFD(wait_q);
        stats.arrival_time = queueGetTime(wait_q);
        queuePop(wait_q);
        */
        stats.arrival_time = queueGetTime(wait_q);
        int connfd = queuePop(wait_q);

        queuePush(busy_q, connfd, stats.service_time);
        pthread_mutex_unlock(&queue_lock);

        thread_stats[thread_id][STAT_TOTAL]++;

        // Filling in stats
        DEBUG_PRINTF("thread: Filling in stats\n");
        gettimeofday(&stats.service_time, NULL);
        timersub(&stats.service_time, &stats.arrival_time, &stats.dispatch_interval);
        stats.handler_thread_req_count = thread_stats[thread_id][STAT_TOTAL];
        stats.handler_thread_static_req_count = thread_stats[thread_id][STAT_STATIC];
        stats.handler_thread_dynamic_req_count = thread_stats[thread_id][STAT_DYNAMIC];

        req_handle_res res = requestHandle(connfd, &stats);
        Close(connfd);

        // Lock is not needed because each thread is changing his own cell
        if (res == REQ_STATIC)
            thread_stats[thread_id][STAT_STATIC]++;
        else if (res == REQ_DYNAMIC)
            thread_stats[thread_id][STAT_DYNAMIC]++;

        pthread_mutex_lock(&queue_lock);
        // queueRemove(busy_q, connfd);
        queueRemoveByPlace(busy_q, queueIndex(busy_q, connfd));
        pthread_cond_signal(&thread_ready_cond);

        pthread_mutex_unlock(&queue_lock);
    }
    return NULL;
}

int isFull(int queue_size)
{
    return (queueGetSize(busy_q) + queueGetSize(wait_q)) >= queue_size;
}

int main(int argc, char *argv[])
{
    DEBUG_PRINTF("Server: main\n");

    int listenfd, connfd, port, threads_num, queue_size, clientlen;
    struct sockaddr_in clientaddr;
    sched_alg_t schedalg;
    pthread_t *threads;
    int *thread_ids;

    DEBUG_PRINTF("Server: getArgs\n");
    getArgs(&port, &threads_num, &queue_size, &schedalg, argc, argv);
    DEBUG_PRINTF("Server: initQueues\n");
    if (initQueues(threads_num, queue_size))
    {
        fprintf(stderr, "Failed to Initialize queues");
        exit(1);
    }
    DEBUG_PRINTF("Server: initLocks\n");
    if (initLocks())
    {
        queueDestroy(busy_q);
        queueDestroy(wait_q);
        fprintf(stderr, "Failed to Initialize locks and conditions");
        exit(1);
    }
    DEBUG_PRINTF("Server: initThreadArrays\n");
    if (initThreadArrays(threads_num, &threads, &thread_ids))
    {
        queueDestroy(busy_q);
        queueDestroy(wait_q);
        pthread_mutex_destroy(&queue_lock);
        pthread_cond_destroy(&thread_ready_cond);
        pthread_cond_destroy(&req_ready_cond);
        exit(1);
    }
    DEBUG_PRINTF("Server: listen loop\n");
    // Start listening to port
    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        struct timeval arrival;
        gettimeofday(&arrival, NULL);
        pthread_mutex_lock(&queue_lock);

        if (isFull(queue_size))
        {
            DEBUG_PRINTF("Server: isFull\n");
            switch (schedalg)
            {
            case SCH_BLOCK:
                DEBUG_PRINTF("Case Block\n");
                while (isFull(queue_size))
                {
                    pthread_cond_wait(&thread_ready_cond, &queue_lock);
                }
                break;
            case SCH_DT:
                DEBUG_PRINTF("Case Dt\n");
                Close(connfd);
                connfd = FD_CLOSED;
                break;
            case SCH_DH:
                DEBUG_PRINTF("Case Dh\n");
                if (!queueIsEmpty(wait_q))
                {
                    Close(queuePop(wait_q));
                }
                else
                {
                    Close(connfd);
                    connfd = FD_CLOSED;
                }
                break;
            case SCH_RANDOM:
                DEBUG_PRINTF("Case Rand\n");
                if (!queueIsEmpty(wait_q))
                {
                    int amount = (int)(ceil(((double)(queueGetSize(wait_q))) * DROP_PERCENT));
                    if (amount > queueGetSize(wait_q))
                    {
                        amount = queueGetSize(wait_q);
                        DEBUG_PRINTF("PROBLEM! Trying to delete too many!");
                    }
                    for (int i = 0; i < amount; i++)
                    {
                        int rand_loc = rand() % (queueGetSize(wait_q));
                        Close(queueRemoveByPlace(wait_q, rand_loc));
                    }
                }
                else
                {
                    Close(connfd);
                    connfd = FD_CLOSED;
                }
                break;
            default:
                break;
            }
        }
        if (connfd != FD_CLOSED)
        {
            DEBUG_PRINTF("Server: Add to queue\n");
            queuePush(wait_q, connfd, arrival);
            DEBUG_PRINTF("Server: send signal\n");
            pthread_cond_signal(&req_ready_cond);
        }
        pthread_mutex_unlock(&queue_lock);
    }
    return 0;
}
