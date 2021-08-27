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

#ifndef IM_CONFIG_DIALOG_H
#define IM_CONFIG_DIALOG_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <fcitx-gclient/fcitxkbd.h>
#include "config_widget.h"

G_BEGIN_DECLS

#define FCITX_TYPE_IM_CONFIG_DIALOG fcitx_im_config_dialog_get_type()

#define FCITX_IM_CONFIG_DIALOG(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_IM_CONFIG_DIALOG, FcitxImConfigDialog))

#define FCITX_IM_CONFIG_DIALOG_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_IM_CONFIG_DIALOG, FcitxImConfigDialogClass))

#define FCITX_IS_IM_CONFIG_DIALOG(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_IM_CONFIG_DIALOG))

#define FCITX_IS_IM_CONFIG_DIALOG_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_IM_CONFIG_DIALOG))

#define FCITX_IM_CONFIG_DIALOG_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_IM_CONFIG_DIALOG, FcitxImConfigDialogClass))

typedef struct _FcitxImConfigDialog FcitxImConfigDialog;
typedef struct _FcitxImConfigDialogClass FcitxImConfigDialogClass;

struct _FcitxImConfigDialog {
    GtkDialog parent;
    FcitxConfigWidget* config_widget;
    FcitxKbd* kbd;
    GtkListStore* model;
    gchar* imname;
    GtkWidget* combobox;
};

struct _FcitxImConfigDialogClass {
    GtkDialogClass parent_class;
};

int fcitx_im_config_thirdpart(FcitxConfigFileDesc *cdesc);

GtkWidget*
fcitx_im_config_dialog_new(GtkWindow* parent, FcitxAddon* addon, gchar* imname);

G_END_DECLS


#endif
