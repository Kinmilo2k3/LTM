#include <string.h>

#include "client_fn.h"
#include "ui.h"

extern GtkWidget *window;
extern char room_code[];

GtkWidget *grid;
GtkWidget *text_view;

int running = 1;

extern void display_alert(const char *, const char *);

static void on_send_message(GtkWidget *button, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(data);
    const gchar *_message = gtk_entry_get_text(GTK_ENTRY(entry));
    char msg[4000];
    char msg_with_sender[4000];

    if (g_strcmp0(_message, "") != 0) {
        sprintf(msg_with_sender, "You: %s", _message);
        sprintf(msg, "%s", _message);

        GtkTextBuffer *buffer;
        GtkTextIter end_iter;

        // Get the text buffer from the text view
        GtkWidget *text_view = g_object_get_data(G_OBJECT(entry), "text_view");
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

        // Append the new message to the text buffer
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        gtk_text_buffer_insert(buffer, &end_iter, msg_with_sender, -1);
        gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

        // Clear the entry field
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }

    if (send_message(msg) < 0) {
        g_error("Cannot send message");
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

void display_incoming_message(const char *sender, const char *msg) {
    GtkTextBuffer *buffer;
    GtkTextIter end_iter;

    // Get the text buffer from the text view
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Get the end of the buffer to insert new message
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    // Format and insert the message with the username
    gchar *formatted_message = g_strdup_printf("%s: %s\n", sender, msg);
    gtk_text_buffer_insert(buffer, &end_iter, formatted_message, -1);

    // Clean up
    g_free(formatted_message);
}

void listen_for_messages() {
    extern int sockfd;
    extern char buffer[];

    char incomming_sender[128];
    char incomming_msg[4000];

    while (running) {
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (ready < 0) {
            perror("select error");
            break;
        } else if (ready == 0) {
            continue;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

            if (received < 0) {
                perror("recvfrom failed");
                break;
            }

            switch (buffer[0]) {
                case MESSAGE:
                    sscanf(buffer + 1, "%s", incomming_sender);
                    strncpy(incomming_msg, buffer + 129, strlen(buffer + 129));
                    display_incoming_message(incomming_sender, incomming_msg);
                    break;
                
                case JOIN_NOTI:
                {
                    char temp[1024];

                    sscanf(buffer + 1, "%s", incomming_sender);
                    sprintf(temp, "*** %s has joined in the chat ***", incomming_sender);
                    display_incoming_message("", temp);
                }
                    

                default:
                    break;
            }
        }
    }
}

void display_chatbox() {
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
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

    // Adjust the line height
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Create a tag for adjusting line height
    GtkTextTag *line_height_tag = gtk_text_buffer_create_tag(
        buffer, "line_height", "pixels-above-lines", 20,  // Add 5 pixels above each line
        "pixels-below-lines", 20,                         // Add 5 pixels below each line
        "pixels-inside-wrap", 3,                          // Add 3 pixels inside wrapped lines
        NULL
    );

    // Apply the tag to the entire buffer
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_apply_tag(buffer, line_height_tag, &start, &end);
    //

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
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message), entry);

    // Show all widgets
    gtk_widget_show_all(window);

    pthread_t thread;
    pthread_create(&thread, NULL, (void *)listen_for_messages, NULL);

    // Run the GTK main loop
    gtk_main();
}
