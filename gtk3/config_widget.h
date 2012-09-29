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

/* fcitx-config-widget.h */

#ifndef _FCITX_CONFIG_WIDGET
#define _FCITX_CONFIG_WIDGET

#include <gtk/gtk.h>
#include <glib.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx/addon.h>
#include "sub_config_parser.h"
#include "dummy_config.h"

G_BEGIN_DECLS

#define FCITX_TYPE_CONFIG_WIDGET fcitx_config_widget_get_type()

#define FCITX_CONFIG_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_CONFIG_WIDGET, FcitxConfigWidget))

#define FCITX_CONFIG_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_CONFIG_WIDGET, FcitxConfigWidgetClass))

#define FCITX_IS_CONFIG_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_CONFIG_WIDGET))

#define FCITX_IS_CONFIG_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_CONFIG_WIDGET))

#define FCITX_CONFIG_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_CONFIG_WIDGET, FcitxConfigWidgetClass))


enum UIType {
    CW_Simple = 0x1,
    CW_Full = 0x2,
    CW_NoShow = 0x0
};

typedef struct {
    GtkGrid parent;
    FcitxConfigFileDesc* cfdesc;
    gchar* prefix;
    gchar* name;
    FcitxSubConfigParser* parser;
    GHashTable* argmap;
    enum UIType fullUiType;
    enum UIType simpleUiType;
    GtkWidget* simpleWidget;
    GtkWidget* fullWidget;
    GtkWidget* advanceCheckBox;
    DummyConfig* config;
} FcitxConfigWidget;

typedef struct {
    GtkGridClass parent_class;
} FcitxConfigWidgetClass;

typedef enum {
    CONFIG_WIDGET_SAVE,
    CONFIG_WIDGET_CANCEL,
    CONFIG_WIDGET_DEFAULT
} ConfigWidgetAction;

GType fcitx_config_widget_get_type(void);

FcitxConfigWidget* fcitx_config_widget_new(FcitxConfigFileDesc* cfdesc, const gchar* prefix, const gchar* name, const char* subconfig);

void fcitx_config_widget_response(FcitxConfigWidget* config_widget, ConfigWidgetAction action);

gboolean fcitx_config_widget_response_cb(GtkDialog *dialog,
        gint response,
        gpointer user_data);

GtkWidget* fcitx_config_dialog_new(FcitxAddon* addon, GtkWindow* parent);

G_END_DECLS

#endif /* _FCITX_CONFIG_WIDGET */
