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

#ifndef WIZARD_IM_WIDGET_H
#define WIZARD_IM_WIDGET_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "fcitx-gclient/fcitxinputmethod.h"
#include "wizard_conf_data.h"

G_BEGIN_DECLS

#define FCITX_TYPE_WIZARD_IM_WIDGET fcitx_wizard_im_widget_get_type()

#define FCITX_WIZARD_IM_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), FCITX_TYPE_WIZARD_IM_WIDGET, FcitxWizardImWidget))

#define FCITX_WIZARD_IM_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), FCITX_TYPE_WIZARD_IM_WIDGET, FcitxWizardImWidgetClass))

#define FCITX_IS_WIZARD_IM_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), FCITX_TYPE_WIZARD_IM_WIDGET))

#define FCITX_IS_WIZARD_IM_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), FCITX_TYPE_WIZARD_IM_WIDGET))

#define FCITX_WIZARD_IM_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), FCITX_TYPE_WIZARD_IM_WIDGET, FcitxWizardImWidgetClass))

typedef struct _FcitxWizardImWidget FcitxWizardImWidget;
typedef struct _FcitxWizardImWidgetClass FcitxWizardImWidgetClass;

struct _FcitxWizardImWidget {
    GtkBox parent;
    GtkListStore* imstore;
    GtkWidget* imview;
    FcitxInputMethod* improxy;
    GPtrArray* array;
    GPtrArray* im_dialog_array;
    GPtrArray* im_dialog_array_del;
    gchar* focus;
    GtkWidget* addimbutton;
    GtkWidget* delimbutton;
    GtkWidget* moveupbutton;
    GtkWidget* movedownbutton;
    GtkWidget* scrolledwindow;
    GtkWidget* toolbar;
    GtkBuilder* builder;
    Wizard_Conf_Data *conf_data;
};

struct _FcitxWizardImWidgetClass {
    GtkBoxClass parent_class;
};

GtkWidget*
fcitx_wizard_im_widget_new(Wizard_Conf_Data *conf_data);

void 
_fcitx_wizard_im_widget_refresh_view(FcitxWizardImWidget* self);

void _fcitx_wizard_im_widget_update(void* data);

G_END_DECLS


#endif
