/***************************************************************************
 *   Copyright (C) 2010~2012 by CSSlayer                                   *
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

static void
fcitx_config_app_activate (GApplication *application)
{
    GList* list = gtk_application_get_windows (GTK_APPLICATION(application));
    if (list)
    {
        gtk_window_present (GTK_WINDOW (list->data));
    }
    else {
        GtkWidget *window;
        window = fcitx_main_window_new();
        gtk_application_add_window(GTK_APPLICATION(application), GTK_WINDOW(window));
        gtk_widget_show_all (GTK_WIDGET (window));
    }
}

typedef GtkApplication FcitxConfigApp;
typedef GtkApplicationClass FcitxConfigAppClass;

G_DEFINE_TYPE (FcitxConfigApp, fcitx_config_app, GTK_TYPE_APPLICATION)

static void
fcitx_config_app_finalize (GObject *object)
{
    G_OBJECT_CLASS (fcitx_config_app_parent_class)->finalize (object);
}

static void
fcitx_config_app_init (FcitxConfigApp *app)
{
}

static void
fcitx_config_app_class_init (FcitxConfigAppClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize= fcitx_config_app_finalize;

    G_APPLICATION_CLASS (klass)->activate = fcitx_config_app_activate;
}

FcitxConfigApp *
fcitx_config_app_new (void)
{
    g_type_init ();

    return g_object_new (fcitx_config_app_get_type (),
                         "application-id", "org.fcitx.FcitxConfigGtk3",
                         "flags", G_APPLICATION_FLAGS_NONE,
                         NULL);
}

int
main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain("fcitx-configtool", LOCALEDIR);
    bind_textdomain_codeset("fcitx-configtool", "UTF-8");
    bindtextdomain("fcitx", LOCALEDIR);
    bind_textdomain_codeset("fcitx", "UTF-8");
    textdomain("fcitx-configtool");

    GtkApplication* app = fcitx_config_app_new();

    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}


