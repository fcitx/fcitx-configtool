/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <gtk/gtk.h>
#include <langinfo.h>
#include <libintl.h>
#include <locale.h>
#include "config.h"
#include "main_window.h"

static GtkWidget *window = NULL;

#ifdef HAVE_UNIQUE
#include <unique/unique.h>

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
#endif

int
main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

#ifdef HAVE_UNIQUE
    UniqueApp *app;

    app = unique_app_new_with_commands ("org.fcitx.fcitx-configtool", NULL,
            NULL, NULL);

    if (unique_app_is_running(app))
    {
        UniqueResponse response = unique_app_send_message (app, UNIQUE_ACTIVATE, NULL);
        g_object_unref (app);
        if (response == UNIQUE_RESPONSE_OK)
            return 0;
        else
            return 1;
    }
#endif

    setlocale(LC_ALL, "");
    bindtextdomain("fcitx-configtool", LOCALEDIR);
    bind_textdomain_codeset("fcitx-configtool", "UTF-8");
    bindtextdomain("fcitx", LOCALEDIR);
    bind_textdomain_codeset("fcitx", "UTF-8");
    textdomain("fcitx-configtool");

    window = fcitx_main_window_new ();

#ifdef HAVE_UNIQUE
    unique_app_watch_window (app, GTK_WINDOW (window));
    g_signal_connect (app, "message-received", G_CALLBACK (message_received_cb), NULL);
#endif

    gtk_widget_show_all(window);

    gtk_main();

#ifdef HAVE_UNIQUE
    g_object_unref (app);
#endif

    return 0;
}

