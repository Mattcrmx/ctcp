#include "sys/socket.h"
#include "netinet/in.h"
#include <complex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "fcntl.h"
#include "server.h"
#include <unistd.h>

#define BUFFER_SIZE 1024
#define EXIT_CMD "exit"

int get_socket(int port, int backlog) {
    // sock stream is TCP and AF_INET is ipv4
    int socket_fd;
    struct sockaddr_in socket_addr;
    int socket_option = 1;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "Failed to create Socket.");
        exit(EXIT_FAILURE);
    }

    // Reuse port and address
    socket_option = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &socket_option, sizeof(socket_option)) != 0) {
        printf("Cant' apply option: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // init socket
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = INADDR_ANY; // listen to all addresses

    // bind socket
    if (bind(socket_fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr))) {
        fprintf(stderr, "Failed to bind socket to port %d: %s", port,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // listen on the created socket
    if (listen(socket_fd, backlog) != 0) {
        printf("Failed to listen on socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set the socket to non blocking
    fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK);

    return socket_fd;
}

int wait_for_client_connect(int *server_socket_fd) {
    struct sockaddr_in client_socket_addr;
    unsigned int client_addr_length = sizeof(client_socket_addr);
    int client_socket =
        accept(*server_socket_fd, (struct sockaddr *)&client_socket_addr,
               (socklen_t *)&client_addr_length);
    int socket_opt = 1;

    // accept the connection on the client socket and log it
    if (client_socket != -1) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_socket_addr.sin_addr), ip, INET_ADDRSTRLEN);
        fprintf(stderr, "Accepted connection on %s\n", ip);

        // non blocking socket
        setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &socket_opt,
                   sizeof(socket_opt));
    }

    return client_socket;
}

int handle_client_connect(int *clients, int client_socket,
                          int simultaneous_clients) {
    int can_accept = false;
    for (int idx = 0; idx < simultaneous_clients; idx++) {
        // check free slots
        if (clients[idx] == 0) {
            clients[idx] = client_socket;
            send(client_socket, "Type exit to quit\n",
                 strlen("Type exit to quit\n"), MSG_DONTWAIT);
            can_accept = true;
            break;
        }
    }
    if (!can_accept) {
        fprintf(stderr, "All sockets are taken\n");
        close(client_socket);
    }
    return 1;
}

int echo_client(int client_socket) {
    char *client_msg = malloc(BUFFER_SIZE + 1);
    fprintf(stderr, "Echo-ing client's message at socket %d\n", client_socket);

    if (client_msg == NULL) {
        fprintf(stderr, "Failed to allocate memory for client message.\n");
    }

    int close_socket = 0;

    if (client_socket == 0) {
        free(client_msg);
        return -1;
    }
    int msg_length = recv(client_socket, client_msg, BUFFER_SIZE, 0);
    fprintf(stderr, "Received message %s", client_msg);

    if (msg_length == -1 && errno != EAGAIN) {
        fprintf(stderr, "Errno [%i] : %s\n", errno, strerror(errno));
        close_socket = 1;
    } else if (msg_length == 0) {
        // client disconnect causes msg len to be 0
        close_socket = 1;
    } else if (msg_length > 0) {
        client_msg[msg_length] = '\0';
        if (strncmp(client_msg, EXIT_CMD, 4) == 0) {
            // client disconnect
            char msg[] = "Disconnecting socket\n";
            send(client_socket, msg, strlen(msg), 0);
            close_socket = 1;
        } else {
            // return client msg
            char echo_msg[] = "echo: ";
            char *response = malloc(strlen(client_msg) + strlen(echo_msg) + 1);

            strcpy(response, echo_msg);
            strcat(response, client_msg);
            send(client_socket, response, strlen(response), 0);
        }
    }

    if (close_socket == 1) {
        close(client_socket);
        free(client_msg);
        return 1;
    }
    free(client_msg);
    return 0;
}
