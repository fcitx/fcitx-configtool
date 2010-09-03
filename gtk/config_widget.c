#include "uthash.h"
#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>
#include <libintl.h>

#include "config_widget.h"

#define _(s) gettext(s)

GtkWidget* config_widget_new(ConfigFileDesc *cfdesc)
{
    GtkWidget *configNotebook = gtk_notebook_new();
    ConfigGroupDesc *cgdesc = NULL;
    ConfigOptionDesc *codesc = NULL;
    for(cgdesc = cfdesc->groupsDesc;
        cgdesc != NULL;
        cgdesc = (ConfigGroupDesc*)cgdesc->hh.next)
    {
        codesc = cgdesc->optionsDesc;
        if (codesc == NULL)
            continue;

        GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
        GtkWidget *plabel = gtk_label_new(_(cgdesc->groupName));
        GtkWidget *scrollwnd = gtk_scrolled_window_new(NULL, NULL);

        gtk_box_set_spacing(GTK_BOX(vbox), 4);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollwnd), vbox);
        gtk_notebook_append_page(GTK_NOTEBOOK(configNotebook),
                scrollwnd,
                plabel);
        
        for ( ; codesc != NULL;
            codesc = (ConfigOptionDesc*)codesc->hh.next)
        {
            GtkWidget* hbox = gtk_hbox_new(FALSE, 5);
            GtkWidget* label = gtk_label_new(codesc->optionName);
            gtk_widget_set_size_request(label, 220, -1);
            g_object_set(label, "xalign", 0.0f, NULL);

            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
            GtkWidget *inputWidget = NULL;


            switch(codesc->type)
            {
                case T_Integer:
                    inputWidget = gtk_entry_new();
                    break;
                case T_Color:
                    inputWidget = gtk_color_button_new();
                    break;
                case T_Boolean:
                    inputWidget = gtk_check_button_new();
                    break;
                case T_File:
                    inputWidget = gtk_file_chooser_button_new(codesc->optionName, GTK_FILE_CHOOSER_ACTION_OPEN);
                    break;
                case T_Font:
                    inputWidget = gtk_font_button_new();
                    break;
                case T_Enum:
                    {
                        int i;
                        ConfigEnum *e = &codesc->configEnum;
                        inputWidget = gtk_combo_box_new_text();
                        for (i = 0; i < e->enumCount; i ++)
                        {
                            gtk_combo_box_append_text(GTK_COMBO_BOX(inputWidget), _(e->enumDesc[i]));
                        }
                    }
                    break;
                case T_Char:
                case T_Image:
                case T_Hotkey:
                case T_String:
                    inputWidget = gtk_entry_new();
                    break;
            }
            gtk_box_pack_start(GTK_BOX(hbox), inputWidget, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
        }
    }

    return configNotebook;
}
