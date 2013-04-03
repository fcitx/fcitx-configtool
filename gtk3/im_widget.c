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
#include "im_widget.h"
#include "gdm-languages.h"
#include "im_dialog.h"
#include "config_widget.h"
#include "main_window.h"
#include "im_config_dialog.h"

G_DEFINE_TYPE(FcitxImWidget, fcitx_im_widget, GTK_TYPE_BOX)

enum {
    AVAIL_TREE_IM_STRING,
    AVAIL_TREE_IM,
    AVAIL_TREE_LANG,
    AVAIL_N_COLUMNS
};

enum {
    IM_LIST_IM_STRING,
    IM_LIST_IM,
    IM_LIST_IM_LANGUAGE,
    IM_N_COLUMNS
};

typedef struct {
    FcitxImWidget* widget;
    GtkTreeIter iter;
    gboolean flag;
} foreach_ct;

static void fcitx_im_widget_dispose(GObject* object);
static void _fcitx_im_widget_connect(FcitxImWidget* self);
static void _fcitx_im_widget_load(FcitxImWidget* self);
static void _fcitx_inputmethod_insert_foreach_cb(gpointer data, gpointer user_data);
static void _fcitx_im_widget_im_selection_changed(GtkTreeSelection *selection, gpointer data);
static void _fcitx_im_widget_addim_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_delim_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_moveup_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_movedown_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_configure_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_default_layout_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data);

static void _fcitx_im_widget_row_activated(GtkTreeView       *tree_view,
                                           GtkTreePath       *path,
                                           GtkTreeViewColumn *column,
                                           gpointer           user_data);
static GObject *
fcitx_im_widget_constructor   (GType                  gtype,
                               guint                  n_properties,
                               GObjectConstructParam *properties);

static void
fcitx_im_widget_class_init(FcitxImWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->dispose = fcitx_im_widget_dispose;
    gobject_class->constructor = fcitx_im_widget_constructor;
}


static GObject *
fcitx_im_widget_constructor   (GType                  gtype,
                               guint                  n_properties,
                               GObjectConstructParam *properties)
{
    GObject *obj;
    FcitxImWidget *self;
    GtkWidget *widget;

    obj = G_OBJECT_CLASS (fcitx_im_widget_parent_class)->constructor (gtype, n_properties, properties);

    self = FCITX_IM_WIDGET (obj);

    widget = GTK_WIDGET(gtk_builder_get_object (self->builder,
                                                "im_widget"));

    gtk_widget_reparent (widget, GTK_WIDGET(self));

    _fcitx_im_widget_connect(self);

  return obj;
}

static void
fcitx_im_widget_init(FcitxImWidget* self)
{
    self->builder = gtk_builder_new();
    gtk_builder_add_from_resource(self->builder, "/org/fcitx/fcitx-config-gtk3/im_widget.ui", NULL);

#define _GET_OBJECT(NAME) \
    self->NAME = (typeof(self->NAME)) gtk_builder_get_object(self->builder, #NAME);

    _GET_OBJECT(imstore)
    _GET_OBJECT(imview)
    _GET_OBJECT(addimbutton)
    _GET_OBJECT(delimbutton)
    _GET_OBJECT(moveupbutton)
    _GET_OBJECT(movedownbutton)
    _GET_OBJECT(configurebutton)
    _GET_OBJECT(default_layout_button)
    _GET_OBJECT(scrolledwindow)
    _GET_OBJECT(toolbar)

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(_fcitx_im_widget_im_selection_changed), self);


    GtkStyleContext* context;
    context = gtk_widget_get_style_context (self->scrolledwindow);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    context = gtk_widget_get_style_context (self->toolbar);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_style_context_add_class (context, "inline-toolbar");

    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(self->addimbutton), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("list-add-symbolic"), GTK_ICON_SIZE_BUTTON));
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(self->delimbutton), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("list-remove-symbolic"), GTK_ICON_SIZE_BUTTON));
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(self->moveupbutton), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("go-up-symbolic"), GTK_ICON_SIZE_BUTTON));
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(self->movedownbutton), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("go-down-symbolic"), GTK_ICON_SIZE_BUTTON));
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(self->configurebutton), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("preferences-system-symbolic"), GTK_ICON_SIZE_BUTTON));
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(self->default_layout_button), gtk_image_new_from_gicon(g_themed_icon_new_with_default_fallbacks("input-keyboard-symbolic"), GTK_ICON_SIZE_BUTTON));

    g_signal_connect(G_OBJECT(self->addimbutton), "clicked", G_CALLBACK(_fcitx_im_widget_addim_button_clicked), self);
    g_signal_connect(G_OBJECT(self->delimbutton), "clicked", G_CALLBACK(_fcitx_im_widget_delim_button_clicked), self);
    g_signal_connect(G_OBJECT(self->moveupbutton), "clicked", G_CALLBACK(_fcitx_im_widget_moveup_button_clicked), self);
    g_signal_connect(G_OBJECT(self->movedownbutton), "clicked", G_CALLBACK(_fcitx_im_widget_movedown_button_clicked), self);
    g_signal_connect(G_OBJECT(self->configurebutton), "clicked", G_CALLBACK(_fcitx_im_widget_configure_button_clicked), self);
    g_signal_connect(G_OBJECT(self->default_layout_button), "clicked", G_CALLBACK(_fcitx_im_widget_default_layout_button_clicked), self);
    g_signal_connect(G_OBJECT(self->imview), "row-activated", G_CALLBACK(_fcitx_im_widget_row_activated), self);
}

GtkWidget*
fcitx_im_widget_new(void)
{
    FcitxImWidget* widget =
        g_object_new(FCITX_TYPE_IM_WIDGET,
                     NULL);

    return GTK_WIDGET(widget);
}

void fcitx_im_widget_dispose(GObject* object)
{
    FcitxImWidget* self = FCITX_IM_WIDGET(object);
    if (self->array) {
        g_ptr_array_set_free_func(self->array, (GDestroyNotify) fcitx_im_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }

    if (self->improxy) {
        g_signal_handlers_disconnect_by_func(self->improxy, G_CALLBACK(_fcitx_im_widget_imlist_changed_cb), self);
        g_object_unref(self->improxy);
        self->improxy = NULL;
    }

    if (self->focus) {
        g_free(self->focus);
        self->focus = NULL;
    }

    G_OBJECT_CLASS (fcitx_im_widget_parent_class)->dispose (object);
}

void _fcitx_im_widget_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    _fcitx_im_widget_load(self);
}

void _fcitx_im_widget_connect(FcitxImWidget* self)
{
    GError* error = NULL;
    self->improxy = fcitx_input_method_new(G_BUS_TYPE_SESSION,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          fcitx_utils_get_display_number(),
                                          NULL,
                                          &error
                                         );
    if (self->improxy == NULL) {
        g_error_free(error);
        return;
    }
    g_signal_connect(self->improxy, "imlist-changed", G_CALLBACK(_fcitx_im_widget_imlist_changed_cb), self);

    _fcitx_im_widget_load(self);
}

void _fcitx_im_widget_load(FcitxImWidget* self)
{
    gtk_list_store_clear(self->imstore);

    if (self->array) {
        g_ptr_array_set_free_func(self->array, (GDestroyNotify) fcitx_im_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }

    self->array = fcitx_input_method_get_imlist(self->improxy);

    if (self->array) {
        g_ptr_array_set_free_func(self->array, NULL);
        foreach_ct context;
        context.widget = self;
        context.flag = FALSE;
        g_ptr_array_foreach(self->array, _fcitx_inputmethod_insert_foreach_cb, &context);

        if (context.flag)
            gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview)), &context.iter);
        g_free(self->focus);
        self->focus = NULL;

        _fcitx_im_widget_im_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview)), self);
    }
}

void _fcitx_inputmethod_insert_foreach_cb(gpointer data,
        gpointer user_data)
{
    foreach_ct* context = user_data;
    FcitxIMItem* item = data;
    FcitxImWidget* self = context->widget;
    GtkTreeIter iter;

    if (item->enable) {
        context->flag = TRUE;
        gtk_list_store_append(self->imstore, &iter);
        gtk_list_store_set(self->imstore, &iter, IM_LIST_IM_STRING, item->name, -1);
        gtk_list_store_set(self->imstore, &iter, IM_LIST_IM, item, -1);
        char* lang = NULL;
        if (strlen(item->langcode) != 0)
            lang = gdm_get_language_from_name(item->langcode, NULL);
        if (!lang) {
            if (strcmp(item->langcode, "*") == 0)
                lang = g_strdup_printf("%s", _("Unknown"));
            else
                lang = g_strdup_printf("%s", _("Unknown"));
        }
        gtk_list_store_set(self->imstore, &iter, IM_LIST_IM_LANGUAGE, lang, -1);
        if (self->focus == NULL || strcmp(self->focus, item->unique_name) == 0)
            context->iter = iter;
    }
}

void _fcitx_im_widget_im_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    FcitxImWidget* self = data;
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_widget_set_sensitive(GTK_WIDGET(self->delimbutton), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(self->configurebutton), TRUE);
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);

        gint* ind = gtk_tree_path_get_indices(path);

        gint n = gtk_tree_model_iter_n_children(model, NULL);

        if (ind) {
            gtk_widget_set_sensitive(GTK_WIDGET(self->moveupbutton), (*ind != 0));
            gtk_widget_set_sensitive(GTK_WIDGET(self->movedownbutton), (*ind != n - 1));
        }

        gtk_tree_path_free(path);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(self->delimbutton), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(self->configurebutton), FALSE);
    }
}


void _fcitx_im_widget_addim_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget* dialog = fcitx_im_dialog_new(GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(self))));
    gtk_widget_show_all(dialog);

    g_free(self->focus);
    self->focus = NULL;
}

void _fcitx_im_widget_delim_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->imview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get(model,
                           &iter,
                           IM_LIST_IM, &item,
                           -1);
        item->enable = false;

        g_free(self->focus);
        self->focus = NULL;

        fcitx_input_method_set_imlist(self->improxy, self->array);
    }

}

void _fcitx_im_widget_moveup_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->imview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get(model,
                           &iter,
                           IM_LIST_IM, &item,
                           -1);

        int i;
        int switch_index = self->array->len;
        for (i = 0; i < self->array->len; i += 1) {
            if (g_ptr_array_index(self->array, i) == item)
                break;

            FcitxIMItem* temp_item = g_ptr_array_index(self->array, i);
            if (temp_item->enable)
                switch_index = i;
        }

        if (i != self->array->len && switch_index != self->array->len) {
            gpointer temp = g_ptr_array_index(self->array, i);
            g_ptr_array_index(self->array, i) = g_ptr_array_index(self->array, switch_index);
            g_ptr_array_index(self->array, switch_index) = temp;
            g_free(self->focus);
            self->focus = g_strdup(item->unique_name);

            fcitx_input_method_set_imlist(self->improxy, self->array);
        }
    }
}

void _fcitx_im_widget_movedown_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->imview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get(model,
                           &iter,
                           IM_LIST_IM, &item,
                           -1);

        int i;
        int switch_index = -1;
        for (i = self->array->len - 1; i >= 0; i -= 1) {
            if (g_ptr_array_index(self->array, i) == item)
                break;

            FcitxIMItem* temp_item = g_ptr_array_index(self->array, i);
            if (temp_item->enable)
                switch_index = i;
        }

        if (i != -1 && switch_index != -1) {
            gpointer temp = g_ptr_array_index(self->array, i);
            g_ptr_array_index(self->array, i) = g_ptr_array_index(self->array, switch_index);
            g_ptr_array_index(self->array, switch_index) = temp;
            g_free(self->focus);
            self->focus = g_strdup(item->unique_name);

            fcitx_input_method_set_imlist(self->improxy, self->array);
        }
    }
}

void _fcitx_im_widget_default_layout_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    FcitxMainWindow* mainwindow = FCITX_MAIN_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(self)));
    do {
        if (!mainwindow)
            break;

        GtkWidget* dialog = fcitx_im_config_dialog_new(GTK_WINDOW(mainwindow), NULL, "default");
        if (dialog)
            gtk_widget_show_all(GTK_WIDGET(dialog));
    } while(0);
}

static void _fcitx_im_widget_configure_im(FcitxImWidget* self, FcitxIMItem* item)
{
    gchar* addon_name = fcitx_input_method_get_im_addon(self->improxy, item->unique_name);
    FcitxMainWindow* mainwindow = FCITX_MAIN_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(self)));
    do {
        if (!mainwindow)
            break;

        FcitxAddon* addon = find_addon_by_name(mainwindow->addons, addon_name);
        if (!addon)
            break;

        GtkWidget* dialog = fcitx_im_config_dialog_new(GTK_WINDOW(mainwindow), addon, item->unique_name);
        if (dialog)
            gtk_widget_show_all(GTK_WIDGET(dialog));
    } while(0);
    g_free(addon_name);
}

void _fcitx_im_widget_configure_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->imview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get(model,
                           &iter,
                           IM_LIST_IM, &item,
                           -1);

        _fcitx_im_widget_configure_im((FcitxImWidget*)user_data, item);
    }
}

void _fcitx_im_widget_row_activated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get(model, &iter,
                           IM_LIST_IM, &item,
                           -1);

        _fcitx_im_widget_configure_im((FcitxImWidget*)user_data, item);
    }
}
