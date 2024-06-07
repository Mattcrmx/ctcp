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
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_option,	sizeof(socket_option)) != 0) {
        printf("Cant' apply option: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // init socket
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = INADDR_ANY;  // listen to all addresses

    // bind socket
    if (bind(socket_fd, (struct sockaddr *) &socket_addr, sizeof(socket_addr))) {
        fprintf(stderr, "Failed to bind socket to port %d: %s", port, strerror(errno));
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
    int client_socket = accept(*server_socket_fd, (struct sockaddr *) &client_socket_addr, (socklen_t*) &client_addr_length);
    int socket_opt = 1;

    // accept the connection on the client socket and log it
    if (client_socket != -1) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_socket_addr.sin_addr), ip, INET_ADDRSTRLEN);
        fprintf(stderr, "Accepted connection on %s", ip);

        // non blocking socket
		setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &socket_opt, sizeof(socket_opt));
    }

    return client_socket;
}


int handle_client_connect(int *clients, int client_socket, int simultaneous_clients) {
    int can_accept = false;
    for (int idx = 0; idx < simultaneous_clients; idx++) {
        // check free slots
        if (clients[idx] == 0) {
            clients[idx] = client_socket;
            send(client_socket, "Type Exit to quit\n",strlen("Type Exit to quit\n"),MSG_DONTWAIT);
            can_accept = true;
            break;
        }
    }
    if (!can_accept) {
        fprintf(stderr, "All sockets are taken");
        close(client_socket);
    }
    return 1;
}
