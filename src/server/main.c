#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msg.h"

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 4
#define MAX_ROOMS 10
#define INFO_MAXLEN 128
#define DB_PATH "database/users.txt"

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char name[INFO_MAXLEN];
    struct sockaddr_in addr;
} client_t;

typedef struct {
    char room_code[RCODE_LEN];
    client_t clients[MAX_CLIENTS];
    int num_clients;
    int num_clients_sent;
} room_t;

char buffer[BUFFER_SIZE];
int sockfd;
struct sockaddr_in server_addr, client_addr;
socklen_t addr_len = sizeof(client_addr);
room_t rooms[MAX_ROOMS];

int randint(int lower, int upper) {
    int num = rand();

    num = (num % (upper - lower + 1)) + lower;
    return num;
}

void flush_buffer() { memset(buffer, 0, BUFFER_SIZE); }

void broadcast_room(room_t room, char *buffer) {
    struct sockaddr *addr;

    for (int i = 0; i < room.num_clients; i++) {
        addr = (struct sockaddr *)&room.clients[i].addr;
        sendto(sockfd, buffer, BUFFER_SIZE, 0, addr, addr_len);
    }
}

void broad_cast_room_except(room_t room, int except_client_idx, char *buffer) {
    struct sockaddr *addr;

    for (int i = 0; i < room.num_clients; i++) {
        if (i == except_client_idx) continue;

        addr = (struct sockaddr *)&room.clients[i].addr;
        sendto(sockfd, buffer, BUFFER_SIZE, 0, addr, addr_len);

        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, addr, ipstr, INET_ADDRSTRLEN);
    }
}

bool is_valid_user(const char *username, const char *password) {
    char file_user[INFO_MAXLEN], file_pass[INFO_MAXLEN];
    char line[BUFFER_SIZE];
    FILE *file = fopen(DB_PATH, "r");

    if (!file) {
        perror("Failed to open users file");
        return false;
    }

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s", file_user, file_pass);
        if (strcmp(username, file_user) == 0 && strcmp(password, file_pass) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

void handle_login_request(char *buffer) {
    // Received buffer: <LOGIN_REQUEST><username><space><password>
    char username[INFO_MAXLEN], password[INFO_MAXLEN];
    char result;

    sscanf(buffer + 1, "%s %s", username, password);
    printf("[LOGIN_REQUEST] usr: %s; pwd: %s\n", username, password);
    if (is_valid_user(username, password)) {
        result = SUCESS;
    } else {
        result = FAILED;
    }

    // Buffer to send: <LOGIN_RESULT><result>
    flush_buffer();
    sprintf(buffer, "%c%c", LOGIN_RESULT, (char)result);
    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client_addr, addr_len);
}

bool user_existed(const char *username) {
    char file_user[INFO_MAXLEN], file_pass[INFO_MAXLEN];
    FILE *file = fopen(DB_PATH, "r");
    if (!file) {
        perror("Failed to open users file");
        return false;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s", file_user, file_pass);
        if (strcmp(username, file_user) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

void signup(const char *username, const char *password) {
    FILE *file = fopen(DB_PATH, "a");
    if (file == NULL) {
        perror("Failed to open user file");
    } else {
        int score = 0;
        fprintf(file, "%s %s %d\n", username, password, score);
        fclose(file);
    }
}

void handle_signup_request(char *buffer) {
    // Received buffer: <SIGNUP_REQUEST><username><space><password>
    char username[INFO_MAXLEN], password[INFO_MAXLEN];
    char payload;

    sscanf(buffer + 1, "%s %s", username, password);
    printf("[SIGNUP_REQUEST] usr: %s; pwd: %s\n", username, password);
    if (user_existed(username)) {
        payload = FAILED;
    } else {
        signup(username, password);
        payload = SUCESS;
    }

    // Buffer to send: <SIGNUP_RESULT><RESULT>
    flush_buffer();
    sprintf(buffer, "%c%c", SIGNUP_RESULT, (char)payload);
    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client_addr, addr_len);
}

void generate_room_code(char *room_code) {
    extern int randint(int, int);

    bool end = false;
    while (!end) {
        for (int i = 0; i < RCODE_LEN; i++) {
            room_code[i] = (char)randint(48, 57);
        }

        // Check for duplicated room codes
        for (int i = 0; i < MAX_ROOMS; i++) {
            if (strcmp(room_code, rooms[i].room_code) == 0) {
                end = true;
                break;
            } else {
                end = false;
            }
        }
    }
}

void init_rooms() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        generate_room_code(rooms[i].room_code);
        rooms[i].num_clients = 0;
        rooms[i].num_clients_sent = 0;
    }
}

void handle_code_request(char *buffer) {
    // Received buffer: <CODE_REQUEST>
    char result;
    char room_code[RCODE_LEN];
    int i;

    for (i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].num_clients == 0) {
            strncpy(room_code, rooms[i].room_code, RCODE_LEN);
            break;
        }
    }

    if (room_code[0] == '\0') {
        result = (char)FAILED;
    } else {
        rooms[i].clients[0].addr = client_addr;
        rooms[i].num_clients = 1;
        result = (char)SUCESS;
    }

    // Buffer to send: <RETURN_CODE><result><room code><room index>
    flush_buffer();
    sprintf(buffer, "%c%c%s%c", (char)RETURN_CODE, result, room_code, i);
    sendto(sockfd, buffer, 3 + RCODE_LEN, 0, (const struct sockaddr *)&client_addr, addr_len);
}

void handle_join_request(char *buffer) {
    // Received buffer: <JOIN_REQUEST><room code>

    char room_code[RCODE_LEN];
    char result = FAILED;
    int num_clients;
    char username[128];

    sscanf(buffer + 1, "%s %s", room_code, username);

    int i;
    for (i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(rooms[i].room_code, room_code) == 0 && rooms[i].num_clients < MAX_CLIENTS &&
            rooms[i].num_clients >= 1) {
            num_clients = rooms[i].num_clients;

            rooms[i].clients[num_clients].addr = client_addr;

            rooms[i].num_clients++;
            result = (char)SUCESS;
            break;
        }
    }

    // Buffer to send: <JOIN_RESPONSE><result><player index><room index>
    flush_buffer();
    sprintf(buffer, "%c%c%c%c", (char)JOIN_RESPONSE, result, (char)num_clients, (char)i);
    sendto(sockfd, buffer, 4, 0, (const struct sockaddr *)&client_addr, addr_len);
    
    // Buffer to send: <ROOM_NUM_USERS_UPDATE>
    flush_buffer();
    sprintf(buffer, "%c%s", JOIN_NOTI, username);
    broad_cast_room_except(rooms[i], num_clients, buffer);
}

void handle_message(char *buffer) {
    // <MESSAGE><user idx><room idx><username|message>
    int user_idx, room_idx;
    char username[128];
    char message[4000];

    user_idx = (int)buffer[1];
    room_idx = (int)buffer[2];

    sscanf(buffer + 3, "%s", username);
    strncpy(message, buffer + 131, strlen(buffer + 131));

    flush_buffer();
    buffer[0] = MESSAGE;
    sprintf(buffer + 1, "%s", username);
    sprintf(buffer + 129, "%s", message);

    broad_cast_room_except(rooms[room_idx], user_idx, buffer);
}

void handle_quit(char *buffer) {
    int user_idx = buffer[1];
    int room_idx = buffer[2];
    char username[128];

    sscanf(buffer + 3, "%s", username);

    flush_buffer();
    sprintf(buffer, "%c%s", QUIT, username);

    broad_cast_room_except(rooms[room_idx], user_idx, buffer);
}

void handle_client_request(void *arg) {
    pthread_mutex_lock(&buffer_mutex);
    char local_buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr_local;
    socklen_t addr_len_local = addr_len;

    memcpy(local_buffer, buffer, BUFFER_SIZE);
    memcpy(&client_addr_local, &client_addr, addr_len);
    pthread_mutex_unlock(&buffer_mutex);

    switch (local_buffer[0]) {
        case LOGIN_REQUEST:
            handle_login_request(local_buffer);
            break;

        case SIGNUP_REQUEST:
            handle_signup_request(local_buffer);
            break;

        case CODE_REQUEST:
            handle_code_request(local_buffer);
            break;

        case JOIN_REQUEST:
            handle_join_request(local_buffer);
            break;

        case MESSAGE:
            handle_message(local_buffer);
            break;

        case QUIT:
            handle_quit(local_buffer);
            break;

        default:
            printf("[SIGNAL] Undefined signal (0x%02x) received\n", buffer[0]);
            break;
    }
    pthread_exit(NULL);
}

int main() {
    init_rooms();

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] UDP server listening on port %d\n", PORT);

    while (true) {
        pthread_t thread_id;

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("Error receiving data");
            continue;
        }
        buffer[n] = '\0';

        if (pthread_create(&thread_id, NULL, (void *)handle_client_request, NULL) != 0) {
            perror("Failed to create thread");
        }

        pthread_detach(thread_id);
    }

    close(sockfd);
    pthread_mutex_destroy(&buffer_mutex);
    return 0;
}
