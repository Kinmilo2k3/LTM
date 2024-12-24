#include "ui.h"
#include "client_fn.h"

GtkWidget *grid;

extern GtkWidget *window;
extern char room_code[RCODE_LEN];
extern int user_idx;
extern int room_idx;

extern void display_alert(const char *title, const char *content);

void on_join_room_clicked(GtkWidget *widget, gpointer data) {
    const gchar *room_code_data = gtk_entry_get_text(GTK_ENTRY(data));
    memcpy(room_code, room_code_data, RCODE_LEN);

    if (send_join_request() < 0) {
        g_print("[JOIN_ROOM] Room %s does not exist\n", room_code);

        display_alert("Warning", "Room does not exist");
    } else {
        g_print("[JOIN_ROOM] Joined in room %s\n", room_code);
        gtk_main_quit();
        gtk_widget_destroy(grid);

        // TODO: write into `room_idx`
    }
}

void on_create_room_clicked(GtkWidget *widget, gpointer data) {
    if (request_room_code() < 0) {
        g_print("[ROOM_CREATION] Failed to create room\n");

        display_alert("Warning", "Failed to create room");
    } else {
        g_print("[ROOM_CREATION] Room created with code %s\n", room_code);
        
        // TODO: write into `room_idx`

        gtk_main_quit();
        gtk_widget_destroy(grid);
    }
}

void display_home_screen() {
    GtkWidget *room_code_label, *room_code_entry;
    GtkWidget *join_room_button, *create_room_button;

    // Create a grid layout
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create label and entry for room code
    room_code_label = gtk_label_new("Room Code:");
    room_code_entry = gtk_entry_new();

    // Create buttons
    join_room_button = gtk_button_new_with_label("Join Room");
    create_room_button = gtk_button_new_with_label("Create Room");

    // Connect button signals
    g_signal_connect(join_room_button, "clicked", G_CALLBACK(on_join_room_clicked), room_code_entry);
    g_signal_connect(create_room_button, "clicked", G_CALLBACK(on_create_room_clicked), NULL);

    // Add widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), room_code_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), room_code_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), join_room_button, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_room_button, 1, 1, 1, 1);

    gtk_widget_show_all(window);
    gtk_main();
}
