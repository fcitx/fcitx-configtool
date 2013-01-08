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

#include <fcitx-utils/utils.h>
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>

#include "common.h"
#include "ui_widget.h"
#include "gdm-languages.h"
#include "config_widget.h"
#include "main_window.h"
#include "configdesc.h"

G_DEFINE_TYPE(FcitxUIWidget, fcitx_ui_widget, GTK_TYPE_BOX)

static void fcitx_ui_widget_dispose(GObject* object);
static void _fcitx_ui_widget_load(FcitxUIWidget* self, const gchar* uiname);
static void _fcitx_ui_widget_apply_button_clicked(GtkButton* button, gpointer user_data);

static void
fcitx_ui_widget_class_init(FcitxUIWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->dispose = fcitx_ui_widget_dispose;
}

static void
fcitx_ui_widget_init(FcitxUIWidget* self)
{
    gtk_orientable_set_orientation(GTK_ORIENTABLE(self), GTK_ORIENTATION_VERTICAL);
    self->label = gtk_label_new(_("Cannot load currently used user interface info"));
    gtk_box_pack_start(GTK_BOX(self), self->label, TRUE, TRUE, 0);
}

GtkWidget*
fcitx_ui_widget_new(void)
{
    FcitxUIWidget* widget =
        g_object_new(FCITX_TYPE_UI_WIDGET,
                     NULL);

    return GTK_WIDGET(widget);
}

void fcitx_ui_widget_dispose(GObject* object)
{
    FcitxUIWidget* self = FCITX_UI_WIDGET(object);
    if (self->improxy) {
        g_object_unref(self->improxy);
        self->improxy = NULL;
    }

    G_OBJECT_CLASS (fcitx_ui_widget_parent_class)->dispose (object);
}

void fcitx_ui_widget_connect(FcitxUIWidget* self)
{
    GError* error = NULL;
    self->improxy = fcitx_input_method_new(G_BUS_TYPE_SESSION,
                                           G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                           fcitx_utils_get_display_number(),
                                           NULL,
                                           &error
                                          );
    if (self->improxy == NULL) {
        g_error_free(error);
        return;
    }

    gchar* uiname = fcitx_input_method_get_current_ui(self->improxy);
    if (uiname) {
        _fcitx_ui_widget_load(self, uiname);
        g_free(uiname);
    }
}

void _fcitx_ui_widget_load(FcitxUIWidget* self, const gchar* uiname)
{
    FcitxMainWindow* mainwindow = FCITX_MAIN_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(self)));
    FcitxAddon* addon = find_addon_by_name(mainwindow->addons, uiname);
    if (!addon)
        return;

    gchar* config_desc_name = g_strdup_printf("%s.desc", addon->name);
    FcitxConfigFileDesc* cfdesc = get_config_desc(config_desc_name);
    g_free(config_desc_name);
    gboolean configurable = (gboolean)(cfdesc != NULL || strlen(addon->subconfig) != 0);
    if (!configurable) {
        gchar* text = g_strdup_printf(_("No configuration option for %s."), addon->generalname);
        gtk_label_set_text(GTK_LABEL(self->label), text);
        g_free(text);
    }
    else {
        gtk_container_remove(GTK_CONTAINER(self), self->label);
        self->label = NULL;
        gchar* config_file_name = g_strdup_printf("%s.config", addon->name);
        FcitxConfigWidget* config_widget = fcitx_config_widget_new(cfdesc, "conf", config_file_name, addon->subconfig);
        g_free(config_file_name);
        gtk_box_pack_start(GTK_BOX(self), GTK_WIDGET(config_widget), TRUE, TRUE, 0);
        g_object_set(G_OBJECT(config_widget), "margin", 5, NULL);

        GtkWidget* hbuttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(self), hbuttonbox, FALSE, TRUE, 0);
        g_object_set(G_OBJECT(hbuttonbox), "margin", 5, NULL);

        GtkWidget* applybutton = gtk_button_new_from_stock(GTK_STOCK_APPLY);
        gtk_box_pack_start(GTK_BOX(hbuttonbox), applybutton, TRUE, TRUE, 0);
        g_signal_connect(G_OBJECT(applybutton), "clicked", G_CALLBACK(_fcitx_ui_widget_apply_button_clicked), config_widget);
        gtk_widget_show_all(GTK_WIDGET(self));
    }
}

void _fcitx_ui_widget_apply_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxConfigWidget* config_widget = user_data;
    fcitx_config_widget_response(config_widget, CONFIG_WIDGET_SAVE);
}
