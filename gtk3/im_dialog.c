#include <fcitx-utils/utils.h>

#include "common.h"
#include "im_dialog.h"
#include "gdm-languages.h"

G_DEFINE_TYPE(FcitxImDialog, fcitx_im_dialog, GTK_TYPE_DIALOG)

enum {
    IM_LIST_IM_STRING,
    IM_LIST_IM,
    IM_LIST_IM_LANGUAGE,
    IM_N_COLUMNS
};

static void fcitx_im_dialog_dispose(GObject* object);
static void _fcitx_im_dialog_connect(FcitxImDialog* self);
static void _fcitx_im_dialog_load(FcitxImDialog* self);
static void _fcitx_inputmethod_insert_foreach_cb(gpointer data, gpointer user_data);
static void _fcitx_im_dialog_im_selection_changed(GtkTreeSelection *selection, gpointer data);
static void _fcitx_im_dialog_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data);
static void _fcitx_im_dialog_filtertext_changed(GtkEditable* editable, gpointer user_data);
static void _fcitx_im_dialog_onlycurlangcheckbox_toggled(GtkToggleButton* button, gpointer user_data);

static gboolean _fcitx_im_dialog_filter_func(GtkTreeModel *model,
                                             GtkTreeIter  *iter,
                                             gpointer      data);
static gint     _fcitx_im_dialog_sort_func(GtkTreeModel *model,
                                           GtkTreeIter  *a,
                                           GtkTreeIter  *b,
                                           gpointer      user_data);

static void _fcitx_im_dialog_response_cb(GtkDialog *dialog,
                                         gint response,
                                         gpointer user_data);
static GObject *
fcitx_im_dialog_constructor   (GType                  gtype,
                               guint                  n_properties,
                               GObjectConstructParam *properties);

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
fcitx_im_dialog_class_init(FcitxImDialogClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->dispose = fcitx_im_dialog_dispose;
    gobject_class->constructor = fcitx_im_dialog_constructor;
}

static GObject *
fcitx_im_dialog_constructor   (GType                  gtype,
                               guint                  n_properties,
                               GObjectConstructParam *properties)
{
    GObject *obj;
    FcitxImDialog *self;
    GtkWidget *widget;

    obj = G_OBJECT_CLASS (fcitx_im_dialog_parent_class)->constructor (gtype, n_properties, properties);

    self = FCITX_IM_DIALOG (obj);

    widget = GTK_WIDGET(gtk_builder_get_object (self->builder,
                                                "im_dialog"));

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(self));
    gtk_container_add (GTK_CONTAINER (content_area), widget);

    _fcitx_im_dialog_connect(self);

  return obj;
}

void fcitx_im_dialog_dispose(GObject* object)
{
    FcitxImDialog* self = FCITX_IM_DIALOG(object);
    if (self->array) {
        g_ptr_array_set_free_func(self->array, (GDestroyNotify) fcitx_im_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }

    if (self->improxy) {
        g_signal_handlers_disconnect_by_func(self->improxy, G_CALLBACK(_fcitx_im_dialog_imlist_changed_cb), self);
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

    G_OBJECT_CLASS (fcitx_im_dialog_parent_class)->dispose (object);
}

static void
fcitx_im_dialog_init(FcitxImDialog* self)
{
    gtk_window_set_title(GTK_WINDOW(self), _("Add input method"));
    gtk_window_set_modal(GTK_WINDOW(self), TRUE);

    gtk_dialog_add_buttons(GTK_DIALOG(self),
                           _("_Cancel"),
                           GTK_RESPONSE_CANCEL,
                           _("_OK"),
                           GTK_RESPONSE_OK,
                           NULL
                          );

    g_signal_connect(self, "response",
                    G_CALLBACK(_fcitx_im_dialog_response_cb),
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
                                        (GtkTreeModelFilterVisibleFunc) _fcitx_im_dialog_filter_func,
                                           self ,
                                           NULL);
    gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(self->sortmodel), IM_LIST_IM, _fcitx_im_dialog_sort_func, self, NULL);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(self->sortmodel), IM_LIST_IM, GTK_SORT_ASCENDING);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self->availimview), FALSE);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(_fcitx_im_dialog_im_selection_changed), self);

    g_signal_connect(G_OBJECT(self->filterentry), "changed", G_CALLBACK(_fcitx_im_dialog_filtertext_changed), self);
    g_signal_connect(G_OBJECT(self->onlycurlangcheckbox), "toggled", G_CALLBACK(_fcitx_im_dialog_onlycurlangcheckbox_toggled), self);
    g_signal_connect(G_OBJECT(self->filterentry), "icon-press", G_CALLBACK (icon_press_cb), NULL);
}


void _fcitx_im_dialog_filtertext_changed(GtkEditable* editable, gpointer user_data)
{
    FcitxImDialog* self = user_data;
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(self->filtermodel));
}

void _fcitx_im_dialog_onlycurlangcheckbox_toggled(GtkToggleButton* button, gpointer user_data)
{
    FcitxImDialog* self = user_data;
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(self->filtermodel));
}

void _fcitx_im_dialog_connect(FcitxImDialog* self)
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
    g_signal_connect(self->improxy, "imlist-changed", G_CALLBACK(_fcitx_im_dialog_imlist_changed_cb), self);

    _fcitx_im_dialog_load(self);
}

void _fcitx_im_dialog_load(FcitxImDialog* self)
{
    gtk_list_store_clear(self->availimstore);

    if (self->array) {
        g_ptr_array_set_free_func(self->array, (GDestroyNotify) fcitx_im_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }

    self->array = fcitx_input_method_get_imlist(self->improxy);
    g_hash_table_remove_all(self->langset);

    if (self->array) {
        g_ptr_array_set_free_func(self->array, NULL);
        g_ptr_array_foreach(self->array, _fcitx_inputmethod_insert_foreach_cb, self);

        _fcitx_im_dialog_im_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview)), self);
    }
}

void _fcitx_inputmethod_insert_foreach_cb(gpointer data,
        gpointer user_data)
{
    FcitxIMItem* item = data;
    FcitxImDialog* self = user_data;
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

void _fcitx_im_dialog_imlist_changed_cb(FcitxInputMethod* im, gpointer user_data)
{
    FcitxImDialog* self = user_data;
    _fcitx_im_dialog_load(self);
}


void _fcitx_im_dialog_im_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    FcitxImDialog* self = data;
    GtkWidget* button = gtk_dialog_get_widget_for_response(GTK_DIALOG(self), GTK_RESPONSE_OK);
    if (!button)
        return;
    if (gtk_tree_selection_count_selected_rows(selection))
        gtk_widget_set_sensitive(button, TRUE);
    else
        gtk_widget_set_sensitive(button, FALSE);
}

GtkWidget* fcitx_im_dialog_new(GtkWindow       *parent)
{
    FcitxImDialog* self =
        g_object_new(FCITX_TYPE_IM_DIALOG,
                     NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (self), parent);

    return GTK_WIDGET(self);
}

gint _cmp_im_item(FcitxImDialog* self, FcitxIMItem* itema, FcitxIMItem* itemb)
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

gint _fcitx_im_dialog_sort_func(GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
    FcitxImDialog* self = user_data;
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


gboolean _fcitx_im_dialog_filter_func(GtkTreeModel *model,
                                      GtkTreeIter  *iter,
                                      gpointer      data)
{
    FcitxImDialog* self = data;
    const gchar* filter_text = gtk_entry_get_text(GTK_ENTRY(self->filterentry));
    FcitxIMItem* item = NULL;
    gtk_tree_model_get(GTK_TREE_MODEL(self->availimstore),
                       iter,
                       IM_LIST_IM, &item,
                       -1);

    gboolean flag = TRUE;
    if (item) {
        if (strcmp(item->unique_name, "fcitx-keyboard-us") != 0) {
            const char* language = gdm_get_language_from_name(item->langcode, NULL);
            flag = flag && (
                        strlen(filter_text) == 0
                     || strstr(item->name, filter_text)
                     || strstr(item->unique_name, filter_text)
                     || strstr(item->langcode, filter_text)
                     || (language && strstr(language, filter_text)));

            gchar temp[3] = {0, 0, 0};
            strncpy(temp, item->langcode, 2);
            flag = flag && (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->onlycurlangcheckbox)) ?
                     strncmp(item->langcode, self->language , 2) == 0 || g_hash_table_contains(self->langset, temp) : TRUE) ;
        }
    }
    return flag;
}

typedef struct {
    FcitxImDialog* self;
    gboolean changed;
} add_foreach_context;

void add_foreach (GtkTreeModel      *model,
                  GtkTreePath       *path,
                  GtkTreeIter       *iter,
                  gpointer           data)
{
    FcitxIMItem* item = NULL;
    add_foreach_context* context = data;
    FcitxImDialog* self = context->self;
    gtk_tree_model_get(model,
                       iter,
                       IM_LIST_IM, &item,
                       -1);
    if (item == NULL)
        return;
    item->enable = TRUE;
    context->changed = TRUE;

    g_ptr_array_remove(self->array, item);
    g_ptr_array_add(self->array, item);
}

void _fcitx_im_dialog_response_cb(GtkDialog *dialog,
                                  gint response,
                                  gpointer user_data)
{
    FcitxImDialog* self = FCITX_IM_DIALOG(dialog);
    if (response == GTK_RESPONSE_OK) {
        GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
        add_foreach_context context;
        context.self = self;
        context.changed = FALSE;

        gtk_tree_selection_selected_foreach(selection, add_foreach, &context);
        if (context.changed)
            fcitx_input_method_set_imlist(self->improxy, self->array);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}


static void
icon_press_cb (GtkEntry       *entry,
               gint            position,
               GdkEventButton *event,
               gpointer        data)
{
    gtk_entry_set_text (entry, "");
}
