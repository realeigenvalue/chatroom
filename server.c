#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

void write_to_clients(const char *message, int len);
void *processClient(void *arg);
int create_socket();
void bind_socket(const char *port);
void listen_socket();
void close_server(int exit_status);
void add_socketFD(int client);
void remove_socketFD(int client);
void signal_handler(int SIG);

int endingServer;
int serverSocket;
int clients[MAX_CLIENTS]; //-1 free spot holds all the socket file descriptors
int clientsConnected;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t num_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t attr; //thread attribute
struct sigaction sa;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        close_server(EXIT_FAILURE);
    }
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    memset(clients, -1, sizeof(int) * MAX_CLIENTS);
    serverSocket = create_socket();
    bind_socket(argv[1]);
    listen_socket();
    int client_socket; pthread_t tid;
    while(1) {
        if((client_socket = accept(serverSocket, NULL, NULL)) < 0) {
            perror(NULL);
            close_server(EXIT_FAILURE);
        }
        pthread_mutex_lock(&num_mutex);
        if(clientsConnected == MAX_CLIENTS) {
            write(client_socket, REJECT, strlen(REJECT));
            shutdown(client_socket, SHUT_RDWR);
            close(client_socket);
        } else {
            clientsConnected++;
            add_socketFD(client_socket);
            pthread_create(&tid, &attr, processClient, (void *)(intptr_t)client_socket);
        }
        pthread_mutex_unlock(&num_mutex);
    }
    close_server(EXIT_SUCCESS);
}

// DO NOT MODIFY THIS FUNCTION
void write_to_clients(const char *message, int len) {
    pthread_mutex_lock(&write_mutex);
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != -1) {
            write(clients[i], message, len);
        }
    }
    pthread_mutex_unlock(&write_mutex);
}

// DO NOT MODIFY THIS FUNCTION (BUT DO USE IT)
void *processClient(void *arg) {
    int client_fd = (intptr_t)arg;
    int client_is_connected = 1;
    while (client_is_connected) {
        char buffer[MSG_SIZE];
        int len = 0;
        int num;
        // Read until client sends eof or \n is read
        while (1) {
            num = read(client_fd, buffer + len, MSG_SIZE);
            len += num;
            if (!num) {
                client_is_connected = 0;
                break;
            }
            if (buffer[len - 1] == '\n')
                break;
        }
        // Error or client closed the connection, so time to close this specific
        // client connection
        if(!client_is_connected) {
            printf("User %d left\n", client_fd);
            break;
        }
        write_to_clients(buffer, len);
    }
    remove_socketFD(client_fd);
    close(client_fd);
    pthread_mutex_lock(&num_mutex);
    clientsConnected--;
    pthread_mutex_unlock(&num_mutex);
    return NULL;
}

int create_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

void bind_socket(const char *port) {
    struct addrinfo hints, *other_socket_info;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(NULL, port, &hints, &other_socket_info);
    if(s != 0) {
        fprintf(stderr, "%s\n", gai_strerror(s));
        freeaddrinfo(other_socket_info);
        close_server(EXIT_FAILURE);
    }
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    //bind the socket to the port
    if(bind(serverSocket, other_socket_info->ai_addr, other_socket_info->ai_addrlen) != 0) {
        perror(NULL);
        freeaddrinfo(other_socket_info);
        close_server(EXIT_FAILURE);
    }
    freeaddrinfo(other_socket_info);
}

void listen_socket() {
    //listen for connections sets queue size = 8
    if (listen(serverSocket, MAX_CLIENTS) != 0) {
        perror(NULL);
        close_server(EXIT_FAILURE);
    }
}

void close_server(int exit_status) {
    // Use a signal handler to call this function and close the server
    if(serverSocket != 0) {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
    }
    if(exit_status == 0) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}

void add_socketFD(int client) {
    int i = 0;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == -1) { //found an open spot
            clients[i] = client;
            break;
        }
    }
}

void remove_socketFD(int client) {
    int i = 0;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == client) { //found client to free
            clients[i] = -1;
            break;
        }
    }
}

void signal_handler(int SIG) {
    if(SIG == SIGINT) {
        close_server(EXIT_SUCCESS);
    }
}