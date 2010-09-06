#include "uthash.h"
#include <gtk/gtk.h>
#include <fcitx-config/fcitx-config.h>
#include <libintl.h>
#include <stdlib.h>
#include <libgen.h>

#include "config_widget.h"

#define _(s) gettext(s)
static void sync_filter(ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg);

static void set_none_font_clicked(GtkWidget *button, gpointer arg)
{
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), "");
}

static void reset_config_clicked(GtkWidget* button, gpointer arg)
{
    ConfigPage *page = (ConfigPage*) arg;
    ConfigBindSync(&page->config);
}

static void save_config_clicked(GtkWidget* button, gpointer arg)
{
    ConfigPage *page = (ConfigPage*) arg;

    FILE *fp;
    
    fp = fopen(page->filename, "w");
    fprintf(stderr, "%s\n", page->filename);
    if (!fp)
    {
        char *sbak = strdup(page->filename);
        char *dir = dirname(sbak);
        make_path(dir);
        fp = fopen (page->filename, "w");
        free(sbak);
    }
    if (fp)
    {
        SaveConfigFileFp(fp, page->config.configFile, page->cfdesc);
        fclose(fp);
        fp = fopen(page->filename, "rt");
        page->config.configFile = ParseIniFp(fp, page->config.configFile);
        fclose(fp);
    }
}

GtkWidget* config_widget_new(ConfigFileDesc *cfdesc, ConfigFile *cfile, ConfigPage *page)
{
    GtkWidget *cvbox = gtk_vbox_new(FALSE, 0);
    GtkWidget *chbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *configNotebook = gtk_notebook_new();
    GtkWidget *saveButton = gtk_button_new_with_label(_("Save"));
    GtkWidget *resetButton = gtk_button_new_with_label(_("Reset"));
                        
    gtk_signal_connect(GTK_OBJECT(resetButton), "clicked", GTK_SIGNAL_FUNC(reset_config_clicked), page);
    gtk_signal_connect(GTK_OBJECT(saveButton), "clicked", GTK_SIGNAL_FUNC(save_config_clicked), page);
        
    gtk_box_pack_start(GTK_BOX(cvbox), configNotebook, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(cvbox), chbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(chbox), saveButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(chbox), resetButton, FALSE, FALSE, 0);
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
            const char *s;
            if (codesc->desc && strlen(codesc->desc) != 0)
                s = _(codesc->desc);
            else
                s = _(codesc->optionName);
            GtkWidget* label = gtk_label_new(s);
            gtk_widget_set_size_request(label, 220, -1);
            g_object_set(label, "xalign", 0.0f, NULL);

            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
            GtkWidget *inputWidget = NULL;
            GtkWidget *argWidget = NULL;

            switch(codesc->type)
            {
                case T_Integer:
                    inputWidget = gtk_spin_button_new_with_range(-1.0, 10000.0, 1.0);
                    argWidget = inputWidget;
                    break;
                case T_Color:
                    inputWidget = gtk_color_button_new();
                    argWidget = inputWidget;
                    break;
                case T_Boolean:
                    inputWidget = gtk_check_button_new();
                    argWidget = inputWidget;
                    break;
                case T_Font:
                    {
                        inputWidget = gtk_hbox_new(FALSE, 0);
                        argWidget = gtk_font_button_new();
                        GtkWidget *button = gtk_button_new_with_label(_("Clear font setting"));
                        gtk_box_pack_start(GTK_BOX(inputWidget), argWidget, TRUE, TRUE, 0);
                        gtk_box_pack_start(GTK_BOX(inputWidget), button, FALSE, FALSE, 0);
                        gtk_font_button_set_use_size(GTK_FONT_BUTTON(argWidget), FALSE);
                        gtk_font_button_set_show_size(GTK_FONT_BUTTON(argWidget), FALSE);
                        gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(set_none_font_clicked), argWidget);
                    }
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
                        argWidget = inputWidget;
                    }
                    break;
                case T_File:
                case T_Char:
                case T_Image:
                case T_Hotkey:
                case T_String:
                    inputWidget = gtk_entry_new();
                    argWidget = inputWidget;
                    break;
            }
            gtk_box_pack_start(GTK_BOX(hbox), inputWidget, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
            ConfigBindValue(cfile, cgdesc->groupName, codesc->optionName, NULL, sync_filter, argWidget);
        }
    }
    gtk_widget_set_size_request(configNotebook, 500, -1);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(configNotebook), TRUE);

    return cvbox;
}

void sync_filter(ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg)
{
    ConfigOptionDesc *codesc = option->optionDesc;
    if (!codesc)
        return;
    if (sync == Raw2Value)
    {
        switch (codesc->type)
        {
            case T_Integer:
                {
                    int value = atoi(option->rawValue);
                    gtk_spin_button_set_value(GTK_SPIN_BUTTON(arg), value);
                }
                break;
            case T_Color:
                {
                    int r = 0,g = 0,b = 0;
                    char scolor[9];
                    sscanf(option->rawValue, "%d %d %d",&r,&g,&b);
                    r = RoundColor(r);
                    g = RoundColor(g);
                    b = RoundColor(b);
                    snprintf(scolor, 8 , "#%02X%02X%02X", r, g, b);
                    GdkColor color;
                    gdk_color_parse(scolor, &color);
                    gtk_color_button_set_color(GTK_COLOR_BUTTON(arg), &color);
                }
                break;
            case T_Boolean:
                {
                    gboolean bl;
                    if (strcmp(option->rawValue, "True") == 0)
                        bl = TRUE;
                    else
                        bl = FALSE;

                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arg), bl);
                }
                break;
            case T_Font:
                {
                    gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), option->rawValue);
                }
                break;
            case T_Enum:
                {
                    ConfigEnum* cenum = &codesc->configEnum;
                    int index = 0, i;
                    for (i = 0; i< cenum->enumCount; i++)
                    {
                        if ( strcmp(cenum->enumDesc[i], option->rawValue) == 0)
                        {
                            index = i;
                        }
                    }
                    gtk_combo_box_set_active(GTK_COMBO_BOX(arg), index);
                }
                break;
            case T_File:
            case T_Char:
            case T_Image:
            case T_Hotkey:
            case T_String:
                {
                    gtk_entry_set_text(GTK_ENTRY(arg), option->rawValue);
                }
                break;
        }
    }
    else
    {
        if (option->rawValue)
            free(option->rawValue);
        switch (codesc->type)
        {
            case T_Integer:
                {
                    int value;
                    value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(arg));
                    asprintf(&option->rawValue, "%d", value);
                }
                break;
            case T_Color:
                {
                    int r = 0,g = 0,b = 0;
                    GdkColor color;
                    gtk_color_button_get_color(GTK_COLOR_BUTTON(arg), &color);
                    r = color.red / 256;
                    g = color.green / 256;
                    b = color.blue / 256;
                    r = RoundColor(r);
                    g = RoundColor(g);
                    b = RoundColor(b);
                    asprintf(&option->rawValue, "%d %d %d",r,g,b);
                }
                break;
            case T_Boolean:
                {
                    gboolean bl;
                    bl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg));
                    if (bl)
                        option->rawValue = strdup("True");
                    else
                        option->rawValue = strdup("False");
                }
                break;
            case T_Font:
                {
                    const char *font  = gtk_font_button_get_font_name(GTK_FONT_BUTTON(arg));
                    PangoFontDescription *fontdesc = pango_font_description_from_string(font);
                    if (fontdesc)
                    {
                        const char *family = pango_font_description_get_family(fontdesc);
                        if (family)
                            option->rawValue = strdup(family);
                        else
                            option->rawValue = strdup("");
                        pango_font_description_free(fontdesc);
                    }
                    else
                        option->rawValue = strdup("");
                }
                break;
            case T_Enum:
                {
                    ConfigEnum* cenum = &codesc->configEnum;
                    int index = 0;
                    index = gtk_combo_box_get_active(GTK_COMBO_BOX(arg));
                    option->rawValue = strdup(cenum->enumDesc[index]);
                }
                break;
            case T_File:
            case T_Char:
            case T_Image:
            case T_Hotkey:
            case T_String:
                {
                    option->rawValue = strdup(gtk_entry_get_text(GTK_ENTRY(arg)));
                }
                break;
        }

    }
}
