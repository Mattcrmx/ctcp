#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"


#define PORT 8888
#define SOCKET_BACKLOG 3
#define SIMULTANEOUS_CLIENTS 5


int main(void) {
    int server_socket = get_socket(PORT, SOCKET_BACKLOG);
    int client_socket;
    int clients[SIMULTANEOUS_CLIENTS];
    int *connected_clients = calloc(SIMULTANEOUS_CLIENTS, sizeof(int));

    if (connected_clients == NULL) {
        fprintf(stderr, "Failed to allocate connected clients array.");
    }


    while (true) {
        if ((client_socket = wait_for_client_connect(&server_socket)) != 1) {
            handle_client_connect(clients, client_socket, SIMULTANEOUS_CLIENTS);
        }
    }


    return 1;
}