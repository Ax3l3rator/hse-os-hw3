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

    if (argc < 3) {
        printf("Usage: %s [ip_addr] [port]", argv[0]);
        return 1;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
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
    memset(msg, 0, sizeof(message));
    memset(inner_msg, 0, sizeof(message));
    int amount = 1;
    while (1) {
        if (amount) {
            int id = rand() % 40;
            if (flowers[id] != STATUS_WITHERING) {
                flowers[id] = STATUS_WITHERING;
                inner_msg->action = WITHER_FLOWER;
                inner_msg->flower_id = id;
                inner_msg->dest = 0;
                printf("Sending withering flower %d\n", id);
                send(sock, inner_msg, sizeof(message), 0);
                printf("Sent: {action: %d, flower_id: %d}\n", inner_msg->action,
                       inner_msg->flower_id);
            }
            amount = 0;
        }

        int mlen;
        if ((mlen = recv(sock, msg, sizeof(message), 0)) < 0) {
            endError("oopse");
        }

        if (mlen == 0) {
            break;
        }
        if (msg->action != 0) {
            if (msg->action == GET_FLOWER_STATUS) {
                printf("%d is getting flower status of %d\n", msg->dest, msg->flower_id);
                inner_msg->action = flowers[msg->flower_id] + 5;
                inner_msg->flower_id = msg->flower_id;
                printf("Sending %d status %d\n", inner_msg->dest, flowers[msg->flower_id]);
                printf("Sent: {action: %d, flower_id: %d}\n", inner_msg->action,
                       inner_msg->flower_id);
                inner_msg->dest = msg->dest;
                send(sock, inner_msg, sizeof(message), 0);
            } else if (msg->action == WATER_FLOWER) {
                flowers[msg->flower_id] = STATUS_WATERING;
                printf("%d is being watered by %d\n", msg->flower_id, msg->dest);
            } else if (msg->action == WATERED_FLOWER) {
                flowers[msg->flower_id] = STATUS_HEALTHY;
                printf("%d is watered by %d\n", msg->flower_id, msg->dest);
                sleep(1);
                amount = 1;
            }
        }
    }

    close(sock);
    return 0;
}

/*
    Sending withering flower 23
Sent: {action: 2, flower_id: 23}
2 is getting flower status of 23
Sending 2 status 1
Sent: {action: 6, flower_id: 23}
23 is being watered by 2
1 is getting flower status of 23
Sending 1 status 2
Sent: {action: 7, flower_id: 23}
23 is watered by 2
Sending withering flower 6
Sent: {action: 2, flower_id: 6}
2 is getting flower status of 6
Sending 2 status 1
Sent: {action: 6, flower_id: 6}
1 is getting flower status of 6
Sending 1 status 1
Sent: {action: 6, flower_id: 6}
6 is being watered by 2
6 is being watered by 1
6 is watered by 2
Sending withering flower 17
Sent: {action: 2, flower_id: 17}
6 is watered by 1
Sending withering flower 35
Sent: {action: 2, flower_id: 35}
2 is getting flower status of 17
Sending 2 status 1
Sent: {action: 6, flower_id: 17}
35 is being watered by 2
1 is getting flower status of 17
Sending 1 status 1
Sent: {action: 6, flower_id: 17}
35 is watered by 2
Sending withering flower 33
Sent: {action: 2, flower_id: 33}
35 is being watered by 1
35 is watered by 1
Sending withering flower 15
Sent: {action: 2, flower_id: 15}
2 is getting flower status of 33
Sending 2 status 1
Sent: {action: 6, flower_id: 33}
15 is being watered by 2
1 is getting flower status of 33
Sending 1 status 1
Sent: {action: 6, flower_id: 33}
15 is watered by 2
Sending withering flower 26
Sent: {action: 2, flower_id: 26}
15 is being watered by 1
15 is watered by 1
Sending withering flower 12
Sent: {action: 2, flower_id: 12}
2 is getting flower status of 26
Sending 2 status 1
Sent: {action: 6, flower_id: 26}
12 is being watered by 2
1 is getting flower status of 26
Sending 1 status 1
Sent: {action: 6, flower_id: 26}
12 is watered by 2
Sending withering flower 9
Sent: {action: 2, flower_id: 9}
12 is being watered by 1
12 is watered by 1
Sending withering flower 21
Sent: {action: 2, flower_id: 21}
2 is getting flower status of 9
Sending 2 status 1
Sent: {action: 6, flower_id: 9}
21 is being watered by 2
1 is getting flower status of 9
Sending 1 status 1
Sent: {action: 6, flower_id: 9}
21 is watered by 2
Sending withering flower 2
Sent: {action: 2, flower_id: 2}
21 is being watered by 1
21 is watered by 1
Sending withering flower 27
Sent: {action: 2, flower_id: 27}
2 is getting flower status of 2
Sending 2 status 1
Sent: {action: 6, flower_id: 2}
27 is being watered by 2
1 is getting flower status of 2
Sending 1 status 1
Sent: {action: 6, flower_id: 2}
27 is watered by 2
Sending withering flower 10
Sent: {action: 2, flower_id: 10}
27 is being watered by 1
27 is watered by 1
Sending withering flower 19
Sent: {action: 2, flower_id: 19}
2 is getting flower status of 10
Sending 2 status 1
Sent: {action: 6, flower_id: 10}
19 is being watered by 2
1 is getting flower status of 10
Sending 1 status 1
Sent: {action: 6, flower_id: 10}
19 is watered by 2
Sending withering flower 3
Sent: {action: 2, flower_id: 3}
19 is being watered by 1
19 is watered by 1
Sending withering flower 6
Sent: {action: 2, flower_id: 6}
2 is getting flower status of 3
Sending 2 status 1
Sent: {action: 6, flower_id: 3}
6 is being watered by 2
1 is getting flower status of 3
Sending 1 status 1
Sent: {action: 6, flower_id: 3}
6 is watered by 2
Sending withering flower 20
Sent: {action: 2, flower_id: 20}
6 is being watered by 1
6 is watered by 1
2 is getting flower status of 20
Sending 2 status 1
Sent: {action: 6, flower_id: 20}
20 is being watered by 2
1 is getting flower status of 20
Sending 1 status 2
Sent: {action: 7, flower_id: 20}
20 is watered by 2
Sending withering flower 12
Sent: {action: 2, flower_id: 12}
2 is getting flower status of 12
Sending 2 status 1
Sent: {action: 6, flower_id: 12}
12 is being watered by 2
1 is getting flower status of 12
Sending 1 status 2
Sent: {action: 7, flower_id: 12}
12 is watered by 2
Sending withering flower 16
Sent: {action: 2, flower_id: 16}
2 is getting flower status of 16
Sending 2 status 1
Sent: {action: 6, flower_id: 16}
16 is being watered by 2
1 is getting flower status of 16
Sending 1 status 2
Sent: {action: 7, flower_id: 16}
16 is watered by 2
Sending withering flower 11
Sent: {action: 2, flower_id: 11}
2 is getting flower status of 11
Sending 2 status 1
Sent: {action: 6, flower_id: 11}
1 is getting flower status of 11
Sending 1 status 1
Sent: {action: 6, flower_id: 11}
11 is being watered by 2
11 is being watered by 1
11 is watered by 2
Sending withering flower 8
Sent: {action: 2, flower_id: 8}
11 is watered by 1
Sending withering flower 7
Sent: {action: 2, flower_id: 7}
2 is getting flower status of 8
Sending 2 status 1
Sent: {action: 6, flower_id: 8}
7 is being watered by 2
1 is getting flower status of 8
Sending 1 status 1
Sent: {action: 6, flower_id: 8}
7 is watered by 2
Sending withering flower 29
Sent: {action: 2, flower_id: 29}
7 is being watered by 1

*/