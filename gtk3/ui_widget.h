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

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "fcitx-gclient/fcitxinputmethod.h"

G_BEGIN_DECLS

#define FCITX_TYPE_UI_WIDGET fcitx_ui_widget_get_type()

#define FCITX_UI_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_UI_WIDGET, FcitxUIWidget))

#define FCITX_UI_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_UI_WIDGET, FcitxUIWidgetClass))

#define FCITX_IS_UI_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_UI_WIDGET))

#define FCITX_IS_UI_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_UI_WIDGET))

#define FCITX_UI_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_UI_WIDGET, FcitxUIWidgetClass))

typedef struct _FcitxUIWidget FcitxUIWidget;
typedef struct _FcitxUIWidgetClass FcitxUIWidgetClass;

struct _FcitxUIWidget {
    GtkBox parent;
    FcitxInputMethod* improxy;
    GtkWidget* label;
};

struct _FcitxUIWidgetClass {
    GtkBoxClass parent_class;
};

GType        fcitx_ui_widget_get_type(void) G_GNUC_CONST;

GtkWidget*
fcitx_ui_widget_new(void);

void
fcitx_ui_widget_connect(FcitxUIWidget* widget);

G_END_DECLS


#endif