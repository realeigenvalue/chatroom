// Maximum number of clients that can connect to server at the same time
#define MAX_CLIENTS 8

// Message sent to the new client trying to connect
// when the server is already hosting the maximum
// number of clients
#define REJECT "Server full\n"

// The largest size the message can be that a client
// sends to the server
#define MSG_SIZE 256