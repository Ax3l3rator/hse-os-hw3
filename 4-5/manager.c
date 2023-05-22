#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../essentials.h"

#define PORT 12345
#define MAX_CLIENTS 10

int continueocity = 1;
int client_socket;
void end(int dummy) {
    continueocity = 0;
    close(client_socket);
    sleep(3);
    exit(0);
}

int main() {
    int server_socket;
    char buffer[1024] = {0};

    int sockets_of_gardeners[2];
    int flowerbed_socket;
    signal(SIGINT, end);
    server_socket = TCPCreateSocket(PORT);

    printf("Server started listening on port %d\n", PORT);

    int pid;
    int child_count = 0;

    while (continueocity) {
        client_socket = TCPAccept(server_socket);
        if ((pid = fork()) < 0) {
            endError("fork() failed");
        } else if (pid == 0) {
            close(server_socket);
            char handshake_message[2];
            int handshake_msg_len;
            printf("Waiting for handshake...\n");
            if ((handshake_msg_len = recv(client_socket, handshake_message, 2, 0)) < 0) {
                endError("Handshake error");
            }
            if (handshake_message[0] == 'G') {
                printf("Handling gardener %c\n", handshake_message[1]);
                sockets_of_gardeners[handshake_message[1] - 49] = client_socket;
            } else if (handshake_message[0] == 'F') {
                printf("Handling flowerbed\n");
                flowerbed_socket = client_socket;
            } else {
                endError("Wrong handshake");
            }

            TCPHandle(client_socket);

            printf("Connection leaved\n");

            exit(0);
        }

        printf("with child process: %d\n", (int)pid);
        close(client_socket);
        child_count++;

        while (child_count) {
            pid = waitpid((pid_t)-1, NULL, WNOHANG);
            if (pid < 0) {
                endError("waitpid() failed");
            } else if (pid == 0) {
                break;
            } else {
                child_count--;
            }
        }
    }

    return 0;
}
