#ifndef SERVER_H
#define SERVER_H

int get_socket(int port, int backlog);
int wait_for_client_connect(int *server_socket_fd);
int handle_client_connect(int *clients, int client_socket,
                          int simultaneous_clients);

#endif // SERVER_H
