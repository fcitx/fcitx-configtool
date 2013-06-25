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

#include <fcitx-utils/utils.h>
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>

#include "common.h"
#include "wizard_candidate_widget.h"
#include "gdm-languages.h"
#include "config_widget.h"

#include "sub_config_widget.h"
#include "configdesc.h"
#include "dummy_config.h"

enum {
    PROP_0,

    PROP_CONFIG,
    PROP_CLASSIC_UI
};

G_DEFINE_TYPE(FcitxWizardCandidateWidget, fcitx_wizard_candidate_widget, GTK_TYPE_BOX)

static void fcitx_wizard_candidate_widget_dispose(GObject* object);

static void
fcitx_wizard_candidate_widget_set_property(GObject *gobject,
    guint prop_id, const GValue *value, GParamSpec *pspec);


static GObject *
fcitx_wizard_candidate_widget_constructor(GType gtype, guint n_properties,
    GObjectConstructParam *properties);

static void
fcitx_wizard_candidate_widget_class_init(FcitxWizardCandidateWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->set_property = fcitx_wizard_candidate_widget_set_property;
    gobject_class->dispose = fcitx_wizard_candidate_widget_dispose;
    gobject_class->constructor = fcitx_wizard_candidate_widget_constructor;

    g_object_class_install_property(gobject_class, PROP_CONFIG,
        g_param_spec_pointer("config", "", "", 
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class, PROP_CLASSIC_UI,
        g_param_spec_pointer("classic_ui", "", "", 
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

}


static GObject *
fcitx_wizard_candidate_widget_constructor(GType gtype,
    guint n_properties, GObjectConstructParam *properties)
{
    GObject *obj;
    obj = G_OBJECT_CLASS(fcitx_wizard_candidate_widget_parent_class)->constructor(
        gtype, n_properties, properties);
    return obj;
}

static void
fcitx_wizard_candidate_widget_init(FcitxWizardCandidateWidget* self)
{

}

static void set_none_font_clicked(GtkWidget *button, gpointer arg)
{
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), "");
}

static void
fcitx_wizard_candidate_widget_load_conf(FcitxWizardCandidateWidget *self)
{
    FcitxConfigValueType value;
    File_Conf_Data* config_conf = self->config_conf_data->conf_data;
    File_Conf_Data* classic_ui_conf = self->classic_ui_conf_data->conf_data;

    value = FcitxConfigGetBindValue(&config_conf->config->config, "Output", 
        "CandidateWordNumber");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->candidate_word_number_spin_button), 
        *value.integer);

    value = FcitxConfigGetBindValue(&classic_ui_conf->config->config, "ClassicUI", 
        "FontSize");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->font_size_spin_button), 
        *value.integer);

    value = FcitxConfigGetBindValue(&classic_ui_conf->config->config, "ClassicUI", 
        "Font");
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(self->font_button), 
        *value.string);

    value = FcitxConfigGetBindValue(&classic_ui_conf->config->config, "ClassicUI", 
        "VerticalList");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->vertical_candidate_button),
        *value.boolvalue); 

}

void 
candidate_word_number_value_changed(GtkWidget* button, 
    gpointer user_data)
{
    FcitxWizardCandidateWidget *self = user_data;
    File_Conf_Data* config_conf = self->config_conf_data->conf_data;

    self->conf_data.candidate_word_number = 
        gtk_spin_button_get_value(GTK_SPIN_BUTTON(button));
    FcitxConfigBindValue(config_conf->config->config.configFile, "Output", 
        "CandidateWordNumber", 
        &self->conf_data.candidate_word_number, NULL, NULL);
}

void 
font_size_value_changed(GtkWidget* button, 
    gpointer user_data)
{
    FcitxWizardCandidateWidget *self = user_data;
    File_Conf_Data* classic_ui_conf = self->classic_ui_conf_data->conf_data;

    self->conf_data.font_size = 
        gtk_spin_button_get_value(GTK_SPIN_BUTTON(button));
    
    FcitxConfigBindValue(classic_ui_conf->config->config.configFile, "ClassicUI", 
        "FontSize", 
        &self->conf_data.font_size, NULL, NULL);

}

void 
font_button_font_set(GtkWidget* button, 
    gpointer user_data)
{
    const char *value;
    FcitxWizardCandidateWidget *self = user_data;
    File_Conf_Data* classic_ui_conf = self->classic_ui_conf_data->conf_data;

    value = gtk_font_button_get_font_name(GTK_FONT_BUTTON(self->font_button));

    if (self->conf_data.font == NULL &&
        (self->conf_data.font = malloc(FONT_VALUE_MAX_LEN)) == NULL)
    {
        FcitxLog(WARNING, _("Malloc memory(%d) failed.\n"), FONT_VALUE_MAX_LEN);
        return;
    }

    strcpy(self->conf_data.font, value);
    
    FcitxConfigBindValue(classic_ui_conf->config->config.configFile, "ClassicUI", 
        "Font", &self->conf_data.font, NULL, NULL);
}

void 
vertical_candidate_button_toggled(GtkWidget* button, 
    gpointer user_data)
{
    FcitxWizardCandidateWidget *self = user_data;
    File_Conf_Data* classic_ui_conf = self->classic_ui_conf_data->conf_data;

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
        self->conf_data.vertical_candidate = true;
    else
        self->conf_data.vertical_candidate = false;

    FcitxConfigBindValue(classic_ui_conf->config->config.configFile, "ClassicUI", 
        "VerticalList", &self->conf_data.vertical_candidate, NULL, NULL);
}

static void
fcitx_wizard_candidate_widget_setup_ui(FcitxWizardCandidateWidget *self)
{
    int row = 0;
    GtkWidget *cvbox = GTK_WIDGET(self);
    GtkWidget *hbox;

    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_margin_left(grid, 0);
    gtk_widget_set_margin_top(grid, 6);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);

    GtkWidget* label = gtk_label_new(("Candidate Word Number"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->candidate_word_number_spin_button = gtk_spin_button_new_with_range(
            1, 10, 1.0);
    g_object_set(self->candidate_word_number_spin_button, "hexpand", TRUE, NULL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->candidate_word_number_spin_button, 1, 
        row, 1, 1);

    row ++;
    label = gtk_label_new(("Font Size"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->font_size_spin_button = gtk_spin_button_new_with_range(
            0, 72, 1.0);
    g_object_set(self->font_size_spin_button, "hexpand", TRUE, NULL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->font_size_spin_button, 1, 
        row, 1, 1);

    row ++;
    label = gtk_label_new(("Font"));
    g_object_set(label, "xalign", 0.0f, NULL);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->font_button = gtk_font_button_new();
    GtkWidget *button = gtk_button_new_with_label(_("Clear font setting"));
    gtk_box_pack_start(GTK_BOX(hbox), self->font_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(self->font_button), FALSE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(self->font_button), FALSE);
    g_signal_connect(G_OBJECT(button), "clicked", (GCallback) set_none_font_clicked, 
        self->font_button);
    
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hbox, 1, row, 1, 1);    

    row ++;
    label = gtk_label_new(("Vertical Candidate Word List"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->vertical_candidate_button = gtk_check_button_new();
    g_object_set(self->vertical_candidate_button, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->vertical_candidate_button, 1, row, 1, 1);
    
 
    gtk_box_pack_start(GTK_BOX(cvbox), grid, TRUE, TRUE, 0);

    fcitx_wizard_candidate_widget_load_conf(self);

    g_signal_connect(G_OBJECT(self->candidate_word_number_spin_button), "value-changed",
                 G_CALLBACK(candidate_word_number_value_changed), self);
    g_signal_connect(G_OBJECT(self->font_size_spin_button), "value-changed",
                 G_CALLBACK(font_size_value_changed), self);
    g_signal_connect(G_OBJECT(self->font_button), "font-set",
                 G_CALLBACK(font_button_font_set), self);
    g_signal_connect(G_OBJECT(self->vertical_candidate_button), "toggled",
                 G_CALLBACK(vertical_candidate_button_toggled), self);

}

GtkWidget*
fcitx_wizard_candidate_widget_new(Wizard_Conf_Data *config,
    Wizard_Conf_Data *classic_ui)
{
    FcitxWizardCandidateWidget* widget =
        g_object_new(FCITX_TYPE_WIZARD_CANDIDATE_WIDGET,
            "config", config, "classic_ui", classic_ui,
             NULL);

    fcitx_wizard_candidate_widget_setup_ui(widget);

    return GTK_WIDGET(widget);
}

void fcitx_wizard_candidate_widget_dispose(GObject* object)
{
    G_OBJECT_CLASS(fcitx_wizard_candidate_widget_parent_class)->dispose(object);
}

static void
fcitx_wizard_candidate_widget_set_property(GObject *gobject,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
    FcitxWizardCandidateWidget* config_widget = FCITX_WIZARD_CANDIDATE_WIDGET(gobject);
    switch (prop_id) {
    case PROP_CONFIG:
        config_widget->config_conf_data = g_value_get_pointer(value);
        break;
    case PROP_CLASSIC_UI:
        config_widget->classic_ui_conf_data = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}


