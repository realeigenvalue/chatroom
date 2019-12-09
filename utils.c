#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "utils.h"
#include "chat_window.h"

extern int serverSocket;

int buffer_len;
char* buffer = NULL;
char* msg=NULL;

void write_cleanup_handler(void* arg)
{
     if ( arg){
	free( arg);
	arg = NULL;
     }

     free( buffer);
     buffer = NULL; msg = NULL;
}

void* write_to_server(void* arg){
    char* name = (char*)arg;
    int name_len = strlen(name);

    pthread_cleanup_push(write_cleanup_handler, NULL);

    while (1)
    {
        read_message_from_screen(&buffer);
        if (buffer == NULL)
            break;
        buffer_len = strlen(buffer);
        msg = calloc(1, buffer_len + name_len + 4);
        sprintf(msg, "%s: %s\n", name, buffer);
        write(serverSocket, msg, buffer_len + name_len + 3);
        free(msg);
    }

    free(buffer);
	pthread_cleanup_pop(0);
    return 0;
}

void* read_from_server(){

    int connected = 1;
    while (connected ){
        char buffer[MSG_SIZE];
        int len = 0;  int num;
        while (1)
        {
            num = read(serverSocket, buffer+len, MSG_SIZE);
            // write_message_to_screen("%d\n", num);
            len += num;

            if (!num || num == -1){
                connected = 0;
                break;
            }
            if (buffer[len-1] == '\n')
                break;
        }
        if (connected){
            buffer[len-1] = '\0';
            write_message_to_screen("%s\n", buffer);
        }
    }
    return 0;
}