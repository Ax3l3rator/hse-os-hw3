#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../essentials.h"
#define PORT 12345

int main(int argc, char *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char input[1024] = {0};
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    char handshake_message[2] = {'G', '0'};
    send(sock, handshake_message, 2, 0);
    if (recv(sock, handshake_message, 1, 0) <= 0) {
        endError("Err during handshake");
    }
    printf("Handshake happened");

    while (1) {
        // printf("Enter message to send (type 'quit' to exit): ");

        input[strlen(input) - 1] = '\0';
        if (strcmp(input, "quit") == 0) {
            break;
        }

        if (send(sock, input, strlen(input), 0) <= 0) {
            printf("Server unreachable\n");
            close(sock);
            exit(0);
        }

        printf("Message sent\n");
    }

    close(sock);
    return 0;
}
