#include "client_fn.h"
#include "ui.h"

char room_code[RCODE_LEN];
int user_idx;
int room_idx;

int main(int argc, char *argv[]) {
    init_connection();

    gtk_init(&argc, &argv);

    display_login_screen();
    display_home_screen();
    display_chatbox();

    return 0;
}