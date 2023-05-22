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

#include "../essentials.h"

#define MAX_CLIENTS 10

int pid;
int parent;
int continueocity = 1;
int gard1_sock = 0;
int gard2_sock = 0;
int flow_sock = 0;

int shmid;
server_memory *shared;

void end(int dummy) {
    continueocity = 0;

    if (dummy == SIGUSR1) {
        if (getpid() != parent) {
            exit(0);
        }
        if (close(shmid) == -1) {
            endError("Failed to close shmem");
        }
        if (shm_unlink("serv-memory") == -1) {
            endError("Failed to unlink memory");
        }
    } else {
        kill(0, SIGUSR1);
    }

    sleep(5);

    exit(0);
}

int main(int argc, char *argv[]) {
    parent = getpid();
    int server_socket;
    char buffer[1024] = {0};

    int sockets_of_gardeners[2] = {0, 0};
    int flowerbed_socket = 0;
    signal(SIGINT, end);
    signal(SIGUSR1, end);
    if (argc < 2) {
        printf("Usage: %s [server_port]", argv[0]);
        exit(1);
    }
    const int PORT = atoi(argv[1]);
    server_socket = TCPCreateSocket(PORT);

    printf("Server started listening on port %d\n", PORT);

    if ((shmid = shm_open("serv-memory", O_CREAT | O_RDWR, 0666)) == -1) {
        endError("failed to get mem");
    }

    if (ftruncate(shmid, sizeof(server_memory)) == -1) {
        endError("failed to resize mem");
    }

    if ((shared = mmap(0, sizeof(server_memory), PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0)) ==
        MAP_FAILED) {
        endError("failed to map mem");
    }

    int child_count = 0;
    int mode = 0;

    struct sockaddr_in client_addr;
    unsigned int client_len = sizeof(client_addr);

    while (1) {
        printf("Waiting for clients to connect....\n");
        char handshake_message[2];
        int handshake_msg_len;
        int temp_sock;
        if ((temp_sock = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            endError("failed accept client");
        }
        if ((handshake_msg_len = recv(temp_sock, handshake_message, 2, 0)) < 0) {
            endError("failed handshake");
        }
        if (handshake_message[0] == 'G' && handshake_message[1] == '1') {
            mode = 1;
            gard1_sock = temp_sock;
            printf("Gardener 1 connected!\n");
            send(gard1_sock, &mode, sizeof(int), 0);
        } else if (handshake_message[0] == 'G' && handshake_message[1] == '2') {
            mode = 2;
            gard2_sock = temp_sock;
            printf("Gardener 2 connected!\n");
            send(gard2_sock, &mode, sizeof(int), 0);
        } else if (handshake_message[0] == 'F') {
            mode = 3;
            flow_sock = temp_sock;
            printf("Flowerbed connected!\n");
            send(flow_sock, &mode, sizeof(int), 0);
        } else {
            close(temp_sock);
        }
        if (flow_sock && gard1_sock && gard2_sock) {
            break;
        }
    }

    pid = fork();
    if (pid == 0) {
        int pide = fork();
        if (pide < 0) {
        } else if (pide == 0) {
            while (1) {
                message *msg = (message *)malloc(sizeof(message));
                memset(msg, 0, sizeof(message));
                int len = recv(flow_sock, msg, sizeof(message), 0);
                if (len < 0) {
                    endError("ds");
                }
                if (len == 0) {
                    break;
                }
                if (msg->action != 0) {
                    // printf("%ln", (long int *)msg);
                    printf("[Flowers] {action: %d, flower_id:%d, dest: %d}\n", msg->action,
                           msg->flower_id, msg->dest);
                    if (msg->dest == 1) {
                        send(gard1_sock, msg, sizeof(message), 0);
                    } else if (msg->dest == 2) {
                        send(gard2_sock, msg, sizeof(message), 0);
                    } else if (msg->action == 2) {
                        msg->dest = 0;
                        int pud = fork();
                        if (pud == 0) {
                            send(gard1_sock, msg, sizeof(message), 0);
                            exit(0);
                        } else {
                            send(gard2_sock, msg, sizeof(message), 0);
                        }

                        printf("Sent to both gardeners\n");
                    }
                }
            }
        } else {
            while (1) {
                message *msg = (message *)malloc(sizeof(message));
                int len = recv(gard1_sock, msg, sizeof(message), 0);
                if (len < 0) {
                    endError("ds");
                }
                if (len == 0) {
                    break;
                }
                msg->dest = 1;
                printf("[Gard1] {action: %d, flower_id:%d}\n", msg->action, msg->flower_id);
                send(flow_sock, msg, sizeof(message), 0);
            }
        }
    } else {
        int pide = fork();
        if (pide == 0) {
            while (1) {
                message *msg = (message *)malloc(sizeof(message));
                int len = recv(gard2_sock, msg, sizeof(message), 0);
                if (len < 0) {
                    endError("ds");
                }
                if (len == 0) {
                    break;
                }
                msg->dest = 2;
                printf("[Gard2] {action: %d, flower_id:%d}\n", msg->action, msg->flower_id);
                send(flow_sock, msg, sizeof(message), 0);
            }

        } else {
            char msg[1] = {'+'};
            send(gard1_sock, msg, 1, 0);
            send(gard2_sock, msg, 1, 0);
            send(flow_sock, msg, 1, 0);
            while (1) {
                pid = wait(NULL);
                if (pid == -1) {
                    printf("All child processes terminated.\n");
                    break;
                }
            }
        }
    }

    close(gard1_sock);
    close(gard2_sock);
    close(flow_sock);
    close(server_socket);

    return 0;
}
