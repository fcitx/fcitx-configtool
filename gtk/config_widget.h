#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>

#include "main_window.h"

extern GtkWidget* config_widget_new(ConfigFileDesc *cfdesc, ConfigFile *cfile, ConfigPage *page, gboolean readonly);

#define CHANGE_DOMAIN_BEGIN(domain) { \
    char *lastdomain = strdup(textdomain(NULL)); \
    textdomain(domain);

#define CHANGE_DOMAIN_END() \
    textdomain(lastdomain); \
    free(lastdomain); \
}
