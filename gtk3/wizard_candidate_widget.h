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

#ifndef WIZARD_CANDIDATE_WIDGET_H
#define WIZARD_CANDIDATE_WIDGET_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "fcitx-gclient/fcitxinputmethod.h"
#include "sub_config_parser.h"
#include "dummy_config.h"
#include "wizard_conf_data.h"

G_BEGIN_DECLS

#define FCITX_TYPE_WIZARD_CANDIDATE_WIDGET fcitx_wizard_candidate_widget_get_type()

#define FCITX_WIZARD_CANDIDATE_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_WIZARD_CANDIDATE_WIDGET, FcitxWizardCandidateWidget))

#define FCITX_WIZARD_CANDIDATE_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_WIZARD_CANDIDATE_WIDGET, FcitxWizardCandidateWidgetClass))

#define FCITX_IS_WIZARD_CANDIDATE_WIDGET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_WIZARD_CANDIDATE_WIDGET))

#define FCITX_IS_WIZARD_CANDIDATE_WIDGET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_WIZARD_CANDIDATE_WIDGET))

#define FCITX_WIZARD_CANDIDATE_WIDGET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_WIZARD_CANDIDATE_WIDGET, FcitxWizardCandidateWidgetClass))

typedef struct _FcitxWizardCandidateWidget FcitxWizardCandidateWidget;
typedef struct _FcitxWizardCandidateWidgetClass FcitxWizardCandidateWidgetClass;


typedef struct _FcitxWizardCandidateConfData FcitxWizardCandidateConfData;

#define FONT_VALUE_MAX_LEN 128
struct _FcitxWizardCandidateConfData {
    int candidate_word_number;
    int font_size;
    char *font;
    boolean vertical_candidate;
};

struct _FcitxWizardCandidateWidget {
    GtkBox parent;

    Wizard_Conf_Data *config_conf_data;
    Wizard_Conf_Data *classic_ui_conf_data;

    GtkWidget *candidate_word_number_spin_button;
    GtkWidget *font_size_spin_button;
    GtkWidget *font_button;
    GtkWidget *vertical_candidate_button;

    FcitxWizardCandidateConfData conf_data;

    
};

struct _FcitxWizardCandidateWidgetClass {
    GtkBoxClass parent_class;
};

GtkWidget*
fcitx_wizard_candidate_widget_new(Wizard_Conf_Data *config,
    Wizard_Conf_Data *classic_ui);


G_END_DECLS


#endif
