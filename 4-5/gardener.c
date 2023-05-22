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
#include <time.h>
#include <unistd.h>

#include "../essentials.h"
#define PORT 12345

int flg = 1;
int sock = 0;
void ctrl_c(int dummy) {
    flg = 0;
    close(sock);
}

int main(int argc, char *argv[]) {
    // signal(SIGINT, ctrl_c);

    struct sockaddr_in serv_addr;

    sem_t *sem = sem_open("garden-se10m", O_CREAT, 0666, 1);

    if (sem == SEM_FAILED) {
        endError("FAILED TO ATTACH SEMAPHORE");
    }

    if (argc < 2) {
        printf("Usage: %s [identifier]", argv[0]);
        return 1;
    }

    char identifier = argv[1][0];

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

    char handshake_message[2] = {'G', identifier};

    printf("Asking for handshake...\n");

    send(sock, handshake_message, 2, 0);

    int mode = 0;

    if (recv(sock, &mode, sizeof(mode), 0) < 0) {
        endError("Handshake error");
    }

    printf("Handshake happened\n");
    if (mode != GARDENER1_MODE && mode != GARDENER2_MODE) {
        endError("WRONG MODE");
    }
    printf("Waiting for begining\n");
    char a[1] = {'-'};
    if (recv(sock, a, 1, 0) < 0) {
        endError("failed to recv");
    }

    if (a[0] != '+') {
        printf("No start\n");
        flg = 0;
    }

    int *flowers = calloc(sizeof(int), 40);

    message *msg = (message *)malloc(sizeof(message));
    message *inner_msg = (message *)malloc(sizeof(message));
    int mlen;
    while (flg) {
        if ((mlen = recv(sock, msg, sizeof(message), 0)) < 0) {
            close(sock);
            endError("Cant reach server");
        }
        if (mlen == 0) {
            break;
        }
        printf("Recieved message: {action: %d, flower_id: %d}\n", msg->action, msg->flower_id);
        inner_msg->flower_id = msg->flower_id;
        if (msg->action == WITHER_FLOWER) {
            if (sem_wait(sem) < 0) {
                endError("Sem wait failed");
            }
            inner_msg->action = GET_FLOWER_STATUS;
            //* request info
            printf("[Send] {action: %d, flower_id: %d}\n", inner_msg->action, inner_msg->flower_id);
            send(sock, inner_msg, sizeof(message), 0);
            recv(sock, msg, sizeof(message), 0);
            printf("[Rec] {action: %d, flower_id: %d}\n", msg->action, msg->flower_id);
            //* =====
            if (msg->action == RET_WATERING) {
                if (sem_post(sem) < 0) {
                    endError("Sem psot failed");
                }
                continue;
            }
            printf("%d", time(NULL));
            inner_msg->action = WATER_FLOWER;
            inner_msg->flower_id = msg->flower_id;
            printf("[Send] {action: %d, flower_id: %d}\n", inner_msg->action, inner_msg->flower_id);
            send(sock, inner_msg, sizeof(message), 0);
            usleep(200 * 1000);
            if (sem_post(sem) < 0) {
                endError("Sem psot failed");
            }
            usleep(200 * 1000);
            inner_msg->action = WATERED_FLOWER;
            printf("[Send] {action: %d, flower_id: %d}\n", inner_msg->action, inner_msg->flower_id);
            send(sock, inner_msg, sizeof(message), 0);
        }
    }
    free(msg);
    free(inner_msg);
    close(sock);
    sem_close(sem);

    sem_unlink("garden-sem");

    return 0;
}
