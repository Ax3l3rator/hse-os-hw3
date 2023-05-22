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

    int flowers[40];
    for (int i = 0; i < 40; ++i) {
        flowers[i] = 0;
    }

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

    char handshake_message[2] = {'F', '0'};
    send(sock, handshake_message, 2, 0);
    int mode = 0;
    if (recv(sock, &mode, sizeof(mode), 0) < 0) {
        endError("Err during handshake");
    }
    printf("Handshake happened\n");

    if (mode != FLOWERBED_MODE) {
        endError("WRONG MODE");
    }
    char a[1] = {'-'};
    if (recv(sock, a, 1, 0) < 0) {
        endError("failed to recv");
    }

    if (a[0] != '+') {
        printf("No start\n");
        close(sock);
        exit(0);
    }

    message *msg = (message *)malloc(sizeof(message));
    message *inner_msg = (message *)malloc(sizeof(message));
    int amount = 1;
    while (1) {
        if (amount) {
            int id = rand() % 40;
            if (flowers[id] != STATUS_WITHERING) {
                flowers[id] = STATUS_WITHERING;
                inner_msg->action = WITHER_FLOWER;
                inner_msg->flower_id = id;
                printf("Sending withering flower %d\n", id);
                send(sock, inner_msg, sizeof(inner_msg), 0);
                printf("Sent: {action: %d, flower_id: %d}\n", inner_msg->action,
                       inner_msg->flower_id);
            }
            amount = 0;
        }

        int mlen;
        printf("Waiting\n");
        if ((mlen = recv(sock, msg, sizeof(message), 0)) < 0) {
            endError("oopse");
        }
        if (mlen == 0) {
            break;
        }
        printf("Received:{action: %d, flower_id: %d} \n", msg->action, msg->flower_id);
        if (msg->action != 0) {
            printf("Received:{action: %d, flower_id: %d} \n", msg->action, msg->flower_id);
            if (msg->action == GET_FLOWER_STATUS) {
                inner_msg->action = flowers[msg->flower_id] + 5;
                inner_msg->flower_id = msg->flower_id;
                send(sock, inner_msg, sizeof(message), 0);
            } else if (msg->action == WATER_FLOWER) {
                flowers[msg->flower_id] = STATUS_WATERING;
            } else if (msg->action == WATERED_FLOWER) {
                flowers[msg->flower_id] = STATUS_HEALTHY;
                amount = 1;
            }
        }
    }

    close(sock);
    return 0;
}
