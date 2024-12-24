#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdbool.h>

#include "msg.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void init_connection();

int send_login_request(const char *username, const char *password);

int send_create_account_request(const char *username, const char *password);

int send_join_request();

int request_room_code();

void *listen_room_updates_thread();

#endif