/***************************************************************************
 *   Copyright (C) 2013~2014 by Lenky0401                                  *
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

#ifndef WIZARD_HOTKEY_WIDGET_H
#define WIZARD_HOTKEY_WIDGET_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "fcitx-gclient/fcitxinputmethod.h"
#include "sub_config_parser.h"
#include "dummy_config.h"
#include "wizard_conf_data.h"

G_BEGIN_DECLS

#define FCITX_TYPE_WIZARD_HOTKEY_WIDGET fcitx_wizard_hotkey_widget_get_type()

#define FCITX_WIZARD_HOTKEY_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_WIZARD_HOTKEY_WIDGET, FcitxWizardHotkeyWidget))

#define FCITX_WIZARD_HOTKEY_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_WIZARD_HOTKEY_WIDGET, FcitxWizardHotkeyWidgetClass))

#define FCITX_IS_WIZARD_HOTKEY_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_WIZARD_HOTKEY_WIDGET))

#define FCITX_IS_WIZARD_HOTKEY_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_WIZARD_HOTKEY_WIDGET))

#define FCITX_WIZARD_HOTKEY_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_WIZARD_HOTKEY_WIDGET, FcitxWizardHotkeyWidgetClass))

typedef struct _FcitxWizardHotkeyWidget FcitxWizardHotkeyWidget;
typedef struct _FcitxWizardHotkeyWidgetClass FcitxWizardHotkeyWidgetClass;

typedef struct _FcitxWizardHotkeyConfData FcitxWizardHotkeyConfData;

struct _FcitxWizardHotkeyConfData {
    FcitxHotkeys trigger_key;
    boolean im_switch_key;
    int im_switch_hotkey;
    FcitxHotkeys prev_page;
    FcitxHotkeys next_page;
};

struct _FcitxWizardHotkeyWidget {
    GtkBox parent;
    Wizard_Conf_Data *config_conf_data;
    GtkWidget *trigger_key_button[2];
    GtkWidget *im_switch_key_button;
    GtkWidget *im_switch_hotkey_combo;
    GtkWidget *prev_page_button[2];
    GtkWidget *next_page_button[2];

    FcitxWizardHotkeyConfData conf_data;
};

struct _FcitxWizardHotkeyWidgetClass {
    GtkBoxClass parent_class;
};

GtkWidget*
fcitx_wizard_hotkey_widget_new(Wizard_Conf_Data *config);

G_END_DECLS


#endif
