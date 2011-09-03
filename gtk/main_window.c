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

#include <stdlib.h>
#include <libintl.h>
#include <errno.h>

#include <fcitx/addon.h>
#include <fcitx-utils/utarray.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

#include "config.h"
#include "main_window.h"
#include "menu.h"
#include "config_widget.h"
#include "configdesc.h"

enum
{
    LIST_ADDON,
    N_COLUMNS
};

#define _(s) gettext(s)


static void
enabled_data_func (GtkCellLayout   *cell_layout,
                  GtkCellRenderer *renderer,
                  GtkTreeModel    *tree_model,
                  GtkTreeIter     *iter,
                  gpointer         user_data);

static void
name_data_func (GtkCellLayout   *cell_layout,
                GtkCellRenderer *renderer,
                GtkTreeModel    *tree_model,
                GtkTreeIter     *iter,
                gpointer         user_data);

static void configure_button_clicked (GtkButton *button,
                                      gpointer   user_data);
static void apply_button_clicked (GtkButton *button,
                                      gpointer   user_data);
static void
toggled_cb (GtkCellRenderer *renderer,
            gchar* str_path,
            gpointer         user_data);

static GtkWidget *mainWnd = NULL;
static GtkWidget *configTreeView = NULL;
static GtkWidget *configNotebook = NULL;
static GtkTreeStore *store = NULL;
static GtkWidget *hpaned = NULL;
static ConfigPage *configPage, *lastPage = NULL, *addonPage;
static GtkWidget* configureButton = NULL;
const UT_icd addonicd= {sizeof ( FcitxAddon ), 0, 0, FreeAddon};
UT_array* addonBuf;

static int main_window_close(GtkWidget *theWindow, gpointer data);
static GtkTreeModel *fcitx_config_create_model();

int main_window_close(GtkWidget *theWindow, gpointer data)
{
    utarray_free(addonBuf);
    gtk_main_quit();
}

ConfigPage* main_window_add_page(const char* name, GtkWidget* widget)
{
    ConfigPage *page = (ConfigPage*) malloc(sizeof(ConfigPage));
    memset(page, 0, sizeof(ConfigPage));

    page->page = widget;

    g_object_ref(page->page);

    gtk_widget_show_all(widget);

    gtk_tree_store_append(store, &page->iter, NULL);
    gtk_tree_store_set(store, &page->iter, 0, name, 1, page, -1);

    return page;
}

gboolean selection_changed(GtkTreeSelection *selection, gpointer data) {
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;
    ConfigPage* page;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter,
                1, &page,
                -1);

        if (lastPage)
            gtk_container_remove(GTK_CONTAINER(hpaned), lastPage->page);
        gtk_paned_add2(GTK_PANED(hpaned), page->page);
        gtk_widget_show_all(mainWnd);

        lastPage = page;
    }
    else
    {
        gtk_tree_selection_select_iter(selection, &configPage->iter);
    }
}


gboolean addon_selection_changed(GtkTreeSelection *selection, gpointer data) {
    GtkTreeView *treeView = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;
    FcitxAddon *addon = NULL;

    if (!configureButton)
        return;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter,
                LIST_ADDON, &addon,
                -1);
        gchar* config_desc_name = g_strdup_printf("%s.desc",addon->name);
        ConfigFileDesc* cfdesc = get_config_desc ( config_desc_name );
        g_free(config_desc_name);
        gboolean configurable = ( gboolean ) ( cfdesc != NULL || strlen ( addon->subconfig ) != 0 );
        gtk_widget_set_sensitive(configureButton, configurable);
    }
    else
    {
        gtk_widget_set_sensitive(configureButton, FALSE);
    }
}

GtkTreeModel *fcitx_config_create_model()
{
    store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
    return GTK_TREE_MODEL(store);
}

void add_config_file_page()
{
    char *file;
    ConfigFileDesc* configDesc = get_config_desc("config.desc");

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
    g_signal_connect(G_OBJECT(applybutton), "clicked", G_CALLBACK(apply_button_clicked), config_widget);


    configPage = main_window_add_page(_("Global Config"), vbox);
}

void add_addon_page()
{
    int i, j;
    FcitxAddon* addon;
    utarray_new ( addonBuf, &addonicd );
    LoadAddonInfo(addonBuf);

    GtkWidget* vbox = gtk_vbox_new(FALSE, 0);

    GtkWidget* swin = gtk_scrolled_window_new ( NULL, NULL );
    gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);
    g_object_set(swin, "hscrollbar-policy", GTK_POLICY_NEVER, NULL);
    GtkWidget* list = gtk_tree_view_new();
    g_object_set(list, "headers-visible", FALSE, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), list);
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    store = gtk_list_store_new(N_COLUMNS, G_TYPE_POINTER);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Enable", renderer, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (column),
                                        renderer,
                                        enabled_data_func,
                                        list,
                                        NULL);
    g_signal_connect(G_OBJECT(renderer), "toggled",
            G_CALLBACK(toggled_cb), GTK_TREE_MODEL(store));

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (column),
                                        renderer,
                                        name_data_func,
                                        list,
                                        NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(list),
                            GTK_TREE_MODEL(store));

    g_object_unref(store);

    for ( addon = ( FcitxAddon * ) utarray_front ( addonBuf );
        addon != NULL;
        addon = ( FcitxAddon * ) utarray_next ( addonBuf, addon ) )
    {
        GtkTreeIter iter;
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, LIST_ADDON, addon, -1);
    }

    GtkWidget* hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, TRUE, 0);

    configureButton = gtk_button_new_with_label(_("Configure"));
    gtk_button_set_image(GTK_BUTTON(configureButton), gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(hbuttonbox), configureButton, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(configureButton), "clicked", G_CALLBACK(configure_button_clicked), list);

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
            G_CALLBACK(addon_selection_changed), NULL);

    addonPage = main_window_add_page(_("Addon Configuration"), vbox);
}

GtkWidget* fcitx_config_main_window_new()
{
    if (mainWnd != NULL)
        return mainWnd;

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    mainWnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    configTreeView = gtk_tree_view_new_with_model(fcitx_config_create_model());

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
            _("Config"), renderer,
            "text", 0,
            NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (configTreeView), column);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(configTreeView), FALSE);

    add_config_file_page();
    add_addon_page();

    gtk_widget_set_size_request(configTreeView, 170, -1);
    gtk_widget_set_size_request(mainWnd, -1, 500);

    hpaned = gtk_hpaned_new();
    GtkWidget *treescroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(treescroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(treescroll), configTreeView);
    gtk_paned_add1(GTK_PANED(hpaned), treescroll);

    gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(mainWnd), vbox);

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(configTreeView));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    gtk_signal_connect(GTK_OBJECT(mainWnd), "destroy", GTK_SIGNAL_FUNC(main_window_close), NULL);
    g_signal_connect(G_OBJECT(selection), "changed",
            G_CALLBACK(selection_changed), NULL);

    gtk_tree_view_expand_all(GTK_TREE_VIEW(configTreeView));
    gtk_tree_selection_select_iter(selection, &configPage->iter);

    gtk_window_set_icon_name(GTK_WINDOW(mainWnd), "fcitx-configtool");
    gtk_window_set_title(GTK_WINDOW(mainWnd), _("Fcitx Config"));
    return mainWnd;
}

static void
toggled_cb (GtkCellRenderer *renderer,
            gchar* str_path,
            gpointer         user_data)
{
    GtkTreeModel *model = (GtkTreeModel *)user_data;
    GtkTreePath *path= gtk_tree_path_new_from_string(str_path);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    FcitxAddon* addon = NULL;
    gboolean state;
    gtk_tree_path_free(path);
    gtk_tree_model_get (model,
                        &iter,
                        LIST_ADDON, &addon,
                        -1);

    if (!addon)
        return;

    addon->bEnabled = !addon->bEnabled;
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s.conf", addon->name);
    buf[PATH_MAX - 1] = 0;
    FILE* fp = GetXDGFileUserWithPrefix("addon", buf, "w", NULL);
    if (fp)
    {
        fprintf(fp, "[Addon]\nEnabled=%s\n", addon->bEnabled ? "True": "False");
        fclose(fp);
    }
    g_object_set (renderer,
                "active", (gboolean) addon->bEnabled,
                NULL);
}

static void
enabled_data_func (GtkCellLayout   *cell_layout,
                  GtkCellRenderer *renderer,
                  GtkTreeModel    *tree_model,
                  GtkTreeIter     *iter,
                  gpointer         user_data)
{
    FcitxAddon* addon;

    gtk_tree_model_get (tree_model,
                        iter,
                        LIST_ADDON, &addon,
                        -1);
    g_object_set (renderer,
                "active", (gboolean) addon->bEnabled,
                NULL);
}

static void
name_data_func (GtkCellLayout   *cell_layout,
                GtkCellRenderer *renderer,
                GtkTreeModel    *tree_model,
                GtkTreeIter     *iter,
                gpointer         user_data)
{
    FcitxAddon* addon;

    gtk_tree_model_get (tree_model,
                        iter,
                        LIST_ADDON, &addon,
                        -1);
    gchar* string = g_strdup_printf("%s\n%s", addon->generalname, addon->comment);
    g_object_set (renderer,
                "text", string,
                NULL);

    g_free(string);
}


void apply_button_clicked(GtkButton* button, gpointer user_data)
{
    FcitxConfigWidget* config_widget = user_data;
    fcitx_config_widget_response(config_widget, CONFIG_WIDGET_SAVE);
}

void configure_button_clicked (GtkButton *button,
                               gpointer   user_data)
{
    GtkTreeView* view = user_data;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeIter iter;
    FcitxAddon *addon = NULL;
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter,
                LIST_ADDON, &addon,
                -1);
        gchar* config_desc_name = g_strdup_printf("%s.desc",addon->name);
        ConfigFileDesc* cfdesc = get_config_desc ( config_desc_name );
        g_free(config_desc_name);
        gboolean configurable = ( gboolean ) ( cfdesc != NULL || strlen ( addon->subconfig ) != 0 );
        if (configurable)
        {
            GtkWidget* dialog = gtk_dialog_new_with_buttons(addon->generalname,
                                                            GTK_WINDOW(mainWnd),
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

            g_signal_connect (dialog, "response",
                              G_CALLBACK (response_cb),
                              config_widget);

            gtk_widget_show_all (GTK_WIDGET(dialog));
        }
    }
}

gboolean response_cb (GtkDialog *dialog,
                    gint response,
                    gpointer user_data)
{
    if (response == GTK_RESPONSE_OK)
    {
        FcitxConfigWidget* config_widget = (FcitxConfigWidget*) user_data;
        fcitx_config_widget_response(config_widget, CONFIG_WIDGET_SAVE);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return FALSE;
}
