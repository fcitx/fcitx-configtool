#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>

#include "main_window.h"
#include "config_widget.h"

static GtkWidget *mainWnd = NULL;
static GtkWidget *configTreeView = NULL;
static GtkWidget *configNotebook = NULL;

static int main_window_close(GtkWidget *theWindow, gpointer data);

int main_window_close(GtkWidget *theWindow, gpointer data)
{
    gtk_main_quit();
}

GtkWidget* fcitx_config_main_window_new()
{
    if (mainWnd != NULL)
        return mainWnd;
    GtkWidget *hpaned;

    mainWnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    configTreeView = gtk_tree_view_new();
    ConfigFileDesc *cfdesc = ParseConfigFileDesc("/usr/share/fcitx/data/config.desc");

    configNotebook = config_widget_new(cfdesc);

    gtk_widget_set_size_request(configTreeView, 220, -1);
    gtk_widget_set_size_request(configNotebook, 500, -1);
    gtk_widget_set_size_request(mainWnd, -1, 400);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(configNotebook), TRUE);

    hpaned = gtk_hpaned_new();

    gtk_paned_add1(GTK_PANED(hpaned), configTreeView);
    gtk_paned_add2(GTK_PANED(hpaned), configNotebook);

    gtk_container_add(GTK_CONTAINER(mainWnd), hpaned);

    gtk_signal_connect(GTK_OBJECT(mainWnd), "destroy", GTK_SIGNAL_FUNC(main_window_close), NULL);

    return mainWnd;
}
