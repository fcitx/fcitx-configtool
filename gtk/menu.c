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

#include <gtk/gtk.h>
#include <libintl.h>

#include "config.h"

#define _(x) gettext(x)
static void fcitx_reload_config(GtkMenuItem *menuitem, gpointer user_data);

GtkWidget* fcitx_config_menu_new()
{
    GtkWidget *menu = gtk_menu_bar_new();
    
    GtkWidget *menuItemFile = gtk_menu_item_new_with_mnemonic(_("_File"));
    GtkWidget *menuFile = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItemFile), menuFile);
    gtk_menu_bar_append(menu,menuItemFile);

    GtkWidget *menuitem, *image;

    menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Reload Config"));
    image = gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_BUTTON);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
    gtk_menu_append(menuFile, menuitem);
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate", G_CALLBACK(fcitx_reload_config), NULL);

    menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Exit"));
    image = gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_BUTTON);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
    gtk_menu_append(menuFile, menuitem);
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate", G_CALLBACK(gtk_main_quit), NULL);

    return menu;
}

void fcitx_reload_config(GtkMenuItem *menuitem, gpointer user_data)
{
    system(EXEC_PREFIX "/fcitx-remote -r");
}
