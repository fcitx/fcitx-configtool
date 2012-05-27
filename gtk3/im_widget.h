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

#ifndef IM_WIDGET_H
#define IM_WIDGET_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "fcitx-gclient/fcitxinputmethod.h"

G_BEGIN_DECLS

#define FCITX_TYPE_IM_WIDGET fcitx_im_widget_get_type()

#define FCITX_IM_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_IM_WIDGET, FcitxImWidget))

#define FCITX_IM_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_IM_WIDGET, FcitxImWidgetClass))

#define FCITX_IS_IM_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_IM_WIDGET))

#define FCITX_IS_IM_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_IM_WIDGET))

#define FCITX_IM_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_IM_WIDGET, FcitxImWidgetClass))

typedef struct _FcitxImWidget FcitxImWidget;
typedef struct _FcitxImWidgetClass FcitxImWidgetClass;

struct _FcitxImWidget {
    GtkBox parent;
    GtkListStore* imstore;
    GtkWidget* imview;
    GtkWidget* addimbutton;
    GtkWidget* delimbutton;
    GtkWidget* moveupbutton;
    GtkWidget* movedownbutton;
    FcitxInputMethod* improxy;
    GPtrArray* array;
    GtkWidget* configurebutton;
    gchar* focus;
};

struct _FcitxImWidgetClass {
    GtkBoxClass parent_class;
};

GtkWidget*
fcitx_im_widget_new(void);

G_END_DECLS


#endif