#include <gtk/gtk.h>
#include <langinfo.h>
#include <libintl.h>
#include <locale.h>
#include <unique/unique.h>

#include "main_window.h"

static GtkWidget *window = NULL;

static UniqueResponse
message_received_cb (UniqueApp         *app,
        UniqueCommand      command,
        UniqueMessageData *message,
        guint              time_,
        gpointer           user_data)
{
    UniqueResponse res;
    switch (command)
    {
        case UNIQUE_ACTIVATE:
            gtk_window_set_screen (GTK_WINDOW (window), unique_message_data_get_screen (message));
            gtk_window_present_with_time (GTK_WINDOW (window), time_);
            res = UNIQUE_RESPONSE_OK;
            break;
        default:
            res = UNIQUE_RESPONSE_OK;
            break;
    }
    return res;
}

int
main(int argc, char **argv)
{
    UniqueApp *app;

    gtk_init(&argc, &argv);

    app = unique_app_new_with_commands ("org.fcitx.fcitx-configtool", NULL,
            NULL);

    if (unique_app_is_running(app))
    {
        UniqueResponse response = unique_app_send_message (app, UNIQUE_ACTIVATE, NULL);
        g_object_unref (app);
        if (response == UNIQUE_RESPONSE_OK)
            return 0;
        else
            return 1;
    }

    setlocale(LC_ALL, "");
    bindtextdomain("fcitx-configtool", LOCALEDIR);
    bindtextdomain("fcitx", LOCALEDIR);
    textdomain("fcitx-configtool");

    window = fcitx_config_main_window_new (GTK_WINDOW_TOPLEVEL);
    unique_app_watch_window (app, GTK_WINDOW (window));
    g_signal_connect (app, "message-received", G_CALLBACK (message_received_cb), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    g_object_unref (app);

    return 0;
}
 
