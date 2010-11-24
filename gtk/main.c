#include <gtk/gtk.h>
#include <langinfo.h>
#include <libintl.h>
#include <locale.h>

#include "main_window.h"

int
main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain("fcitx-configtool", LOCALEDIR); 
    bindtextdomain("fcitx", LOCALEDIR); 
    textdomain("fcitx-configtool");

    GtkWidget *window;
    gtk_init(&argc, &argv);

    window = fcitx_config_main_window_new (GTK_WINDOW_TOPLEVEL);
    
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
 
