#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXCONNECTIONS 13

#define HEALTHY 0
#define WITHERING 1
#define WATERED 2

typedef struct msg_struct {
    int flower_statuses[40];
    int id;
    int author_id;
} message;

void endError(char *message) {
    perror(message);
    exit(1);
}

int TCPAccept(int server_socket) {
    int client_socket;
    struct sockaddr_in client_addr;
    unsigned int client_len = sizeof(client_addr);
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        endError("failed accept client");
    }

    printf("Handling client %s\n", inet_ntoa(client_addr.sin_addr));

    return client_socket;
}

int TCPCreateSocket(unsigned short port) {
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        endError("socket() failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        endError("bind() failed");
    }

    if (listen(server_socket, MAXCONNECTIONS) < 0) {
        endError("listen() failed");
    }

    return server_socket;
}

void TCPHandle(int client_socket) {
    char msg[1024] = {0};
    int recieved_size;

    while (1) {
        if ((recieved_size = recv(client_socket, msg, 1024, 0)) < 0) {
            endError("recv() failed");
        }
        if (recieved_size == 0) {
            break;
        }
        printf("Response: %s\n", msg);
        memset(msg, 0, 1024);
    }
    printf("Leaving connection\n");
    close(client_socket);
}