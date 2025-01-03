#ifndef __MSG_H
#define __MSG_H

#define RCODE_LEN 6

typedef enum {
    // client-side flags
    LOGIN_REQUEST = 0x01,
    SIGNUP_REQUEST,
    CODE_REQUEST,
    JOIN_REQUEST,

    // server-side flags
    LOGIN_RESULT,
    SIGNUP_RESULT,
    RETURN_CODE,
    JOIN_RESPONSE,

    // common
    MESSAGE,
    JOIN_NOTI
} flag_t;

typedef enum {
    SUCESS = 0x01,
    FAILED,
} result_t;

#endif