#ifndef __REQUEST_H__

struct stat_s
{
    struct timeval arrival_time;
    struct timeval service_time;
    struct timeval dispatch_interval;
    int handler_thread_id;
    int handler_thread_req_count;
    int handler_thread_static_req_count;
    int handler_thread_dynamic_req_count;
};

typedef enum 
{
    REQ_ERROR,
    REQ_STATIC,
    REQ_DYNAMIC,
} req_handle_res;

req_handle_res requestHandle(int fd, struct stat_s *stats);

#endif
