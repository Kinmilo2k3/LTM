#include "client_fn.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int sockfd;
char buffer[BUFFER_SIZE];
struct sockaddr_in servaddr;

extern int room_idx;
extern int user_idx;
extern char room_code[];

void flush_buffer() { memset(buffer, 0, BUFFER_SIZE); }

void init_connection() {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Fill server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
}

int send_login_request(const char *username, const char *password) {
    flush_buffer();
    sprintf(buffer, "%c%s %s", (char)LOGIN_REQUEST, username, password);

    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

    if (buffer[0] == LOGIN_RESULT && buffer[1] == SUCESS) {
        return 1;
    }

    return -1;
}

int send_create_account_request(const char *username, const char *password) {
    flush_buffer();
    sprintf(buffer, "%c%s %s", (char)SIGNUP_REQUEST, username, password);

    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

    if (buffer[0] == SIGNUP_RESULT && buffer[1] == SUCESS) {
        return 1;
    }

    return -1;
}

int send_join_request() {
    flush_buffer();
    sprintf(buffer, "%c%6s", (char)JOIN_REQUEST, room_code);

    sendto(sockfd, buffer, 1 + RCODE_LEN, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

    if (buffer[0] == JOIN_RESPONSE && buffer[1] == SUCESS) {
        user_idx = (int)buffer[2];
        room_idx = (int)buffer[3];
        return 1;
    }
    return -1;
}

int request_room_code() {
    flush_buffer();
    sprintf(buffer, "%c", (char)CODE_REQUEST);

    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

    // Received <RETURN_CODE><result><room code><room index>
    if (buffer[0] == RETURN_CODE && buffer[1] == SUCESS) {
        strncpy(room_code, buffer + 2, RCODE_LEN);
        room_idx = (int)buffer[3 + RCODE_LEN];
        return 1;
    }

    return -1;
}