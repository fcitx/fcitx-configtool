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

#ifndef WIZARD_SKIN_WIDGET_H
#define WIZARD_SKIN_WIDGET_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "fcitx-gclient/fcitxinputmethod.h"
#include "sub_config_parser.h"
#include "dummy_config.h"
#include "wizard_conf_data.h"

G_BEGIN_DECLS

#define FCITX_TYPE_WIZARD_SKIN_WIDGET fcitx_wizard_skin_widget_get_type()

#define FCITX_WIZARD_SKIN_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_WIZARD_SKIN_WIDGET, FcitxWizardSkinWidget))

#define FCITX_WIZARD_SKIN_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_WIZARD_SKIN_WIDGET, FcitxWizardSkinWidgetClass))

#define FCITX_IS_WIZARD_SKIN_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_WIZARD_SKIN_WIDGET))

#define FCITX_IS_WIZARD_SKIN_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_WIZARD_SKIN_WIDGET))

#define FCITX_WIZARD_SKIN_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_WIZARD_SKIN_WIDGET, FcitxWizardSkinWidgetClass))

typedef struct _FcitxWizardSkinWidget FcitxWizardSkinWidget;
typedef struct _FcitxWizardSkinWidgetClass FcitxWizardSkinWidgetClass;

typedef struct _FcitxWizardSkinConfData FcitxWizardSkinConfData;

#define SKIN_VALUE_MAX_LEN 32

struct _FcitxWizardSkinConfData {
    char *skin_type;
};

struct _FcitxWizardSkinWidget {
    GtkBox parent;
    Wizard_Conf_Data *classic_ui_conf_data;
    DummyConfig* config;
    GtkWidget* default_skin;
    GtkWidget* classic_skin;
    GtkWidget* dark_skin;
    GtkWidget* default_skin_img;
    GtkWidget* classic_skin_img;
    GtkWidget* dark_skin_img;
    GtkBuilder* builder;
    FcitxWizardSkinConfData conf_data;
};

struct _FcitxWizardSkinWidgetClass {
    GtkBoxClass parent_class;
};

GtkWidget*
fcitx_wizard_skin_widget_new(Wizard_Conf_Data *classic_ui);


G_END_DECLS


#endif
