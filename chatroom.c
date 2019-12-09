#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#include "chat_window.h"
#include "client.h"

typedef struct info_t {
  char *host;
  char *port;
  char *username;
} info_t;

info_t *info;
void *clientRetVal;
pthread_t client;

void close_program(int signal) {
  if (signal == SIGINT) {
    close_chat();
    close_client();
  }
}

int main(int argc, char **argv) {

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <address> <port> <username>\n", argv[0]);
    exit(1);
  }

  info = malloc(sizeof(info_t));
  info->host = argv[1];
  info->port = argv[2];
  info->username = argv[3];

  signal(SIGINT, close_program);
  create_windows();

  pthread_create(&client, NULL, run_client, (void *)info);

  pthread_join(client, &clientRetVal);
  write_message_to_screen("joining client\n");

  write_message_to_screen("closing chatroom\n");
  free(info);

  destroy_windows();
  return 0;
}