#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>

#include "main_window.h"

extern GtkWidget* config_widget_new(ConfigFileDesc *cfdesc, ConfigFile *cfile, ConfigPage *page, gboolean readonly);
