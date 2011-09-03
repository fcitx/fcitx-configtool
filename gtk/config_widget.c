/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "config.h"

#include <gtk/gtk.h>
#include <libintl.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcitx-config/fcitx-config.h>
#include <fcitx-utils/uthash.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>

#include "config_widget.h"
#include "keygrab.h"
#include "sub_config_widget.h"

#define _(s) gettext(s)
#define D_(d, x) dgettext (d, x)

G_DEFINE_TYPE (FcitxConfigWidget, fcitx_config_widget, GTK_TYPE_VBOX)

typedef struct {
    int i;
    FcitxConfigWidget* widget;
    GtkWidget* table;
} HashForeachContext;

enum
{
    PROP_0,

    PROP_CONFIG_DESC,
    PROP_PREFIX,
    PROP_NAME,
    PROP_SUBCONFIG
};

static void
fcitx_config_widget_set_property (GObject      *gobject,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec);


static void sync_filter(GenericConfig* gconfig, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg);

static void set_none_font_clicked(GtkWidget *button, gpointer arg);

static void hash_foreach_cb (gpointer       key,
                             gpointer       value,
                             gpointer       user_data);

static void
fcitx_config_widget_setup_ui (FcitxConfigWidget *self);

static void
fcitx_config_widget_finalize (GObject *object);

static void
fcitx_config_widget_class_init (FcitxConfigWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->set_property = fcitx_config_widget_set_property;
    gobject_class->finalize = fcitx_config_widget_finalize;
    g_object_class_install_property (gobject_class,
                                     PROP_CONFIG_DESC,
                                     g_param_spec_pointer ("cfdesc",
                                                           "Configuration Description",
                                                           "Configuration Description for this widget",
                                                           G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_PREFIX,
                                     g_param_spec_string ("prefix",
                                                          "Prefix of path",
                                                          "Prefix of configuration path",
                                                          NULL,
                                                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                                          "File name",
                                                          "File name of configuration file",
                                                          NULL,
                                                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_SUBCONFIG,
                                     g_param_spec_string ("subconfig",
                                                          "subconfig",
                                                          "subconfig",
                                                          NULL,
                                                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
fcitx_config_widget_init (FcitxConfigWidget *self)
{
}

static void
fcitx_config_widget_setup_ui (FcitxConfigWidget *self)
{
    ConfigFileDesc* cfdesc = self->cfdesc;
    GtkWidget *cvbox = GTK_WIDGET(self);
    GtkWidget *configNotebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(cvbox), configNotebook, TRUE, TRUE, 0);
    if (cfdesc)
    {
        bindtextdomain ( cfdesc->domain, LOCALEDIR );
        bind_textdomain_codeset ( cfdesc->domain, "UTF-8" );

        FILE *fp;
        fp = GetXDGFileWithPrefix ( self->prefix, self->name, "rt", NULL );
        self->gconfig.configFile = ParseConfigFileFp ( fp, cfdesc );

        GtkWidget *chbox = NULL;
        GtkWidget *saveButton = NULL;
        GtkWidget *resetButton = NULL;

        ConfigGroupDesc *cgdesc = NULL;
        ConfigOptionDesc *codesc = NULL;
        for (cgdesc = cfdesc->groupsDesc;
                cgdesc != NULL;
                cgdesc = (ConfigGroupDesc*)cgdesc->hh.next)
        {
            codesc = cgdesc->optionsDesc;
            if (codesc == NULL)
                continue;

            GtkWidget *table = gtk_table_new(2, HASH_COUNT(codesc), FALSE);
            GtkWidget *plabel = gtk_label_new(D_(cfdesc->domain, cgdesc->groupName));
            GtkWidget *scrollwnd = gtk_scrolled_window_new(NULL, NULL);
            GtkWidget *viewport = gtk_viewport_new(NULL, NULL);

            gtk_container_set_border_width(GTK_CONTAINER(table), 4);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            gtk_container_add(GTK_CONTAINER(scrollwnd), viewport);
            gtk_container_add(GTK_CONTAINER(viewport), table);
            gtk_notebook_append_page(GTK_NOTEBOOK(configNotebook),
                                     scrollwnd,
                                     plabel);

            int i = 0;
            for ( ; codesc != NULL;
                    codesc = (ConfigOptionDesc*)codesc->hh.next, i++)
            {
                const char *s;
                if (codesc->desc && strlen(codesc->desc) != 0)
                    s = D_(cfdesc->domain, codesc->desc);
                else
                    s = D_(cfdesc->domain, codesc->optionName);

                GtkWidget *inputWidget = NULL;
                void *argument = NULL;

                switch (codesc->type)
                {
                case T_Integer:
                    inputWidget = gtk_spin_button_new_with_range(-1.0, 10000.0, 1.0);
                    argument = inputWidget;
                    break;
                case T_Color:
                    inputWidget = gtk_color_button_new();
                    argument = inputWidget;
                    break;
                case T_Boolean:
                    inputWidget = gtk_check_button_new();
                    argument = inputWidget;
                    break;
                case T_Font:
                {
                    inputWidget = gtk_hbox_new(FALSE, 0);
                    argument = gtk_font_button_new();
                    GtkWidget *button = gtk_button_new_with_label(_("Clear font setting"));
                    gtk_box_pack_start(GTK_BOX(inputWidget), argument, TRUE, TRUE, 0);
                    gtk_box_pack_start(GTK_BOX(inputWidget), button, FALSE, FALSE, 0);
                    gtk_font_button_set_use_size(GTK_FONT_BUTTON(argument), FALSE);
                    gtk_font_button_set_show_size(GTK_FONT_BUTTON(argument), FALSE);
                    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(set_none_font_clicked), argument);
                }
                break;
                case T_Enum:
                {
                    int i;
                    ConfigEnum *e = &codesc->configEnum;
                    inputWidget = gtk_combo_box_new_text();
                    for (i = 0; i < e->enumCount; i ++)
                    {
                        gtk_combo_box_append_text(GTK_COMBO_BOX(inputWidget), D_(cfdesc->domain, e->enumDesc[i]));
                    }
                    argument = inputWidget;
                }
                break;
                case T_Hotkey:
                {
                    GtkWidget *button[2];
                    button[0] = keygrab_button_new();
                    button[1] = keygrab_button_new();
                    inputWidget = gtk_hbox_new(FALSE, 0);
                    gtk_box_pack_start(GTK_BOX(inputWidget), button[0], FALSE, TRUE, 0);
                    gtk_box_pack_start(GTK_BOX(inputWidget), button[1], FALSE, TRUE, 0);
                    argument = g_array_new(FALSE, FALSE, sizeof(void*));
                    g_array_append_val(argument, button[0]);
                    g_array_append_val(argument, button[1]);
                }
                break;
                case T_File:
                case T_Char:
                case T_String:
                    inputWidget = gtk_entry_new();
                    argument = inputWidget;
                    break;
                }

                if (inputWidget)
                {
                    GtkWidget* label = gtk_label_new(s);
                    g_object_set(label, "xalign", 0.0f, NULL);
                    gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i+1, GTK_FILL, GTK_SHRINK, 5, 5);
                    gtk_table_attach(GTK_TABLE(table), inputWidget, 1, 2, i, i+1, GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 4);
                    ConfigBindValue(self->gconfig.configFile, cgdesc->groupName, codesc->optionName, NULL, sync_filter, argument);
                }
            }
        }

        ConfigBindSync(&self->gconfig);
    }

    if (self->parser)
    {
        GHashTable* subconfigs = self->parser->subconfigs;
        if (g_hash_table_size(subconfigs) != 0)
        {
            GtkWidget *table = gtk_table_new(2, g_hash_table_size(subconfigs), FALSE);
            GtkWidget *plabel = gtk_label_new(_("Other"));
            GtkWidget *scrollwnd = gtk_scrolled_window_new(NULL, NULL);
            GtkWidget *viewport = gtk_viewport_new(NULL, NULL);

            gtk_container_set_border_width(GTK_CONTAINER(table), 4);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            gtk_container_add(GTK_CONTAINER(scrollwnd), viewport);
            gtk_container_add(GTK_CONTAINER(viewport), table);
            gtk_notebook_append_page(GTK_NOTEBOOK(configNotebook),
                                     scrollwnd,
                                     plabel);

            GList* p;
            HashForeachContext context;
            context.i = 0;
            context.table = table;
            context.widget = self;
            g_hash_table_foreach(subconfigs, hash_foreach_cb, &context);
        }
    }

    gtk_widget_set_size_request(configNotebook, 500, -1);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(configNotebook), TRUE);
}
FcitxConfigWidget*
fcitx_config_widget_new (ConfigFileDesc* cfdesc, const gchar* prefix, const gchar* name, const char* subconfig)
{
    FcitxConfigWidget* widget =
           g_object_new (FCITX_TYPE_CONFIG_WIDGET,
                         "cfdesc", cfdesc,
                         "prefix", prefix,
                         "name", name,
                         "subconfig", subconfig,
                         NULL
                        );
    fcitx_config_widget_setup_ui(widget);
    return widget;
}

static void
fcitx_config_widget_set_property (GObject      *gobject,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    FcitxConfigWidget* config_widget = FCITX_CONFIG_WIDGET(gobject);
    switch(prop_id)
    {
        case PROP_CONFIG_DESC:
            config_widget->cfdesc = g_value_get_pointer(value);
            break;
        case PROP_PREFIX:
            if (config_widget->prefix)
                g_free(config_widget->prefix);
            config_widget->prefix = g_strdup(g_value_get_string(value));
            break;
        case PROP_NAME:
            if (config_widget->name)
                g_free(config_widget->name);
            config_widget->name = g_strdup(g_value_get_string(value));
            break;
        case PROP_SUBCONFIG:
            if (config_widget->parser)
                sub_config_parser_free(config_widget->parser);
            config_widget->parser = sub_config_parser_new(g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
            break;
    }
}

static void set_none_font_clicked(GtkWidget *button, gpointer arg)
{
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), "");
}

void sync_filter(GenericConfig* gconfig, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg)
{
    ConfigOptionDesc *codesc = option->optionDesc;
    if (!codesc)
        return;
    if (sync == Raw2Value)
    {
        switch (codesc->type)
        {
            case T_I18NString:
                break;
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
        case T_Hotkey:
        {
            HOTKEYS hotkey[2];
            int j;
            SetHotKey(option->rawValue, hotkey);
            GArray *array = (GArray*) arg;

            for (j = 0; j < 2; j ++)
            {
                GtkWidget *button = g_array_index(array, GtkWidget*, j);
                keygrab_button_set_key(KEYGRAB_BUTTON(button), hotkey[j].sym, hotkey[j].state);
                if (hotkey[j].desc)
                    free(hotkey[j].desc);
            }
        }
        break;
        case T_File:
        case T_Char:
        case T_String:
        {
            gtk_entry_set_text(GTK_ENTRY(arg), option->rawValue);
        }
        break;
        }
    }
    else
    {
        if ( codesc->type != T_I18NString && option->rawValue )
        {
            free ( option->rawValue );
            option->rawValue = NULL;
        }
        switch (codesc->type)
        {
            case T_I18NString:
                break;
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
        case T_Hotkey:
        {
            GArray *array = (GArray*) arg;
            GtkWidget *button;
            guint key;
            GdkModifierType mods;
            char *strkey[2] = { NULL, NULL };
            int j = 0, k = 0;

            for (j = 0; j < 2 ; j ++)
            {
                button = g_array_index(array, GtkWidget*, j);
                keygrab_button_get_key(KEYGRAB_BUTTON(button), &key, &mods);
                strkey[k] = GetKeyString(key, mods);
                if (strkey[k])
                    k ++;
            }
            if (strkey[1])
                asprintf(&option->rawValue, "%s %s", strkey[0], strkey[1]);
            else if (strkey[0])
            {
                option->rawValue = strdup(strkey[0]);
            }
            else
                option->rawValue = strdup("");

            for (j = 0 ; j < k ; j ++)
                free(strkey[j]);

        }
        break;
        case T_File:
        case T_Char:
        case T_String:
        {
            option->rawValue = strdup(gtk_entry_get_text(GTK_ENTRY(arg)));
        }
        break;
        }

    }
}

void fcitx_config_widget_response(
    FcitxConfigWidget* config_widget,
    ConfigWidgetAction action
)
{
    if (!config_widget->cfdesc)
        return;

    if ( action == CONFIG_WIDGET_DEFAULT )
    {
        ResetConfigToDefaultValue ( &config_widget->gconfig );
        ConfigBindSync ( &config_widget->gconfig );
    }
    else if ( action == CONFIG_WIDGET_SAVE )
    {
        FILE* fp = GetXDGFileUserWithPrefix ( config_widget->prefix, config_widget->name, "wt", NULL );

        if ( fp )
        {
            SaveConfigFileFp ( fp, &config_widget->gconfig, config_widget->cfdesc );
            fclose ( fp );

            GError* error;
            gchar* argv[3];
            argv[0] = EXEC_PREFIX "/bin/fcitx-remote";
            argv[1] = "-r";
            argv[2] = 0;
            g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
        }
    }
}

void  fcitx_config_widget_finalize (GObject *object)
{
    FcitxConfigWidget* config_widget = FCITX_CONFIG_WIDGET(object);
    g_free(config_widget->name);
    g_free(config_widget->prefix);
    sub_config_parser_free(config_widget->parser);
    G_OBJECT_CLASS (fcitx_config_widget_parent_class)->finalize (object);
}

void hash_foreach_cb (gpointer       key,
                      gpointer       value,
                      gpointer       user_data)
{
    HashForeachContext* context = user_data;
    FcitxConfigWidget* widget = context->widget;

    FcitxSubConfigPattern* pattern =  value;
    FcitxSubConfig* subconfig = sub_config_new(key, pattern);

    if (subconfig == NULL)
        return;

    int i = context->i;

    GtkWidget* label = gtk_label_new(dgettext(widget->parser->domain, subconfig->name));
    g_object_set(label, "xalign", 0.0f, NULL);

    GtkWidget *inputWidget = GTK_WIDGET( fcitx_sub_config_widget_new(subconfig));

    gtk_table_attach(GTK_TABLE(context->table), label, 0, 1, i, i+1, GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(context->table), inputWidget, 1, 2, i, i+1, GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 4);
    context->i ++;
}