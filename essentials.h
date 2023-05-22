#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXCONNECTIONS 13

#define STATUS_HEALTHY 0
#define STATUS_WITHERING 1
#define STATUS_WATERING 2

#define END_CONVERSATION 0
#define WATER_FLOWER 1
#define WITHER_FLOWER 2
#define WATERED_FLOWER 3
#define GET_FLOWER_STATUS 4

#define RET_HEALTHY 5
#define RET_WITHERING 6
#define RET_WATERING 7

#define GARDENER1_MODE 1
#define GARDENER2_MODE 2
#define FLOWERBED_MODE 3

#define D_G1 1
#define D_G2 2
#define D_F 3

typedef struct msg_struct {
    short action;
    short flower_id;
    short dest;
} message;

typedef struct servmem {
    int to;
    message msg;
} server_memory;

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

    printf("Handling client %s with socket %d \n", inet_ntoa(client_addr.sin_addr), client_socket);

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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        endError("bind() failed");
    }

    if (listen(server_socket, MAXCONNECTIONS) < 0) {
        endError("listen() failed");
    }

    return server_socket;
}
