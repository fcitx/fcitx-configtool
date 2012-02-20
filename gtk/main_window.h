/***************************************************************************
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

#ifndef MAIN_WINDOW_H

#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>

#include "common.h"

G_BEGIN_DECLS

#define FCITX_TYPE_MAIN_WINDOW fcitx_main_window_get_type()

#define FCITX_MAIN_WINDOW(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_MAIN_WINDOW, FcitxMainWindow))

#define FCITX_MAIN_WINDOW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_MAIN_WINDOW, FcitxMainWindowClass))

#define FCITX_IS_MAIN_WINDOW(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_MAIN_WINDOW))

#define FCITX_IS_MAIN_WINDOW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_MAIN_WINDOW))

#define FCITX_MAIN_WINDOW_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_MAIN_WINDOW, FcitxMainWindowClass))

typedef struct {
    GtkWidget* page;
    GtkTreeIter iter;
} ConfigPage;

typedef struct {
    GtkWindow parent;
    GtkWidget* pageview;
    GtkListStore *pagestore;
    GtkWidget* vbox;
    GtkWidget* pagelabel;
    ConfigPage* impage;
    ConfigPage* configpage;
    ConfigPage* lastpage;
    ConfigPage* addonpage;
    GtkWidget* button;
    GtkWidget* addonview;
    UT_array* addons;

} FcitxMainWindow;

typedef struct {
    GtkWindowClass parent_class;
} FcitxMainWindowClass;

GType fcitx_main_window_get_type(void);

GtkWidget* fcitx_main_window_new(void);

#endif
