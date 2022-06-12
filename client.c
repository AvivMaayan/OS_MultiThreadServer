/*
 * client.c: A very, very primitive HTTP client.
 *
 * To run, try:
 *      ./client www.cs.technion.ac.il 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * HW3: For testing your server, you will want to modify this client.
 * For example:
 *
 * You may want to make this multi-threaded so that you can
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs;
 * you may want to get more URIs from the command line
 * or read the list from a file.
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "segel.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Send an HTTP request for the specified file
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];
  int length = 0;
  int n;

  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0))
  {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1)
    {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0)
  {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/**
 * my new stuff
 */

void *print_m(void *fd)
{
  // cast the thread argument
  int *clientfd = (int *)fd;
  printf("I'm a thread! PID = %d, pthread ID = %ld\n",
         getpid(), pthread_self());
  char a[MAXLINE];
  char b[MAXLINE];
  scanf("%2000s %2000[^\n]", a, b);
  // printf("%s\n", a);
  clientSend(*clientfd, a);
  clientPrint(*clientfd);
  printf("Thread number %ld is DEAD!!\n", pthread_self());
  pthread_exit(NULL);
  return NULL;
}

int main()
{
  char *host;
  int port;
  int clientfd;
  pthread_t t[10];
  int result, num_of_threads;
  host = "localhost";
  port = 2003;
  num_of_threads = 10;

  // open a new entry for the relevant port in the FDT
  clientfd = Open_clientfd(host, port);
  FILE *file = freopen("test_me.txt", "r", stdin);
  int fd = fileno(file);
  // create threads in a for loop and let them execute next line in the file
  // threads share FDT's
  for (int i = 0; i < num_of_threads; i++)
  {
    result = pthread_create(t+i, NULL, print_m, &fd);
    if (result)
    {
      printf("Error:unable to create thread number %d", i);
      exit(0);
    }
  }
  for(int i=0; i<num_of_threads; i++) {
    pthread_join(t[i], NULL);
  }
  Close(clientfd);
  Close(fd);
  return 0;
}

int main_orig(int argc, char *argv[])
{
  char *host, *filename;
  int port;
  int clientfd;

  if (argc != 4)
  {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];

  /* Open a single connection to the specified host and port */
  clientfd = Open_clientfd(host, port);

  clientSend(clientfd, filename);
  clientPrint(clientfd);

  Close(clientfd);

  exit(0);
}
