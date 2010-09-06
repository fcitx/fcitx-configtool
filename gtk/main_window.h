#ifndef MAIN_WINDOW_H

#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>

typedef struct ConfigPage
{
    ConfigFileDesc *cfdesc;
    GtkWidget* page;
    struct ConfigPage* parent;
    GenericConfig config;
    char *filename;
    GtkTreeIter iter;
} ConfigPage;

extern GtkWidget* fcitx_config_main_window_new();

#endif
