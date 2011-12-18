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

#include <fcitx-utils/utils.h>
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>

#include "common.h"
#include "im_widget.h"
#include "im.h"
#include "gdm-languages.h"

G_DEFINE_TYPE (FcitxImWidget, fcitx_im_widget, GTK_TYPE_HBOX)

enum
{
    LIST_IM,
    N_COLUMNS
};

static void fcitx_im_widget_finalize(GObject* object);
static void _fcitx_im_widget_connect(FcitxImWidget* self);
static void _fcitx_im_widget_load(FcitxImWidget* self);
static void _fcitx_inputmethod_insert_foreach_cb(gpointer data, gpointer user_data);
static void _fcitx_im_widget_availname_data_func (GtkCellLayout   *cell_layout,
                GtkCellRenderer *renderer,
                GtkTreeModel    *tree_model,
                GtkTreeIter     *iter,
                gpointer         user_data);
static void _fcitx_im_widget_name_data_func (GtkCellLayout   *cell_layout,
                GtkCellRenderer *renderer,
                GtkTreeModel    *tree_model,
                GtkTreeIter     *iter,
                gpointer         user_data);

static void _fcitx_im_widget_availim_selection_changed(GtkTreeSelection *selection, gpointer data);
static void _fcitx_im_widget_im_selection_changed(GtkTreeSelection *selection, gpointer data);
static void _fcitx_im_widget_addim_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_delim_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_moveup_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_movedown_button_clicked(GtkButton* button, gpointer user_data);
static void _fcitx_im_widget_filtertext_changed(GtkEditable *editable, gpointer user_data);
static gboolean _fcitx_im_widget_filter_func (GtkTreeModel *model,
                                              GtkTreeIter  *iter,
                                              gpointer      data);

static void
fcitx_im_widget_class_init (FcitxImWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = fcitx_im_widget_finalize;
}

static void
fcitx_im_widget_init (FcitxImWidget* self)
{
    self->availimstore = gtk_list_store_new(N_COLUMNS, G_TYPE_POINTER);
    self->filtermodel = gtk_tree_model_filter_new(GTK_TREE_MODEL(self->availimstore), NULL);
   
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER ( self->filtermodel ),
                                            (GtkTreeModelFilterVisibleFunc) _fcitx_im_widget_filter_func,
                                            self ,
                                            NULL);
    self->availimview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->filtermodel));
    
    GtkWidget* label = gtk_label_new(_("Available Input Method"));
    self->filterentry = gtk_entry_new();
    
    GtkCellRenderer* renderer;
    GtkTreeViewColumn* column;
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
            _("Input Method"), renderer,
            NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (self->availimview), column);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (column),
                                        renderer,
                                        _fcitx_im_widget_availname_data_func,
                                        self->availimview,
                                        NULL);

    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
            G_CALLBACK(_fcitx_im_widget_availim_selection_changed), self);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self->availimview), FALSE);
    
    GtkWidget* vbox;
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), self->filterentry, FALSE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), self->availimview, TRUE, TRUE, 5);
    
    
    gtk_box_pack_start(GTK_BOX(self), vbox, TRUE, TRUE, 5);
    
    vbox = gtk_vbox_new(FALSE, 0);
    
    self->addimbutton = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(self->addimbutton), gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_sensitive(self->addimbutton, FALSE);
    
    self->delimbutton = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(self->delimbutton), gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_sensitive(self->delimbutton, FALSE);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(""), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), self->addimbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), self->delimbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(""), TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(self), vbox, FALSE, TRUE, 5);
    
    label = gtk_label_new(_("Current Input Method"));
    
    self->imstore = gtk_list_store_new(N_COLUMNS, G_TYPE_POINTER);
    self->imview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->imstore));

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
            _("Input Method"), renderer,
            NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (self->imview), column);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (column),
                                        renderer,
                                        _fcitx_im_widget_name_data_func,
                                        self->imview,
                                        NULL);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self->imview), FALSE);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
            G_CALLBACK(_fcitx_im_widget_im_selection_changed), self);
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), self->imview, TRUE, TRUE, 5);
    
    gtk_box_pack_start(GTK_BOX(self), GTK_WIDGET(vbox), TRUE, TRUE, 5);

    vbox = gtk_vbox_new(FALSE, 0);

    self->moveupbutton = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(self->moveupbutton), gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_sensitive(self->moveupbutton, FALSE);

    self->movedownbutton = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(self->movedownbutton), gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_sensitive(self->movedownbutton, FALSE);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(""), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), self->moveupbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), self->movedownbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(""), TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(self), vbox, FALSE, TRUE, 5);
    
    g_signal_connect(G_OBJECT(self->addimbutton), "clicked", G_CALLBACK(_fcitx_im_widget_addim_button_clicked), self);
    g_signal_connect(G_OBJECT(self->delimbutton), "clicked", G_CALLBACK(_fcitx_im_widget_delim_button_clicked), self);
    g_signal_connect(G_OBJECT(self->moveupbutton), "clicked", G_CALLBACK(_fcitx_im_widget_moveup_button_clicked), self);
    g_signal_connect(G_OBJECT(self->movedownbutton), "clicked", G_CALLBACK(_fcitx_im_widget_movedown_button_clicked), self);
    g_signal_connect(G_OBJECT(self->filterentry), "changed", G_CALLBACK(_fcitx_im_widget_filtertext_changed), self);
    
    _fcitx_im_widget_connect(self);
}

GtkWidget*
fcitx_im_widget_new (void)
{
    FcitxImWidget* widget = 
        g_object_new(FCITX_TYPE_IM_WIDGET,
                     NULL);
        
    return GTK_WIDGET(widget);
}

void fcitx_im_widget_finalize ( GObject* object )
{

}

void _fcitx_im_widget_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    _fcitx_im_widget_load(self);
}

void _fcitx_im_widget_connect(FcitxImWidget* self)
{
    GError* error = NULL;
    self->improxy = fcitx_inputmethod_new(G_BUS_TYPE_SESSION,
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
    gtk_list_store_clear(self->availimstore);
    gtk_list_store_clear(self->imstore);

    if (self->array) {
        g_ptr_array_set_free_func(self->array, fcitx_inputmethod_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }
    
    self->array = fcitx_inputmethod_get_imlist(self->improxy);
    
    if (self->array)
        g_ptr_array_foreach(self->array, _fcitx_inputmethod_insert_foreach_cb, self);
}

void _fcitx_inputmethod_insert_foreach_cb(gpointer data,
                                          gpointer user_data)
{
    FcitxIMItem* item = data;
    FcitxImWidget* self = user_data;
    GtkTreeIter iter;
    
    if (item->enable) {
        gtk_list_store_append(self->imstore, &iter);
        gtk_list_store_set(self->imstore, &iter, LIST_IM, item, -1);
    }
    else {
        gtk_list_store_append(self->availimstore, &iter);
        gtk_list_store_set(self->availimstore, &iter, LIST_IM, item, -1);
    }

}

static void
_fcitx_im_widget_name_data_func (GtkCellLayout   *cell_layout,
                GtkCellRenderer *renderer,
                GtkTreeModel    *tree_model,
                GtkTreeIter     *iter,
                gpointer         user_data)
{
    FcitxIMItem* item;

    gtk_tree_model_get (tree_model,
                        iter,
                        LIST_IM, &item,
                        -1);
    g_object_set (renderer,
                "text", item->name,
                NULL);
}

static void
_fcitx_im_widget_availname_data_func (GtkCellLayout   *cell_layout,
                GtkCellRenderer *renderer,
                GtkTreeModel    *tree_model,
                GtkTreeIter     *iter,
                gpointer         user_data)
{
    FcitxIMItem* item;

    gtk_tree_model_get (tree_model,
                        iter,
                        LIST_IM, &item,
                        -1);
    gchar* string = g_strdup_printf(_("%s - %s"), gdm_get_language_from_name(item->langcode, NULL), item->name);
    g_object_set (renderer,
                "text", string,
                NULL);

    g_free(string);
}

void _fcitx_im_widget_im_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    FcitxImWidget* self = data;
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_widget_set_sensitive(self->delimbutton, TRUE);
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
        
        gint* ind = gtk_tree_path_get_indices(path);
        
        gint n = gtk_tree_model_iter_n_children (model, NULL);
        
        if (ind) {
            gtk_widget_set_sensitive(self->moveupbutton, (*ind != 0));
            gtk_widget_set_sensitive(self->movedownbutton, (*ind != n - 1));
        }
        
        gtk_tree_path_free(path);
    }
    else
    {
        gtk_widget_set_sensitive(self->delimbutton, FALSE);
    }
}

void _fcitx_im_widget_availim_selection_changed(GtkTreeSelection* selection, gpointer data)
{
    FcitxImWidget* self = data;
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_widget_set_sensitive(self->addimbutton, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(self->addimbutton, FALSE);
    }

}

void _fcitx_im_widget_addim_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->availimview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get (model,
                            &iter,
                            LIST_IM, &item,
                            -1);
        item->enable = true;
        
        g_ptr_array_remove(self->array, item);
        g_ptr_array_add(self->array, item);
        
        fcitx_inputmethod_set_imlist(self->improxy, self->array);
    }
}

void _fcitx_im_widget_delim_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->imview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get (model,
                            &iter,
                            LIST_IM, &item,
                            -1);
        item->enable = false;
        
        fcitx_inputmethod_set_imlist(self->improxy, self->array);
    }

}

void _fcitx_im_widget_moveup_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    GtkWidget *treeView = self->imview;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->imview));
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get (model,
                            &iter,
                            LIST_IM, &item,
                            -1);
        
        int i;
        int switch_index = self->array->len;
        for (i = 0; i < self->array->len; i += 1)
        {
            if (g_ptr_array_index(self->array, i) == item)
                break;
            
            FcitxIMItem* temp_item = g_ptr_array_index(self->array, i);
            if (temp_item->enable)
                switch_index = i;
        }
        
        if (i != self->array->len && switch_index != self->array->len)
        {
            gpointer temp = g_ptr_array_index(self->array, i);
            g_ptr_array_index(self->array, i) = g_ptr_array_index(self->array, switch_index);
            g_ptr_array_index(self->array, switch_index) = temp;
        
            fcitx_inputmethod_set_imlist(self->improxy, self->array);
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

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        FcitxIMItem* item = NULL;
        gtk_tree_model_get (model,
                            &iter,
                            LIST_IM, &item,
                            -1);
        
        int i;
        int switch_index = -1;
        for (i = self->array->len - 1; i >= 0; i -= 1)
        {
            if (g_ptr_array_index(self->array, i) == item)
                break;
            
            FcitxIMItem* temp_item = g_ptr_array_index(self->array, i);
            if (temp_item->enable)
                switch_index = i;
        }
        
        if (i != -1 && switch_index != -1)
        {
            gpointer temp = g_ptr_array_index(self->array, i);
            g_ptr_array_index(self->array, i) = g_ptr_array_index(self->array, switch_index);
            g_ptr_array_index(self->array, switch_index) = temp;
        
            fcitx_inputmethod_set_imlist(self->improxy, self->array);
        }
    }
}

void _fcitx_im_widget_filtertext_changed(GtkEditable* editable, gpointer user_data)
{
    FcitxImWidget* self = user_data;
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(self->filtermodel));
}


gboolean _fcitx_im_widget_filter_func (GtkTreeModel *model,
                                         GtkTreeIter  *iter,
                                         gpointer      data)
{
    FcitxImWidget* self = data;
    const gchar* filter_text = gtk_entry_get_text(GTK_ENTRY(self->filterentry));
    FcitxIMItem* item = NULL;
    gtk_tree_model_get (GTK_TREE_MODEL(self->availimstore),
                        iter,
                        LIST_IM, &item,
                        -1);
    
    if (item && (strlen(filter_text) == 0
                || strstr(item->name, filter_text)
                || strstr(item->unique_name, filter_text)
                || strstr(item->langcode, filter_text)))
        return TRUE;
    else
        return FALSE;
}