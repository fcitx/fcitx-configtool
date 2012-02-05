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

#include <stdlib.h>
#include <libintl.h>
#include <errno.h>

#include <fcitx/addon.h>
#include <fcitx-utils/utarray.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

#include "config.h"
#include "main_window.h"
#include "config_widget.h"
#include "configdesc.h"
#include "im_widget.h"

enum {
    LIST_ADDON,
    N_COLUMNS
};

G_DEFINE_TYPE(FcitxMainWindow, fcitx_main_window, GTK_TYPE_WINDOW)

static void fcitx_main_window_finalize(GObject* object);

static GtkListStore *_fcitx_main_window_create_model();

static void _fcitx_main_window_add_config_file_page(FcitxMainWindow* self);

static void _fcitx_main_window_add_addon_page(FcitxMainWindow* self);

static void _fcitx_main_window_add_im_page(FcitxMainWindow* self);

static int _fcitx_main_window_close_cb(GtkWidget *theWindow, gpointer data);

static void _fcitx_main_window_selection_changed_cb(GtkTreeSelection *selection, gpointer data);

static ConfigPage* _fcitx_main_window_add_page(FcitxMainWindow* self, const char* name, GtkWidget* widget);

static void _fcitx_main_window_addon_selection_changed(GtkTreeSelection *selection, gpointer data);

static void _fcitx_main_window_configure_button_clicked(GtkButton *button, gpointer data);

static void _fcitx_main_window_enabled_data_func(GtkCellLayout   *cell_layout,
        GtkCellRenderer *renderer,
        GtkTreeModel    *tree_model,
        GtkTreeIter     *iter,
        gpointer         user_data);

static void _fcitx_main_window_name_data_func(GtkCellLayout   *cell_layout,
        GtkCellRenderer *renderer,
        GtkTreeModel    *tree_model,
        GtkTreeIter     *iter,
        gpointer         user_data);

static void _fcitx_main_window_apply_button_clicked(GtkButton *button,
        gpointer   user_data);

static void _fcitx_main_window_toggled_cb(GtkCellRenderer *renderer,
        gchar* str_path,
        gpointer         user_data);

const UT_icd addonicd = {sizeof(FcitxAddon), 0, 0, FcitxAddonFree};

static void
fcitx_main_window_class_init(FcitxMainWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = fcitx_main_window_finalize;
}

static void
fcitx_main_window_init(FcitxMainWindow* self)
{
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    self->pagestore = _fcitx_main_window_create_model();
    self->pageview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->pagestore));

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
                 _("Config"), renderer,
                 "text", 0,
                 NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->pageview), column);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self->pageview), FALSE);

    _fcitx_main_window_add_im_page(self);
    _fcitx_main_window_add_config_file_page(self);
    _fcitx_main_window_add_addon_page(self);

    gtk_widget_set_size_request(self->pageview, 170, -1);
    gtk_widget_set_size_request(GTK_WIDGET(self), -1, 500);

    self->hpaned = gtk_hpaned_new();

    gtk_paned_add1(GTK_PANED(self->hpaned), self->pageview);

    gtk_box_pack_start(GTK_BOX(vbox), self->hpaned, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(self), vbox);

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->pageview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    g_signal_connect(G_OBJECT(self), "destroy", G_CALLBACK(_fcitx_main_window_close_cb), NULL);
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(_fcitx_main_window_selection_changed_cb), self);

    gtk_tree_selection_select_iter(selection, &self->impage->iter);

    gtk_window_set_icon_name(GTK_WINDOW(self), "fcitx-configtool");
    gtk_window_set_title(GTK_WINDOW(self), _("Fcitx Config"));
}

GtkWidget*
fcitx_main_window_new()
{
    FcitxMainWindow* widget =
        g_object_new(FCITX_TYPE_MAIN_WINDOW,
                     NULL);

    return GTK_WIDGET(widget);
}

void fcitx_main_window_finalize(GObject* object)
{
    FcitxMainWindow* self = FCITX_MAIN_WINDOW(object);
    utarray_free(self->addons);
}

int _fcitx_main_window_close_cb(GtkWidget *theWindow, gpointer data)
{
    gtk_main_quit();
    return 0;
}

ConfigPage* _fcitx_main_window_add_page(FcitxMainWindow* self, const char* name, GtkWidget* widget)
{
    ConfigPage *page = (ConfigPage*) malloc(sizeof(ConfigPage));
    memset(page, 0, sizeof(ConfigPage));

    page->page = widget;

    g_object_ref(page->page);

    gtk_widget_show_all(widget);

    gtk_list_store_append(self->pagestore, &page->iter);
    gtk_list_store_set(self->pagestore, &page->iter, 0, name, 1, page, -1);

    return page;
}

void _fcitx_main_window_selection_changed_cb(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;
    ConfigPage* page;
    FcitxMainWindow* self = data;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           1, &page,
                           -1);

        if (self->lastpage)
            gtk_container_remove(GTK_CONTAINER(self->hpaned), self->lastpage->page);
        gtk_paned_add2(GTK_PANED(self->hpaned), page->page);
        gtk_widget_show_all(GTK_WIDGET(self));

        self->lastpage = page;
    } else {
        gtk_tree_selection_select_iter(selection, &self->configpage->iter);
    }
}


void _fcitx_main_window_addon_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;
    FcitxAddon *addon = NULL;
    FcitxMainWindow* self = data;

    if (!self->button)
        return;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           LIST_ADDON, &addon,
                           -1);
        gchar* config_desc_name = g_strdup_printf("%s.desc", addon->name);
        FcitxConfigFileDesc* cfdesc = get_config_desc(config_desc_name);
        g_free(config_desc_name);
        gboolean configurable = (gboolean)(cfdesc != NULL || strlen(addon->subconfig) != 0);
        gtk_widget_set_sensitive(self->button, configurable);
    } else {
        gtk_widget_set_sensitive(self->button, FALSE);
    }
}

static GtkListStore *_fcitx_main_window_create_model()
{
    GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
    return store;
}

void _fcitx_main_window_add_config_file_page(FcitxMainWindow* self)
{
    GtkWidget* vbox = gtk_vbox_new(0, 0);

    FcitxConfigWidget* config_widget = fcitx_config_widget_new(
                                           get_config_desc("config.desc"),
                                           "",
                                           "config",
                                           NULL
                                       );
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(config_widget), TRUE, TRUE, 0);

    GtkWidget* hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, TRUE, 0);

    GtkWidget* applybutton = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), applybutton, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(applybutton), "clicked", G_CALLBACK(_fcitx_main_window_apply_button_clicked), config_widget);


    self->configpage = _fcitx_main_window_add_page(self, _("Global Config"), vbox);
}

void _fcitx_main_window_add_im_page(FcitxMainWindow* self)
{
    GtkWidget* imwidget = fcitx_im_widget_new();
    self->impage = _fcitx_main_window_add_page(self, _("Input Method Configuration"), imwidget);
}

void _fcitx_main_window_add_addon_page(FcitxMainWindow* self)
{
    FcitxAddon* addon;
    utarray_new(self->addons, &addonicd);
    FcitxAddonsLoad(self->addons);

    GtkWidget* vbox = gtk_vbox_new(FALSE, 0);

    GtkListStore *store;
    store = gtk_list_store_new(N_COLUMNS, G_TYPE_POINTER);

    GtkWidget* swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);
    g_object_set(swin, "hscrollbar-policy", GTK_POLICY_NEVER, NULL);
    self->addonview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_set(self->addonview, "headers-visible", FALSE, NULL);
    gtk_container_add(GTK_CONTAINER(swin), self->addonview);
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Enable", renderer, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->addonview), column);
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(column),
                                       renderer,
                                       _fcitx_main_window_enabled_data_func,
                                       self->addonview,
                                       NULL);
    g_signal_connect(G_OBJECT(renderer), "toggled",
                     G_CALLBACK(_fcitx_main_window_toggled_cb), GTK_TREE_MODEL(store));

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->addonview), column);
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(column),
                                       renderer,
                                       _fcitx_main_window_name_data_func,
                                       self->addonview,
                                       NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(self->addonview),
                            GTK_TREE_MODEL(store));

    g_object_unref(store);

    for (addon = (FcitxAddon *) utarray_front(self->addons);
            addon != NULL;
            addon = (FcitxAddon *) utarray_next(self->addons, addon)) {
        GtkTreeIter iter;
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(self->addonview)));
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, LIST_ADDON, addon, -1);
    }

    GtkWidget* hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, TRUE, 0);

    self->button = gtk_button_new_with_label(_("Configure"));
    gtk_widget_set_sensitive(self->button, FALSE);
    gtk_button_set_image(GTK_BUTTON(self->button), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(hbuttonbox), self->button, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(self->button), "clicked", G_CALLBACK(_fcitx_main_window_configure_button_clicked), self);

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->addonview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(_fcitx_main_window_addon_selection_changed), self);

    self->addonpage = _fcitx_main_window_add_page(self, _("Addon Configuration"), vbox);
}

static void
_fcitx_main_window_toggled_cb(GtkCellRenderer *renderer,
                              gchar* str_path,
                              gpointer         user_data)
{
    GtkTreeModel *model = (GtkTreeModel *)user_data;
    GtkTreePath *path = gtk_tree_path_new_from_string(str_path);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    FcitxAddon* addon = NULL;
    gtk_tree_path_free(path);
    gtk_tree_model_get(model,
                       &iter,
                       LIST_ADDON, &addon,
                       -1);

    if (!addon)
        return;

    addon->bEnabled = !addon->bEnabled;
    char *buf;
    asprintf(&buf, "%s.conf", addon->name);
    FILE* fp = FcitxXDGGetFileUserWithPrefix("addon", buf, "w", NULL);
    free(buf);
    if (fp) {
        fprintf(fp, "[Addon]\nEnabled=%s\n", addon->bEnabled ? "True" : "False");
        fclose(fp);
    }
    g_object_set(renderer,
                 "active", (gboolean) addon->bEnabled,
                 NULL);
}

static void
_fcitx_main_window_enabled_data_func(GtkCellLayout   *cell_layout,
                                     GtkCellRenderer *renderer,
                                     GtkTreeModel    *tree_model,
                                     GtkTreeIter     *iter,
                                     gpointer         user_data)
{
    FcitxAddon* addon;

    gtk_tree_model_get(tree_model,
                       iter,
                       LIST_ADDON, &addon,
                       -1);
    g_object_set(renderer,
                 "active", (gboolean) addon->bEnabled,
                 NULL);
}

static void
_fcitx_main_window_name_data_func(GtkCellLayout   *cell_layout,
                                  GtkCellRenderer *renderer,
                                  GtkTreeModel    *tree_model,
                                  GtkTreeIter     *iter,
                                  gpointer         user_data)
{
    FcitxAddon* addon;

    gtk_tree_model_get(tree_model,
                       iter,
                       LIST_ADDON, &addon,
                       -1);
    gchar* string = g_strdup_printf("%s\n%s", addon->generalname, addon->comment);
    g_object_set(renderer,
                 "text", string,
                 NULL);

    g_free(string);
}


void _fcitx_main_window_apply_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxConfigWidget* config_widget = user_data;
    fcitx_config_widget_response(config_widget, CONFIG_WIDGET_SAVE);
}

void _fcitx_main_window_configure_button_clicked(GtkButton* button, gpointer data)
{
    FcitxMainWindow* self = data;
    GtkTreeView* view = GTK_TREE_VIEW(self->addonview);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeIter iter;
    FcitxAddon *addon = NULL;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           LIST_ADDON, &addon,
                           -1);
        gchar* config_desc_name = g_strdup_printf("%s.desc", addon->name);
        FcitxConfigFileDesc* cfdesc = get_config_desc(config_desc_name);
        g_free(config_desc_name);
        gboolean configurable = (gboolean)(cfdesc != NULL || strlen(addon->subconfig) != 0);
        if (configurable) {
            GtkWidget* dialog = gtk_dialog_new_with_buttons(addon->generalname,
                                GTK_WINDOW(self),
                                GTK_DIALOG_MODAL,
                                GTK_STOCK_OK,
                                GTK_RESPONSE_OK,
                                GTK_STOCK_CANCEL,
                                GTK_RESPONSE_CANCEL,
                                NULL
                                                           );

            gchar* config_file_name = g_strdup_printf("%s.config", addon->name);
            FcitxConfigWidget* config_widget = fcitx_config_widget_new(cfdesc, "conf", config_file_name, addon->subconfig);
            GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(config_widget), TRUE, TRUE, 0);
            g_free(config_file_name);
            gtk_widget_set_size_request(GTK_WIDGET(config_widget), -1, 400);

            g_signal_connect(dialog, "response",
                             G_CALLBACK(fcitx_config_widget_response_cb),
                             config_widget);

            gtk_widget_show_all(GTK_WIDGET(dialog));
        }
    }
}
