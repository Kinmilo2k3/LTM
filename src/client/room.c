#include <string.h>

#include "client_fn.h"
#include "ui.h"

extern GtkWidget *window;
extern char room_code[];

GtkWidget *grid;
bool start_game = false;

extern void display_alert(const char *, const char *);

static void send_message(GtkWidget *button, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(data);
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (g_strcmp0(message, "") != 0) {
        GtkTextBuffer *buffer;
        GtkTextIter end_iter;

        // Get the text buffer from the text view
        GtkWidget *text_view = g_object_get_data(G_OBJECT(entry), "text_view");
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

        // Append the new message to the text buffer
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        gtk_text_buffer_insert(buffer, &end_iter, message, -1);
        gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

        // Clear the entry field
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}

void display_lobby_screen() {
    GtkWidget *room_code_label;

    // Create and add the room code label
    char label[128] = "Room code: ";
    strcat(label, room_code);
    room_code_label = gtk_label_new(label);
    gtk_grid_attach(GTK_GRID(grid), room_code_label, 0, 0, 4, 1);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the main loop
    gtk_main();
}

void display_chatbox() {
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkWidget *entry;
    GtkWidget *send_button;
    GtkWidget *hbox;

    // Create vertical box layout
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create and add room code label
    char label_text[128] = "Room code: ";
    strcat(label_text, room_code);
    label = gtk_label_new(label_text);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

    // Create scrolled window for chat messages
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 5);

    // Create text view for chat messages
    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    // Create horizontal box for entry and button
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    // Create entry for message input
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 5);

    // Store reference to text view in entry widget
    g_object_set_data(G_OBJECT(entry), "text_view", text_view);

    // Create send button
    send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(hbox), send_button, FALSE, FALSE, 5);

    // Connect send button signal
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), entry);

    // Show all widgets
    gtk_widget_show_all(window);

    // Run the GTK main loop
    gtk_main();
}
