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

#include <fcitx-gclient/fcitxkbd.h>

#include "common.h"
#include "im_config_dialog.h"
#include "config_widget.h"
#include "configdesc.h"

G_DEFINE_TYPE(FcitxImConfigDialog, fcitx_im_config_dialog, GTK_TYPE_DIALOG)

enum {
    LIST_NAME,
    LIST_LAYOUT,
    LIST_VARIANT,
    N_COLUMNS
};

static
void _fcitx_im_config_dialog_response_cb(GtkDialog *dialog,
                                  gint response,
                                  gpointer user_data);
static
void fcitx_im_config_dialog_finalize(GObject* object);

static void
fcitx_im_config_dialog_class_init(FcitxImConfigDialogClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = fcitx_im_config_dialog_finalize;
}

void fcitx_im_config_dialog_finalize(GObject* object)
{
    FcitxImConfigDialog* self = FCITX_IM_CONFIG_DIALOG(object);
    g_free(self->imname);

    if (self->kbd)
        g_object_unref(self->kbd);

    G_OBJECT_CLASS (fcitx_im_config_dialog_parent_class)->finalize (object);
}

static void
fcitx_im_config_dialog_init(FcitxImConfigDialog* self)
{
    gtk_window_set_modal(GTK_WINDOW(self), TRUE);

    gtk_dialog_add_buttons(GTK_DIALOG(self),
                           GTK_STOCK_CANCEL,
                           GTK_RESPONSE_CANCEL,
                           GTK_STOCK_OK,
                           GTK_RESPONSE_OK,
                           NULL
                          );
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (self),
                                         GTK_RESPONSE_OK,
                                         GTK_RESPONSE_CANCEL,
                                         -1);

    self->kbd = fcitx_kbd_new(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, fcitx_utils_get_display_number(), NULL, NULL);

    g_signal_connect(self, "response",
                    G_CALLBACK(_fcitx_im_config_dialog_response_cb),
                    NULL);
}

typedef struct {
    GtkListStore* model;
    gchar* layout;
    gchar* variant;
    gboolean flag;
    GtkTreeIter iter;
} layout_foreach_ct;

static void
layout_foreach_cb(gpointer data, gpointer user_data)
{
    layout_foreach_ct* context = user_data;
    GtkListStore* model = context->model;
    FcitxLayoutItem* item = data;
    GtkTreeIter iter;

    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, LIST_NAME, item->name, LIST_LAYOUT, item->layout, LIST_VARIANT, item->variant, -1);

    if (strcmp(item->layout, context->layout) == 0
        && strcmp(item->variant, context->variant) == 0)
    {
        context->flag = TRUE;
        context->iter = iter;
    }
}


GtkWidget* fcitx_im_config_dialog_new(GtkWindow* parent, FcitxAddon* addon, gchar* imname)
{
    FcitxImConfigDialog* self =
        g_object_new(FCITX_TYPE_IM_CONFIG_DIALOG,
                     NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (self), parent);

    gtk_window_set_title(GTK_WINDOW(self), addon->generalname);
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(self));
    self->imname = g_strdup(imname);

    do {

        if (strncmp(imname, "fcitx-keyboard", strlen("fcitx-keyboard")) == 0)
            break;
        gchar* layout = NULL, *variant = NULL;
        fcitx_kbd_get_layout_for_im(self->kbd, imname, &layout, &variant);

        if (layout == NULL || variant == NULL)
            break;

        GPtrArray* layouts = fcitx_kbd_get_layouts(self->kbd);

        if (!layouts)
            break;

        self->model = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        GtkTreeIter iter;
        gtk_list_store_append(self->model, &iter);
        gtk_list_store_set(self->model, &iter, LIST_NAME, _("System Default"), LIST_LAYOUT, "", LIST_VARIANT, "", -1);
        layout_foreach_ct context;
        context.model = self->model;
        context.layout = layout;
        context.variant = variant;
        context.flag = FALSE;
        g_ptr_array_foreach(layouts, layout_foreach_cb, &context);
        g_ptr_array_free(layouts, FALSE);

        GtkWidget* combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(self->model));
        gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(combobox), LIST_NAME);

        if (!gtk_combo_box_get_has_entry (GTK_COMBO_BOX (combobox)))
        {
            GtkCellRenderer *cell;
            cell = gtk_cell_renderer_text_new ();
            gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), cell, TRUE);
            gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), cell,
                                            "text", LIST_NAME,
                                            NULL);
        }


        if (context.flag)
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combobox), &context.iter);
        else
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combobox), &iter);
        GtkWidget* label = gtk_label_new(_("Keyboard layout:"));
        g_object_set(label, "xalign", 0.0f, NULL);
        gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(content_area), combobox, FALSE, TRUE, 5);
        self->combobox = combobox;

    } while(0);

    gchar* config_desc_name = g_strdup_printf("%s.desc", addon->name);
    FcitxConfigFileDesc* cfdesc = get_config_desc(config_desc_name);
    g_free(config_desc_name);
    gboolean configurable = (gboolean)(cfdesc != NULL || strlen(addon->subconfig) != 0);
    if (configurable) {
        GtkWidget* label = gtk_label_new(_("Input method settings:"));
        g_object_set(label, "xalign", 0.0f, NULL);
        gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, TRUE, 5);
        gchar* config_file_name = g_strdup_printf("%s.config", addon->name);
        self->config_widget = fcitx_config_widget_new(cfdesc, "conf", config_file_name, addon->subconfig);
        gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(self->config_widget), TRUE, TRUE, 0);
        g_free(config_file_name);
        gtk_widget_set_size_request(GTK_WIDGET(self->config_widget), -1, 400);
    }


    g_signal_connect(self, "response",
                    G_CALLBACK(_fcitx_im_config_dialog_response_cb),
                    NULL);

    return GTK_WIDGET(self);
}


void _fcitx_im_config_dialog_response_cb(GtkDialog *dialog,
                                  gint response,
                                  gpointer user_data)
{
    if (response == GTK_RESPONSE_OK) {
        FcitxImConfigDialog* self = FCITX_IM_CONFIG_DIALOG(dialog);
        if (self->config_widget) {
            FcitxConfigWidget* config_widget = self->config_widget;
            fcitx_config_widget_response(config_widget, CONFIG_WIDGET_SAVE);
        }

        do {
            if (!self->combobox)
                break;
            GtkTreeIter iter;
            if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(self->combobox), &iter))
                break;

            gchar* layout, *variant;
            gtk_tree_model_get(GTK_TREE_MODEL(self->model), &iter, LIST_LAYOUT, &layout, LIST_VARIANT, &variant, -1);

            fcitx_kbd_set_layout_for_im(self->kbd, self->imname, layout, variant);
            g_free(layout);
            g_free(variant);
        } while (0);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}
