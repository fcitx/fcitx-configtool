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

#include <fcitx-config/xdg.h>
#include <stdlib.h>

#include "sub_config_widget.h"
#include "main_window.h"
#include "configdesc.h"
#include "config_widget.h"

G_DEFINE_TYPE(FcitxSubConfigWidget, fcitx_sub_config_widget, GTK_TYPE_BOX)

static void open_subconfig_file(GtkButton *button, gpointer user_data);
static void open_native_file(GtkButton *button, gpointer user_data);
static void run_program(GtkButton *button, gpointer user_data);
static void run_plugin(GtkButton* button, gpointer user_data);
static void push_into_store_cb(gpointer data, gpointer value, gpointer user_data);

static void
fcitx_sub_config_widget_get_property(GObject *object, guint property_id,
                                     GValue *value, GParamSpec *pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
fcitx_sub_config_widget_set_property(GObject *object, guint property_id,
                                     const GValue *value, GParamSpec *pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
fcitx_sub_config_widget_finalize(GObject *object)
{
    FcitxSubConfigWidget* widget = FCITX_SUB_CONFIG_WIDGET(object);
    sub_config_free(widget->subconfig);
    G_OBJECT_CLASS(fcitx_sub_config_widget_parent_class)->finalize(object);
}

static void
fcitx_sub_config_widget_class_init(FcitxSubConfigWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);


    object_class->get_property = fcitx_sub_config_widget_get_property;
    object_class->set_property = fcitx_sub_config_widget_set_property;
    object_class->finalize = fcitx_sub_config_widget_finalize;
}

static void
fcitx_sub_config_widget_init(FcitxSubConfigWidget *self)
{
}

FcitxSubConfigWidget*
fcitx_sub_config_widget_new(FcitxSubConfig* subconfig)
{
    FcitxSubConfigWidget* widget = g_object_new(FCITX_TYPE_SUB_CONFIG_WIDGET, NULL);

    widget->subconfig = subconfig;
    switch (subconfig->type) {
    case SC_ConfigFile: {
        gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
        g_object_set(G_OBJECT(widget), "expand", TRUE, NULL);
        GtkWidget* view = gtk_tree_view_new();

        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

        GtkWidget* swin = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(swin), view);
        gtk_box_pack_start(GTK_BOX(widget), swin, TRUE, TRUE, 0);
        g_object_set(G_OBJECT(swin), "margin-left", 5, "margin-right", 5, "shadow-type", GTK_SHADOW_IN, NULL);

        GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

        gtk_tree_view_set_model(GTK_TREE_VIEW(view),
                                GTK_TREE_MODEL(store));

        g_hash_table_foreach(widget->subconfig->filelist, push_into_store_cb, store);

        GtkWidget* button = gtk_button_new();
        gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("preferences-system"), GTK_ICON_SIZE_BUTTON));
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_subconfig_file), widget);
        GtkWidget* hbuttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        g_object_set(G_OBJECT(hbuttonbox), "margin", 5, NULL);
        gtk_box_pack_start(GTK_BOX(hbuttonbox), button, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(widget), hbuttonbox, FALSE, TRUE, 0);
        widget->view = view;
    }
    break;
    case SC_NativeFile: {
        GtkWidget* button = gtk_button_new();
        gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("document-open"), GTK_ICON_SIZE_BUTTON));
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_native_file), widget);
        gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 0);
    }
    break;
    case SC_Program: {
        GtkWidget* button = gtk_button_new();
        gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("system-run"), GTK_ICON_SIZE_BUTTON));
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(run_program), widget);
        gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 0);
    }
    break;
    case SC_Plugin: {
        GtkWidget* button = gtk_button_new();
        gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("system-run"), GTK_ICON_SIZE_BUTTON));
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(run_plugin), widget);
        gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 0);
    }
    break;
    default:
        break;
    }

    return widget;
}

void open_subconfig_file(GtkButton *button,
                         gpointer   user_data)
{
    FcitxSubConfigWidget* widget = (FcitxSubConfigWidget*) user_data;
    GtkTreeView* view = GTK_TREE_VIEW(widget->view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeIter iter;
    gchar* configfile;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           0, &configfile,
                           -1);
        FcitxConfigFileDesc* cfdesc = get_config_desc(widget->subconfig->configdesc);
        if (cfdesc) {
            GtkWidget* dialog = gtk_dialog_new_with_buttons(configfile,
                                GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)),
                                GTK_DIALOG_MODAL,
                                _("_Cancel"),
                                GTK_RESPONSE_CANCEL,
                                _("_OK"),
                                GTK_RESPONSE_OK,
                                NULL
                                                           );
            FcitxConfigWidget* config_widget = fcitx_config_widget_new(cfdesc, "", configfile, NULL);
            GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(config_widget), TRUE, TRUE, 0);
            gtk_widget_set_size_request(GTK_WIDGET(config_widget), -1, 400);

            g_signal_connect(dialog, "response",
                             G_CALLBACK(fcitx_config_widget_response_cb),
                             config_widget);

            gtk_widget_show_all(GTK_WIDGET(dialog));
        }
    }
}

void open_native_file(GtkButton *button,
                      gpointer   user_data)
{
    FcitxSubConfigWidget* widget = (FcitxSubConfigWidget*) user_data;
    char *newpath = NULL;
    char* qtguiwrapper = fcitx_utils_get_fcitx_path_with_filename ("libdir", "fcitx/libexec/fcitx-qt-gui-wrapper");
    if (qtguiwrapper) {
        gchar* argv[4];
        argv[0] = qtguiwrapper;
        argv[1] = "--test";
        argv[2] = widget->subconfig->nativepath;
        argv[3] = 0;
        int exit_status = 1;
        g_spawn_sync(NULL, argv, NULL, 0, NULL, NULL, NULL, NULL, &exit_status, NULL);

        if (exit_status == 0) {
            gchar* argv2[3];
            argv2[0] = qtguiwrapper;
            argv2[1] = widget->subconfig->nativepath;
            argv2[2] = 0;
            g_spawn_async(NULL, argv2, NULL, 0, NULL, NULL, NULL, NULL);
            free(newpath);
        }
        g_free(qtguiwrapper);

        if (exit_status == 0) {
            return;
        }
    }

    if (g_hash_table_size(widget->subconfig->filelist) > 0) {
        GHashTableIter iter;
        g_hash_table_iter_init(&iter, widget->subconfig->filelist);
        gpointer key;
        if (g_hash_table_iter_next(&iter, &key, NULL)) {
            FILE* fp = FcitxXDGGetFileWithPrefix("",  key, "r", &newpath);
            if (fp)
                fclose(fp);
        }
    } else {
        FILE* fp = FcitxXDGGetFileUserWithPrefix("", widget->subconfig->nativepath, "w", &newpath);
        if (fp) {
            g_hash_table_insert(widget->subconfig->filelist, widget->subconfig->nativepath, NULL);
            fclose(fp);
        }
    }

    if (newpath) {
        gchar* filename = newpath;
        gchar* argv[3];
        argv[0] = "xdg-open";
        argv[1] = filename;
        argv[2] = 0;
        g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
        free(newpath);
    }
}

void run_program(GtkButton* button, gpointer user_data)
{
    FcitxSubConfigWidget* widget = (FcitxSubConfigWidget*) user_data;
    char*args[] = {widget->subconfig->path};
    fcitx_utils_start_process(args);

}

void run_plugin(GtkButton* button, gpointer user_data)
{
    FcitxSubConfigWidget* widget = (FcitxSubConfigWidget*) user_data;
    char *newpath = NULL;
    char* qtguiwrapper = fcitx_utils_get_fcitx_path_with_filename ("libdir", "fcitx/libexec/fcitx-qt-gui-wrapper");
    if (qtguiwrapper) {
        gchar* argv[4];
        argv[0] = qtguiwrapper;
        argv[1] = "--test";
        argv[2] = widget->subconfig->nativepath;
        argv[3] = 0;
        int exit_status = 1;
        g_spawn_sync(NULL, argv, NULL, 0, NULL, NULL, NULL, NULL, &exit_status, NULL);

        if (exit_status == 0) {
            gchar* argv2[3];
            argv2[0] = qtguiwrapper;
            argv2[1] = widget->subconfig->nativepath;
            argv2[2] = 0;
            g_spawn_async(NULL, argv2, NULL, 0, NULL, NULL, NULL, NULL);
            free(newpath);
        }
        g_free(qtguiwrapper);

        if (exit_status == 0) {
            return;
        }
    }

    GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(widget))),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                "%s", _("Didn't install related component."));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void push_into_store_cb(gpointer       data,
                        gpointer       value,
                        gpointer       user_data)
{
    GtkListStore* store = user_data;

    GtkTreeIter iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       0, data,
                       -1);
}
