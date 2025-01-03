#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdbool.h>
#include <arpa/inet.h>

#include "msg.h"

#define PORT 8080
#define BUFFER_SIZE 4096

void init_connection();
int send_login_request(const char *username, const char *password);
int send_create_account_request(const char *username, const char *password);
int send_join_request();
int request_room_code();
int send_message(const char *msg);

#endif