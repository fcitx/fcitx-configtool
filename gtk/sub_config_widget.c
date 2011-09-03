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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <fcitx-config/xdg.h>
#include <stdlib.h>

#include "sub_config_widget.h"
#include "main_window.h"
#include "configdesc.h"
#include "config_widget.h"

G_DEFINE_TYPE (FcitxSubConfigWidget, fcitx_sub_config_widget, GTK_TYPE_VBOX)

static void open_subconfig_file (GtkButton *button, gpointer user_data);
static void open_native_file (GtkButton *button, gpointer user_data);
static void push_into_store_cb (gpointer data, gpointer user_data);

static void
fcitx_sub_config_widget_get_property (GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
fcitx_sub_config_widget_set_property (GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
fcitx_sub_config_widget_finalize (GObject *object)
{
    FcitxSubConfigWidget* widget = FCITX_SUB_CONFIG_WIDGET(object);
    sub_config_free(widget->subconfig);
    G_OBJECT_CLASS (fcitx_sub_config_widget_parent_class)->finalize (object);
}

static void
fcitx_sub_config_widget_class_init (FcitxSubConfigWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);


    object_class->get_property = fcitx_sub_config_widget_get_property;
    object_class->set_property = fcitx_sub_config_widget_set_property;
    object_class->finalize = fcitx_sub_config_widget_finalize;
}

static void
fcitx_sub_config_widget_init (FcitxSubConfigWidget *self)
{
}

FcitxSubConfigWidget*
fcitx_sub_config_widget_new (FcitxSubConfig* subconfig)
{
    FcitxSubConfigWidget* widget = g_object_new (FCITX_TYPE_SUB_CONFIG_WIDGET, NULL);

    widget->subconfig = subconfig;
    switch(subconfig->type)
    {
        case SC_ConfigFile:
            {
                GtkWidget* view = gtk_tree_view_new();
                gtk_box_pack_start(GTK_BOX(widget), view, FALSE, FALSE, 0);

                GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
                GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 0, NULL);
                gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
                GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
                gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

                GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);

                gtk_tree_view_set_model(GTK_TREE_VIEW(view),
                                        GTK_TREE_MODEL(store));

                g_list_foreach(widget->subconfig->filelist, push_into_store_cb, store);

                GtkWidget* button = gtk_button_new();
                gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_BUTTON));
                g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_subconfig_file), widget);
                gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 0);
                widget->view = view;
            }
            break;
        case SC_NativeFile:
            {
                GtkWidget* button = gtk_button_new();
                gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON));
                g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_native_file), widget);
                gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 0);
            }
            break;
        default:
            break;
    }

    return widget;
}

void open_subconfig_file (GtkButton *button,
                          gpointer   user_data)
{
    FcitxSubConfigWidget* widget = (FcitxSubConfigWidget*) user_data;
    GtkTreeView* view = GTK_TREE_VIEW(widget->view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeIter iter;
    gchar* configfile;
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter,
                0, &configfile,
                -1);
        ConfigFileDesc* cfdesc = get_config_desc(widget->subconfig->configdesc);
        if (cfdesc)
        {
            GtkWidget* dialog = gtk_dialog_new_with_buttons(configfile,
                                                            GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_STOCK_OK,
                                                            GTK_RESPONSE_OK,
                                                            GTK_STOCK_CANCEL,
                                                            GTK_RESPONSE_CANCEL,
                                                            NULL
                                                           );
            FcitxConfigWidget* config_widget = fcitx_config_widget_new(cfdesc, "", configfile, NULL);
            GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(config_widget), TRUE, TRUE, 0);
            gtk_widget_set_size_request(GTK_WIDGET(config_widget), -1, 400);

            g_signal_connect (dialog, "response",
                              G_CALLBACK (response_cb),
                              config_widget);

            gtk_widget_show_all (GTK_WIDGET(dialog));
        }
    }
}

void open_native_file (GtkButton *button,
                       gpointer   user_data)
{
    FcitxSubConfigWidget* widget = (FcitxSubConfigWidget*) user_data;
    char *newpath = NULL;
    if (g_list_length(widget->subconfig->filelist) > 0)
    {
        FILE* fp = GetXDGFileWithPrefix("", widget->subconfig->filelist->data, "r", &newpath);
        if (fp)
            fclose(fp);
    }
    else
    {
        FILE* fp = GetXDGFileUserWithPrefix("", widget->subconfig->nativepath, "w", &newpath);
        if (fp)
        {
            widget->subconfig->filelist = g_list_append(widget->subconfig->filelist, widget->subconfig->nativepath);
            fclose(fp);
        }
    }

    if (newpath)
    {
        GError* error;
        gchar* filename = newpath;
        gchar* argv[3];
        argv[0] = "xdg-open";
        argv[1] = filename;
        argv[2] = 0;
        g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
        free(newpath);
    }
}


void push_into_store_cb (gpointer       data,
                         gpointer       user_data)
{
    GtkListStore* store = user_data;

    GtkTreeIter iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       0, data,
                       -1);
}
