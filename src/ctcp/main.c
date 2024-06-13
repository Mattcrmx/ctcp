#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

#define PORT 8888
#define SOCKET_BACKLOG 3
#define SIMULTANEOUS_CLIENTS 5

int main(void) {
    int server_socket = get_socket(PORT, SOCKET_BACKLOG);
    int *connected_clients = calloc(SIMULTANEOUS_CLIENTS, sizeof(int));

    if (connected_clients == NULL) {
        fprintf(stderr, "Failed to allocate connected clients array.");
    }

    while (true) {
        int client_socket;
        if ((client_socket = wait_for_client_connect(&server_socket)) != -1) {
            handle_client_connect(connected_clients, client_socket,
                                  SIMULTANEOUS_CLIENTS);
            echo_client(client_socket);
        }
    }

    return EXIT_SUCCESS;
}
