/***************************************************************************
 *   Copyright (C) 2013~2014 by Lenky0401                                  *
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

#include "common.h"
#include "wizard_im_dialog.h"
#include "gdm-languages.h"

G_DEFINE_TYPE(FcitxWizardImDialog, fcitx_wizard_im_dialog, GTK_TYPE_DIALOG)

enum {
    IM_LIST_IM_STRING,
    IM_LIST_IM,
    IM_LIST_IM_LANGUAGE,
    IM_N_COLUMNS
};

enum {
    PROP_0,

    PROP_OWNER
};


static void fcitx_wizard_im_dialog_dispose(GObject* object);
static void _fcitx_wizard_im_dialog_connect(FcitxWizardImDialog* self);
static void _fcitx_wizard_im_dialog_load(FcitxWizardImDialog* self);
static void _fcitx_inputmethod_insert_foreach_cb(gpointer data, gpointer user_data);
static void _fcitx_wizard_im_dialog_im_selection_changed(GtkTreeSelection *selection, gpointer data);
static void _fcitx_wizard_im_dialog_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data);
static void _fcitx_wizard_im_dialog_filtertext_changed(GtkEditable* editable, gpointer user_data);
static void _fcitx_wizard_im_dialog_onlycurlangcheckbox_toggled(GtkToggleButton* button, gpointer user_data);

static gboolean _fcitx_wizard_im_dialog_filter_func(GtkTreeModel *model,
                                             GtkTreeIter  *iter,
                                             gpointer      data);
static gint     _fcitx_wizard_im_dialog_sort_func(GtkTreeModel *model,
                                           GtkTreeIter  *a,
                                           GtkTreeIter  *b,
                                           gpointer      user_data);

static void _fcitx_wizard_im_dialog_response_cb(GtkDialog *dialog,
                                         gint response,
                                         gpointer user_data);
static GObject *
fcitx_wizard_im_dialog_constructor   (GType                  gtype,
                               guint                  n_properties,
                               GObjectConstructParam *properties);

static void
fcitx_wizard_im_dialog_set_property(GObject *gobject,
    guint prop_id, const GValue *value, GParamSpec *pspec);

static void
icon_press_cb (GtkEntry       *entry,
               gint            position,
               GdkEventButton *event,
               gpointer        data);



static const gchar* _get_current_lang()
{
    const gchar* lang =  g_getenv("LC_ALL");
    if (!lang)
        lang = g_getenv("LANG");
    if (!lang)
        lang = g_getenv("LC_MESSAGES");
    if (!lang)
        lang = "C";
    return lang;
}

static void
fcitx_wizard_im_dialog_set_property(GObject *gobject,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{

    FcitxWizardImDialog* im_dialog = FCITX_WIZARD_IM_DIALOG(gobject);
    
    switch (prop_id) {
    case PROP_OWNER:
        im_dialog->owner = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}

static void
fcitx_wizard_im_dialog_class_init(FcitxWizardImDialogClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->set_property = fcitx_wizard_im_dialog_set_property;
    gobject_class->dispose = fcitx_wizard_im_dialog_dispose;
    gobject_class->constructor = fcitx_wizard_im_dialog_constructor;

    g_object_class_install_property(gobject_class, PROP_OWNER,
        g_param_spec_pointer("owner", "OWNER",
        "The Owner of this Dialog", 
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
    
}

static GObject *
fcitx_wizard_im_dialog_constructor   (GType                  gtype,
                               guint                  n_properties,
                               GObjectConstructParam *properties)
{
    GObject *obj;
    FcitxWizardImDialog *self;
    GtkWidget *widget;

    obj = G_OBJECT_CLASS (fcitx_wizard_im_dialog_parent_class)->constructor (gtype, n_properties, properties);

    self = FCITX_WIZARD_IM_DIALOG (obj);

    widget = GTK_WIDGET(gtk_builder_get_object (self->builder,
                                                "im_dialog"));

    gtk_widget_reparent (widget, gtk_dialog_get_content_area(GTK_DIALOG(self)));

    _fcitx_wizard_im_dialog_connect(self);

  return obj;
}

void fcitx_wizard_im_dialog_dispose(GObject* object)
{
    FcitxWizardImDialog* self = FCITX_WIZARD_IM_DIALOG(object);

    if (self->improxy) {
        g_signal_handlers_disconnect_by_func(self->improxy, G_CALLBACK(_fcitx_wizard_im_dialog_imlist_changed_cb), self);
        g_object_unref(self->improxy);
        self->improxy = NULL;
    }

    if (self->langset) {
        g_hash_table_destroy(self->langset);
        self->langset = NULL;
    }

    if (self->language) {
        g_free(self->language);
        self->language = NULL;
    }

    G_OBJECT_CLASS (fcitx_wizard_im_dialog_parent_class)->dispose (object);
}

static void
fcitx_wizard_im_dialog_init(FcitxWizardImDialog* self)
{
    gtk_window_set_title(GTK_WINDOW(self), _("Add input method"));
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
    g_signal_connect(self, "response",
                    G_CALLBACK(_fcitx_wizard_im_dialog_response_cb),
                    NULL);

    self->langset = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    self->builder = gtk_builder_new();
    gtk_builder_add_from_resource(self->builder, "/org/fcitx/fcitx-config-gtk3/im_dialog.ui", NULL);

    const gchar* lang = _get_current_lang();
    gchar* language = NULL, *territory = NULL;
    gdm_parse_language_name(lang, &language, &territory, NULL, NULL);
    if (!language || language[0] == '\0') {
        self->language = g_strdup("C");
    } else {
        gboolean tisempty = (!territory || territory[0] == '\0');
        self->language = g_strdup_printf("%s%s%s", language, tisempty ? "" : "_",  tisempty ? "" : territory);
    }
    g_free(language);
    g_free(territory);

#define _GET_OBJECT(NAME) \
    self->NAME = (typeof(self->NAME)) gtk_builder_get_object(self->builder, #NAME);

    _GET_OBJECT(availimstore)
    _GET_OBJECT(availimview)
    _GET_OBJECT(filterentry)
    _GET_OBJECT(filtermodel)
    _GET_OBJECT(onlycurlangcheckbox)
    _GET_OBJECT(sortmodel)

    gtk_entry_set_placeholder_text(GTK_ENTRY(self->filterentry), _("Search Input Method"));

    gtk_widget_set_size_request(GTK_WIDGET(self), 400, 300);

    gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(self->filtermodel),
                                        (GtkTreeModelFilterVisibleFunc) _fcitx_wizard_im_dialog_filter_func,
                                           self ,
                                           NULL);
    gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(self->sortmodel), IM_LIST_IM, _fcitx_wizard_im_dialog_sort_func, self, NULL);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(self->sortmodel), IM_LIST_IM, GTK_SORT_ASCENDING);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self->availimview), FALSE);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(_fcitx_wizard_im_dialog_im_selection_changed), self);

    g_signal_connect(G_OBJECT(self->filterentry), "changed", G_CALLBACK(_fcitx_wizard_im_dialog_filtertext_changed), self);
    g_signal_connect(G_OBJECT(self->onlycurlangcheckbox), "toggled", G_CALLBACK(_fcitx_wizard_im_dialog_onlycurlangcheckbox_toggled), self);
    g_signal_connect(G_OBJECT(self->filterentry), "icon-press", G_CALLBACK (icon_press_cb), NULL);
}


void _fcitx_wizard_im_dialog_filtertext_changed(GtkEditable* editable, gpointer user_data)
{
    FcitxWizardImDialog* self = user_data;
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(self->filtermodel));
}

void _fcitx_wizard_im_dialog_onlycurlangcheckbox_toggled(GtkToggleButton* button, gpointer user_data)
{
    FcitxWizardImDialog* self = user_data;
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(self->filtermodel));
}

void _fcitx_wizard_im_dialog_connect(FcitxWizardImDialog* self)
{
    int i;
    GError* error = NULL;
    if (self->owner->im_dialog_array == NULL) {
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
        self->owner->im_dialog_array = fcitx_input_method_get_imlist(self->improxy);
        
        if (self->owner->im_dialog_array_del != NULL) {
            for (i = 0; i < self->owner->im_dialog_array_del->len; i += 1) {
                g_ptr_array_add(self->owner->im_dialog_array, 
                    g_ptr_array_index(self->owner->im_dialog_array_del, i));
            }
        }
    }
    
    _fcitx_wizard_im_dialog_load(self);
}

void _fcitx_wizard_im_dialog_load(FcitxWizardImDialog* self)
{
    gtk_list_store_clear(self->availimstore);

    g_hash_table_remove_all(self->langset);

    if (self->owner->im_dialog_array == NULL) {
        return;
    }

    g_ptr_array_set_free_func(self->owner->im_dialog_array, NULL);
    g_ptr_array_foreach(self->owner->im_dialog_array, 
        _fcitx_inputmethod_insert_foreach_cb, self);

    _fcitx_wizard_im_dialog_im_selection_changed(
        gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview)), self);

}

void _fcitx_inputmethod_insert_foreach_cb(gpointer data,
        gpointer user_data)
{
    FcitxIMItem* item = data;
    FcitxWizardImDialog* self = user_data;
    GtkTreeIter iter;

    if (!item->enable) {
        gtk_list_store_append(self->availimstore, &iter);
        char* lang = NULL;
        if (strlen(item->langcode) != 0)
            lang = gdm_get_language_from_name(item->langcode, NULL);
        if (!lang) {
            if (strcmp(item->langcode, "*") == 0)
                lang = g_strdup_printf("%s", _("Unknown"));
            else
                lang = g_strdup_printf("%s", _("Unknown"));
        }
        gtk_list_store_set(self->availimstore, &iter, IM_LIST_IM_STRING, item->name, IM_LIST_IM, item, IM_LIST_IM_LANGUAGE, lang, -1);
    } else {
        gchar temp[3] = {0, 0, 0};
        strncpy(temp, item->langcode, 2);
        if (!g_hash_table_contains(self->langset, temp)) {
            g_hash_table_insert(self->langset, g_strdup(temp), NULL);
        }
    }
}

void _fcitx_wizard_im_dialog_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data)
{
    FcitxWizardImDialog* self = user_data;
    _fcitx_wizard_im_dialog_load(self);
}


void _fcitx_wizard_im_dialog_im_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    FcitxWizardImDialog* self = data;
    GtkWidget* button = gtk_dialog_get_widget_for_response(GTK_DIALOG(self), GTK_RESPONSE_OK);
    if (!button)
        return;
    if (gtk_tree_selection_count_selected_rows(selection))
        gtk_widget_set_sensitive(button, TRUE);
    else
        gtk_widget_set_sensitive(button, FALSE);
}

GtkWidget* fcitx_wizard_im_dialog_new(GtkWindow *parent, FcitxWizardImWidget* owner)
{
    FcitxWizardImDialog* self =
        g_object_new(FCITX_TYPE_WIZARD_IM_DIALOG,
            "owner", owner, NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (self), parent);

    return GTK_WIDGET(self);
}

gint _cmp_im_item(FcitxWizardImDialog* self, FcitxIMItem* itema, FcitxIMItem* itemb)
{

    int ret = g_strcmp0(itema->langcode, itemb->langcode);
    if (ret == 0) {
        return g_strcmp0(itema->name, itemb->name);
    }

    if (g_strcmp0(itema->langcode, self->language) == 0) {
        return -1;
    }
    if (g_strcmp0(itemb->langcode, self->language) == 0) {
        return 1;
    }

    gchar tempa[3] = {0, 0, 0};
    strncpy(tempa, itema->langcode, 2);
    gchar tempb[3] = {0, 0, 0};
    strncpy(tempb, itemb->langcode, 2);
    gboolean fa = strncmp(tempa, self->language, 2) == 0;
    gboolean fb = strncmp(tempb, self->language, 2) == 0;
    if (fa == fb) {
        return ret;
    }
    return fa ? -1 : 1;
}

gint _fcitx_wizard_im_dialog_sort_func(GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
    FcitxWizardImDialog* self = user_data;
    FcitxIMItem* itema = NULL, *itemb = NULL;
    gtk_tree_model_get(model,
                       a,
                       IM_LIST_IM, &itema,
                       -1);
    gtk_tree_model_get(model,
                       b,
                       IM_LIST_IM, &itemb,
                       -1);

    if (itema == NULL || itemb == NULL)
        return 0;

    int ret = _cmp_im_item(self, itema, itemb);
    return ret;
}


gboolean _fcitx_wizard_im_dialog_filter_func(GtkTreeModel *model,
                                      GtkTreeIter  *iter,
                                      gpointer      data)
{
    FcitxWizardImDialog* self = data;
    const gchar* filter_text = gtk_entry_get_text(GTK_ENTRY(self->filterentry));
    FcitxIMItem* item = NULL;
    gtk_tree_model_get(GTK_TREE_MODEL(self->availimstore),
                       iter,
                       IM_LIST_IM, &item,
                       -1);

    gboolean flag = TRUE;
    if (item) {
        if (strcmp(item->unique_name, "fcitx-keyboard-us") != 0) {
            flag = flag && (
                        strlen(filter_text) == 0
                     || strstr(item->name, filter_text)
                     || strstr(item->unique_name, filter_text)
                     || strstr(item->langcode, filter_text));

            gchar temp[3] = {0, 0, 0};
            strncpy(temp, item->langcode, 2);
            flag = flag && (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->onlycurlangcheckbox)) ?
                     strncmp(item->langcode, self->language , 2) == 0 || g_hash_table_contains(self->langset, temp) : TRUE) ;
        }
    }
    return flag;
}

typedef struct {
    FcitxWizardImDialog* self;
    gboolean changed;
} add_foreach_context;

void add_foreach (GtkTreeModel      *model,
                  GtkTreePath       *path,
                  GtkTreeIter       *iter,
                  gpointer           data)
{
    FcitxIMItem* item = NULL;
    add_foreach_context* context = data;
    FcitxWizardImDialog* self = context->self;
    gtk_tree_model_get(model,
                       iter,
                       IM_LIST_IM, &item,
                       -1);
    if (item == NULL)
        return;
    item->enable = TRUE;
    context->changed = TRUE;

    g_ptr_array_remove(self->owner->im_dialog_array, item);
    g_ptr_array_add(self->owner->array, item);
}

void _fcitx_wizard_im_dialog_response_cb(GtkDialog *dialog,
                                  gint response,
                                  gpointer user_data)
{
    FcitxWizardImDialog* self = FCITX_WIZARD_IM_DIALOG(dialog);
    if (response == GTK_RESPONSE_OK) {
        GtkTreeSelection* selection = 
            gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
        add_foreach_context context;
        context.self = self;
        context.changed = FALSE;

        gtk_tree_selection_selected_foreach(selection, add_foreach, &context);
        if (context.changed) {
            _fcitx_wizard_im_widget_refresh_view(self->owner);
        }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

GtkTreeSelection*
_fcitx_wizard_wizard_im_dialog_get_selection(GtkDialog *dialog,
    gint response)
{
    GtkTreeSelection* selection = NULL;
    FcitxWizardImDialog* self = FCITX_WIZARD_IM_DIALOG(dialog);
    
    if (response == GTK_RESPONSE_OK) {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));

    return selection;
}


static void
icon_press_cb (GtkEntry       *entry,
               gint            position,
               GdkEventButton *event,
               gpointer        data)
{
    gtk_entry_set_text (entry, "");
}
