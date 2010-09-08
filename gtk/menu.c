#include <gtk/gtk.h>
#include <libintl.h>

#define _(x) gettext(x)

GtkWidget* fcitx_config_menu_new()
{
    GtkWidget *menu = gtk_menu_bar_new();
    
    GtkWidget *menuItemFile = gtk_menu_item_new_with_mnemonic(_("_File"));
    GtkWidget *menuFile = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItemFile), menuFile);
    gtk_menu_bar_append(menu,menuItemFile);

    GtkWidget *menuitem, *image;

    menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Exit"));
    image = gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_BUTTON);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
    gtk_menu_append(menuFile, menuitem);

    gtk_signal_connect(GTK_OBJECT(menuitem), "activate", G_CALLBACK(gtk_main_quit), NULL);
    return menu;
}
