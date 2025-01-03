#include "client_fn.h"
#include "ui.h"

typedef struct {
    GtkWidget *username_entry;
    GtkWidget *password_entry;
} user_data_t;

char user_logged_in[128];

GtkWidget *window = NULL;
GtkWidget *grid;

void display_alert(const char *title, const char *content) {
    GtkWidget *dialog = gtk_message_dialog_new(
        (GtkWindow *)window, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK, "%s", content);

    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void on_exit_clicked(GtkWidget *widget, gpointer data) {
    send_quit();
    g_print("[EXIT] Exiting the application...\n");
    gtk_main_quit();
}

void on_sign_in_clicked(GtkWidget *widget, gpointer data) {
    user_data_t *user_data = (user_data_t *)data;

    const gchar *username = gtk_entry_get_text(GTK_ENTRY(user_data->username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(user_data->password_entry));
    
    if (send_login_request(username, password) < 0) {
        g_print("[SIGN_IN] Sign in failed\n");

        display_alert("Warning", "Sign in failed");
    } else {
        g_print("[SIGN_IN] Signed in as %s\n", username);
        strcpy(user_logged_in, username);
        gtk_widget_destroy(grid);
        gtk_main_quit();
    }
}

void on_sign_up_clicked(GtkWidget *widget, gpointer data) {
    user_data_t *user_data = (user_data_t *)data;

    const gchar *username = gtk_entry_get_text(GTK_ENTRY(user_data->username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(user_data->password_entry));

    if (send_create_account_request(username, password) < 0) {
        g_print("[SIGN_UP] Sign up failed\n");

        display_alert("Warning", "Sign up failed!");
    } else {
        g_print("[SIGN_UP] Signed up %s\n", username);

        display_alert("Notification", "Signed up");
    }
}

void display_login_screen() {
    GtkWidget *username_label, *password_label;
    GtkWidget *username_entry, *password_entry;
    GtkWidget *sign_in_button, *sign_up_button;

    if (!window) {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "ChatApp");
        gtk_container_set_border_width(GTK_CONTAINER(window), 10);
        gtk_window_set_resizable((GtkWindow *)window, TRUE);
        g_signal_connect(window, "destroy", G_CALLBACK(on_exit_clicked), NULL);
    }

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create labels
    username_label = gtk_label_new("Username:");
    password_label = gtk_label_new("Password:");

    // Create text entry fields
    username_entry = gtk_entry_new();
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);  // Hide password input
    gtk_entry_set_invisible_char(GTK_ENTRY(password_entry), '*');

    // Create buttons
    sign_in_button = gtk_button_new_with_label("Sign In");
    sign_up_button = gtk_button_new_with_label("Sign Up");

    user_data_t user_data = {username_entry, password_entry};

    // Connect button signals
    g_signal_connect(sign_in_button, "clicked", G_CALLBACK(on_sign_in_clicked), &user_data);
    g_signal_connect(sign_up_button, "clicked", G_CALLBACK(on_sign_up_clicked), &user_data);

    // Add widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sign_up_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sign_in_button, 1, 2, 1, 1);

    gtk_widget_show_all(window);
    gtk_main();
}
