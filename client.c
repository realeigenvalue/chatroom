#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "chat_window.h"
#include "utils.h"

void close_client();
void *run_client(void *information);
int create_socket();
int connect_socket(const char *address, const char *port);
void shutdown_cleanup();

// This is useful
typedef struct info_t {
    char *host;
    char *port;
    char *username;
} info_t;

int serverSocket;
pthread_t threads[2]; //0->used for reading, 1->used for writing
void *writeVal, *readVal;

void close_client() {
    pthread_cancel(threads[0]);
    pthread_join(threads[0], &writeVal);
    pthread_cancel(threads[1]);
    pthread_join(threads[1], &readVal);
    shutdown_cleanup();
}

void *run_client(void *information) {
    info_t *client_info = (info_t *)information;
    serverSocket = create_socket();
    if(connect_socket(client_info->host, client_info->port)) {
        pthread_create(&threads[0], NULL, write_to_server, client_info->username);
        pthread_create(&threads[1], NULL, read_from_server, NULL);
    } else {
        shutdown_cleanup();
        return NULL;
    }
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    return NULL;
}

int create_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

int connect_socket(const char *address, const char *port) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int s = getaddrinfo(address, port, &hints, &result); 
    if(s != 0) {
        shutdown_cleanup(); freeaddrinfo(result);
        fprintf(stderr, "%s\n", gai_strerror(s));
        exit(1);//return 0;
    }
    if(connect(serverSocket, result->ai_addr, result->ai_addrlen) < 0) {
        shutdown_cleanup(); freeaddrinfo(result);
        perror(NULL);
        exit(1);//return 0;
    }
    freeaddrinfo(result);
    return 1;
}

void shutdown_cleanup() {
    if(serverSocket != 0) {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
    }
}