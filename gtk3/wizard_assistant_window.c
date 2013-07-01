/***************************************************************************
 *   Copyright (C) 2013~2014 by Lenky0401                                  *
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

#include <stdlib.h>
#include <libintl.h>
#include <errno.h>

#include <fcitx/addon.h>
#include <fcitx-utils/utarray.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

#include "config.h"
#include "config_widget.h"
#include "configdesc.h"
#include "im_widget.h"
#include "ui_widget.h"
#include "wizard_im_widget.h"
#include "wizard_skin_widget.h"
#include "wizard_candidate_widget.h"
#include "wizard_hotkey_widget.h"
#include "wizard_assistant_window.h"
#include "configdesc.h"
#include "common.h"
#include "gdm-languages.h"

#include "wizard_conf_data.h"

#define FILE_CONF_DATA_CURT_NUM (2)
#define WIZARD_CONF_DATA_CURT_NUM (3)

static File_Conf_Data file_conf_data[FILE_CONF_DATA_CURT_NUM];
static Wizard_Conf_Data wizard_conf_data[WIZARD_CONF_DATA_CURT_NUM];
static GtkWidget* im_widget;

static void assistant_cancel(GtkAssistant * assistant, gpointer data);
static void assistant_close(GtkAssistant * assistant, gpointer data);
static void assistant_apply(GtkAssistant * assistant, gpointer data);

GtkWidget *
page_input_method_engin(void)
{
    GtkWidget* page_box;
    im_widget = fcitx_wizard_im_widget_new(&wizard_conf_data[0]);
   
    page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(page_box), im_widget, TRUE, TRUE, 0);
   
    return page_box;
}

GtkWidget *
page_personality_skin(void)
{
    GtkWidget* page_box;
    GtkWidget* skin_widget = fcitx_wizard_skin_widget_new(
        &wizard_conf_data[2]);
   
    page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(page_box), skin_widget, TRUE, TRUE, 0);
   
    return page_box;
}

GtkWidget *
page_candidate_word(void)
{
    GtkWidget* page_box;
    GtkWidget* candidate_widget = fcitx_wizard_candidate_widget_new(
        &wizard_conf_data[1], &wizard_conf_data[2]);
   
    page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(page_box), candidate_widget, TRUE, TRUE, 0);
   
    return page_box;
}

GtkWidget *
page_hotkey(void)
{
    GtkWidget* page_box;
    GtkWidget* hotkey_widget = fcitx_wizard_hotkey_widget_new(
        &wizard_conf_data[1]);
   
    page_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(page_box), hotkey_widget, TRUE, TRUE, 0);
   
    return page_box;
}

boolean load_wizard_conf_data()
{
    int i;
    FILE *fp;

    file_conf_data[0].path_prefix = "";
    file_conf_data[0].file_name = "config";
    file_conf_data[0].cfdesc = get_config_desc("config.desc");
    
    file_conf_data[1].path_prefix = "conf";
    file_conf_data[1].file_name = "fcitx-classic-ui.config";
    file_conf_data[1].cfdesc = get_config_desc("fcitx-classic-ui.desc");
    
    for (i = 0; i < FILE_CONF_DATA_CURT_NUM; i ++) {
        if (file_conf_data[i].cfdesc == NULL)
            return false;

        bindtextdomain(file_conf_data[i].cfdesc->domain, LOCALEDIR);
        bind_textdomain_codeset(file_conf_data[i].cfdesc->domain, "UTF-8");

        file_conf_data[i].config = dummy_config_new(file_conf_data[i].cfdesc);
        if ((fp = FcitxXDGGetFileWithPrefix(file_conf_data[i].path_prefix, 
            file_conf_data[i].file_name, "r", NULL)) == NULL) 
        {
            FcitxLog(WARNING, _("Open file(%s/%s) error.\n"), 
                file_conf_data[i].path_prefix, file_conf_data[i].file_name);
            return false;
        }

        dummy_config_load(file_conf_data[i].config, fp);
        dummy_config_sync(file_conf_data[i].config);

        fclose(fp);

    }

    wizard_conf_data[0].conf_data = NULL;
    wizard_conf_data[1].conf_data = &file_conf_data[0];
    wizard_conf_data[2].conf_data = &file_conf_data[1];

    return true;
}

void 
update_file_conf_data(void *data)
{
    FILE *fp;
    File_Conf_Data *fcd = (File_Conf_Data *)data;
    
    if ((fp = FcitxXDGGetFileUserWithPrefix(fcd->path_prefix, fcd->file_name, "w", NULL))
        == NULL) 
    {
        FcitxLog(WARNING, _("Open file(%s/%s) error.\n"), fcd->path_prefix, fcd->file_name);
        return;
    }

    FcitxConfigSaveConfigFileFp(fp, &fcd->config->config, fcd->cfdesc);
    fclose(fp);
}

void 
update_wizard_conf_data()
{
    int i;
    GError  *error;
    gchar *argv[3];

    _fcitx_wizard_im_widget_update(im_widget);
    
    for (i = 0; i < FILE_CONF_DATA_CURT_NUM; i ++) {
        update_file_conf_data(&file_conf_data[i]);
    }

    argv[0] = EXEC_PREFIX "/bin/fcitx-remote";
    argv[1] = "-r";
    argv[2] = 0;
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
}
    
GtkWidget *
create_assistant(void)
{
    guint i;
    GtkWidget *assistant;
    PageInfo page[PAGE_INFO_NUM] = {
        {NULL, -1, _("Step 1"), GTK_ASSISTANT_PAGE_INTRO, TRUE},
        {NULL, -1, _("Step 2"), GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, _("Step 3"), GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, _("Step 4"), GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, _("Step 5"), GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, _("Step 6"), GTK_ASSISTANT_PAGE_CONFIRM, TRUE},
    };

    if (!load_wizard_conf_data()) {
        FcitxLog(WARNING, _("load_wizard_conf_data() failed.\n"));
        exit(-1);
    }

    assistant = gtk_assistant_new();
    gtk_widget_set_size_request(assistant, 650, 450);
    gtk_window_set_position(GTK_WINDOW(assistant), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(assistant), _("Fcitx Configure Assistant"));

    page[0].widget = gtk_label_new(NULL);
    gchar* s = g_strdup_printf(_("<b>Welcome to use the configure assistant!</b>"));
    gtk_label_set_markup(GTK_LABEL(page[0].widget), s);
    g_free(s);

    page[1].widget = page_input_method_engin();
    page[2].widget = page_personality_skin();
    page[3].widget = page_candidate_word();
    page[4].widget = page_hotkey();
    page[5].widget = gtk_label_new(NULL);
    s = g_strdup_printf(_("<b>Congratulations! You are finished configuring the settings for the Fcitx!</b>"));
    gtk_label_set_markup(GTK_LABEL(page[5].widget), s);
    g_free(s);

    for (i = 0; i < PAGE_INFO_NUM; i++) {
        page[i].index = gtk_assistant_append_page(GTK_ASSISTANT(assistant), 
            page[i].widget);
        
        gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), page[i].widget, 
            page[i].title);
        gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), page[i].widget, 
            page[i].type);

        gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), page[i].widget, 
            page[i].complete);
    }

    g_signal_connect(G_OBJECT(assistant), "cancel", G_CALLBACK(assistant_cancel), 
        NULL);
    g_signal_connect(G_OBJECT(assistant), "close", G_CALLBACK (assistant_close), 
        NULL);
    g_signal_connect(G_OBJECT(assistant), "apply", G_CALLBACK (assistant_apply), 
        NULL);
    
    return assistant;
}

static void
assistant_cancel(GtkAssistant * assistant, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(assistant));
}

static void
assistant_close(GtkAssistant * assistant, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(assistant));
}

static void
assistant_apply(GtkAssistant * assistant, gpointer data)
{
    update_wizard_conf_data();
}


