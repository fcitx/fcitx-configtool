#include "im_dialog.h"
#include <fcitx-utils/utils.h>
#include "common.h"

G_DEFINE_TYPE(FcitxImDialog, fcitx_im_dialog, GTK_TYPE_DIALOG)

enum {
    AVAIL_TREE_IM_STRING,
    AVAIL_TREE_IM,
    AVAIL_TREE_LANG,
    AVAIL_N_COLUMNS
};

enum {
    IM_LIST_IM_STRING,
    IM_LIST_IM,
    IM_N_COLUMNS
};

static void fcitx_im_dialog_finalize(GObject* object);
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

static void _fcitx_im_dialog_response_cb(GtkDialog *dialog,
                                         gint response,
                                         gpointer user_data);
static void
fcitx_im_dialog_class_init(FcitxImDialogClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = fcitx_im_dialog_finalize;
}

void fcitx_im_dialog_finalize(GObject* object)
{
    FcitxImDialog* self = FCITX_IM_DIALOG(object);
    if (self->array) {
        g_ptr_array_set_free_func(self->array, fcitx_im_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }
    g_object_unref(self->improxy);
}

static void
fcitx_im_dialog_init(FcitxImDialog* self)
{

    self->okbutton = gtk_dialog_add_button(GTK_DIALOG(self),
                            GTK_STOCK_OK,
                            GTK_RESPONSE_OK);

    gtk_dialog_add_button(GTK_DIALOG(self),
                          GTK_STOCK_CANCEL,
                          GTK_RESPONSE_CANCEL
                        );
    GtkCellRenderer* renderer;
    GtkTreeViewColumn* column;

    gtk_window_set_title(GTK_WINDOW(self), _("Add input method"));
    gtk_window_set_modal(GTK_WINDOW(self), TRUE);

    gtk_widget_set_size_request(GTK_WIDGET(self), 400, 300);

    self->availimstore = gtk_list_store_new(IM_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
    self->filtermodel = gtk_tree_model_filter_new(GTK_TREE_MODEL(self->availimstore), NULL);

    gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(self->filtermodel),
                                        (GtkTreeModelFilterVisibleFunc) _fcitx_im_dialog_filter_func,
                                           self ,
                                           NULL);
    self->sortmodel = gtk_tree_model_sort_new_with_model(self->filtermodel);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(self->sortmodel), IM_LIST_IM_STRING, GTK_SORT_ASCENDING);
    self->availimview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->sortmodel));

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
                 _("Input Method"), renderer,
                 "text", IM_LIST_IM_STRING,
                 NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->availimview), column);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self->availimview), FALSE);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->availimview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(_fcitx_im_dialog_im_selection_changed), self);
    GtkWidget* scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolledwindow), self->availimview);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(self))), scrolledwindow, TRUE, TRUE, 0);

    self->onlycurlangcheckbox = gtk_check_button_new_with_label(_("Only Show Current Language"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->onlycurlangcheckbox), TRUE);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(self))), self->onlycurlangcheckbox, FALSE, TRUE, 0);

    self->filterentry = gtk_entry_new();
    gtk_entry_set_icon_from_stock (GTK_ENTRY (self->filterentry),
                                    GTK_ENTRY_ICON_SECONDARY,
                                    GTK_STOCK_CLEAR);
#if GTK_CHECK_VERSION(3,2,0)
    gtk_entry_set_placeholder_text(GTK_ENTRY (self->filterentry), _("Search Input Method"));
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(self))), self->filterentry, FALSE, TRUE, 0);

    g_signal_connect(G_OBJECT(self->filterentry), "changed", G_CALLBACK(_fcitx_im_dialog_filtertext_changed), self);
    g_signal_connect(G_OBJECT(self->onlycurlangcheckbox), "toggled", G_CALLBACK(_fcitx_im_dialog_onlycurlangcheckbox_toggled), self);

    g_signal_connect(self, "response",
                    G_CALLBACK(_fcitx_im_dialog_response_cb),
                    NULL);

    _fcitx_im_dialog_connect(self);
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
        g_ptr_array_set_free_func(self->array, fcitx_im_item_free);
        g_ptr_array_free(self->array, FALSE);
        self->array = NULL;
    }

    self->array = fcitx_input_method_get_imlist(self->improxy);

    if (self->array) {
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
        gtk_list_store_set(self->availimstore, &iter, IM_LIST_IM_STRING, item->name, -1);
        gtk_list_store_set(self->availimstore, &iter, IM_LIST_IM, item, -1);
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
    if (gtk_tree_selection_count_selected_rows(selection))
        gtk_widget_set_sensitive(self->okbutton, TRUE);
    else
        gtk_widget_set_sensitive(self->okbutton, FALSE);
}

GtkWidget* fcitx_im_dialog_new(GtkWindow       *parent)
{
    FcitxImDialog* widget =
        g_object_new(FCITX_TYPE_IM_DIALOG,
                     NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (widget), parent);

    return GTK_WIDGET(widget);
}


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
        flag = flag && (strlen(filter_text) == 0
                 || strstr(item->name, filter_text)
                 || strstr(item->unique_name, filter_text)
                 || strstr(item->langcode, filter_text));
        flag = flag && (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->onlycurlangcheckbox)) ?
                 strncmp(item->langcode, _get_current_lang() , 2) == 0 : TRUE) ;
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
    if (response == GTK_RESPONSE_OK) {
        FcitxImDialog* self = FCITX_IM_DIALOG(dialog);
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