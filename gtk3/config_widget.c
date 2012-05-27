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
#include "configdesc.h"

#define _(s) gettext(s)
#define D_(d, x) dgettext (d, x)
#define RoundColor(c) ((c)>=0?((c)<=255?c:255):0)

G_DEFINE_TYPE(FcitxConfigWidget, fcitx_config_widget, GTK_TYPE_BOX)

typedef struct {
    int i;
    FcitxConfigWidget* widget;
    GtkWidget* grid;
} HashForeachContext;

enum {
    PROP_0,

    PROP_CONFIG_DESC,
    PROP_PREFIX,
    PROP_NAME,
    PROP_SUBCONFIG
};

static void
fcitx_config_widget_set_property(GObject      *gobject,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec);


static void sync_filter(FcitxGenericConfig* gconfig, FcitxConfigGroup *group, FcitxConfigOption *option, void *value, FcitxConfigSync sync, void *arg);

static void set_none_font_clicked(GtkWidget *button, gpointer arg);

static void hash_foreach_cb(gpointer       key,
                            gpointer       value,
                            gpointer       user_data);

static void
fcitx_config_widget_setup_ui(FcitxConfigWidget *self);

static void
fcitx_config_widget_finalize(GObject *object);

static void
fcitx_config_widget_class_init(FcitxConfigWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->set_property = fcitx_config_widget_set_property;
    gobject_class->finalize = fcitx_config_widget_finalize;
    g_object_class_install_property(gobject_class,
                                    PROP_CONFIG_DESC,
                                    g_param_spec_pointer("cfdesc",
                                            "Configuration Description",
                                            "Configuration Description for this widget",
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class,
                                    PROP_PREFIX,
                                    g_param_spec_string("prefix",
                                            "Prefix of path",
                                            "Prefix of configuration path",
                                            NULL,
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class,
                                    PROP_NAME,
                                    g_param_spec_string("name",
                                            "File name",
                                            "File name of configuration file",
                                            NULL,
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class,
                                    PROP_SUBCONFIG,
                                    g_param_spec_string("subconfig",
                                            "subconfig",
                                            "subconfig",
                                            NULL,
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
fcitx_config_widget_init(FcitxConfigWidget *self)
{
}

static void
fcitx_config_widget_setup_ui(FcitxConfigWidget *self)
{
    FcitxConfigFileDesc* cfdesc = self->cfdesc;
    GtkWidget *cvbox = GTK_WIDGET(self);
    GtkWidget *configNotebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(cvbox), configNotebook, TRUE, TRUE, 0);
    if (cfdesc) {
        bindtextdomain(cfdesc->domain, LOCALEDIR);
        bind_textdomain_codeset(cfdesc->domain, "UTF-8");

        FILE *fp;
        fp = FcitxXDGGetFileWithPrefix(self->prefix, self->name, "r", NULL);
        self->gconfig.configFile = FcitxConfigParseConfigFileFp(fp, cfdesc);

        FcitxConfigGroupDesc *cgdesc = NULL;
        FcitxConfigOptionDesc *codesc = NULL;
        for (cgdesc = cfdesc->groupsDesc;
                cgdesc != NULL;
                cgdesc = (FcitxConfigGroupDesc*)cgdesc->hh.next) {
            codesc = cgdesc->optionsDesc;
            if (codesc == NULL)
                continue;

            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            GtkWidget *grid = gtk_grid_new();
            gtk_widget_set_margin_left(grid, 12);
            gtk_widget_set_margin_top(grid, 6);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
            GtkWidget *plabel = gtk_label_new(D_(cfdesc->domain, cgdesc->groupName));
            GtkWidget *scrollwnd = gtk_scrolled_window_new(NULL, NULL);
            g_object_set(G_OBJECT(scrollwnd), "shadow-type", GTK_SHADOW_NONE, NULL);

            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwnd), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
            gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollwnd), grid);
            gtk_box_pack_start(GTK_BOX(hbox), scrollwnd, TRUE, TRUE, 0);
            gtk_notebook_append_page(GTK_NOTEBOOK(configNotebook),
                                     hbox,
                                     plabel);

            int i = 0;
            for (; codesc != NULL;
                    codesc = (FcitxConfigOptionDesc*)codesc->hh.next, i++) {
                const char *s;
                if (codesc->desc && strlen(codesc->desc) != 0)
                    s = D_(cfdesc->domain, codesc->desc);
                else
                    s = D_(cfdesc->domain, codesc->optionName);

                GtkWidget *inputWidget = NULL;
                void *argument = NULL;

                switch (codesc->type) {
                case T_Integer:
                    inputWidget = gtk_spin_button_new_with_range(-10000.0, 10000.0, 1.0);
                    g_object_set(G_OBJECT(inputWidget), "hexpand", TRUE, NULL);
                    argument = inputWidget;
                    break;
                case T_Color:
                    inputWidget = gtk_color_button_new();
                    g_object_set(G_OBJECT(inputWidget), "hexpand", TRUE, NULL);
                    argument = inputWidget;
                    break;
                case T_Boolean:
                    inputWidget = gtk_check_button_new();
                    g_object_set(G_OBJECT(inputWidget), "hexpand", TRUE, NULL);
                    argument = inputWidget;
                    break;
                case T_Font: {
                    inputWidget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                    g_object_set(G_OBJECT(inputWidget), "hexpand", TRUE, NULL);
                    argument = gtk_font_button_new();
                    GtkWidget *button = gtk_button_new_with_label(_("Clear font setting"));
                    gtk_box_pack_start(GTK_BOX(inputWidget), argument, TRUE, TRUE, 0);
                    gtk_box_pack_start(GTK_BOX(inputWidget), button, FALSE, FALSE, 0);
                    gtk_font_button_set_use_size(GTK_FONT_BUTTON(argument), FALSE);
                    gtk_font_button_set_show_size(GTK_FONT_BUTTON(argument), FALSE);
                    g_signal_connect(G_OBJECT(button), "clicked", (GCallback) set_none_font_clicked, argument);
                }
                break;
                case T_Enum: {
                    int i;
                    FcitxConfigEnum *e = &codesc->configEnum;
                    inputWidget = gtk_combo_box_text_new();
                    for (i = 0; i < e->enumCount; i ++) {
                        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(inputWidget), D_(cfdesc->domain, e->enumDesc[i]));
                    }
                    g_object_set(G_OBJECT(inputWidget), "hexpand", TRUE, NULL);
                    argument = inputWidget;
                }
                break;
                case T_Hotkey: {
                    GtkWidget *button[2];
                    button[0] = keygrab_button_new();
                    button[1] = keygrab_button_new();
                    inputWidget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                    gtk_box_pack_start(GTK_BOX(inputWidget), button[0], FALSE, TRUE, 0);
                    gtk_box_pack_start(GTK_BOX(inputWidget), button[1], FALSE, TRUE, 0);
                    g_object_set(G_OBJECT(button[0]), "hexpand", TRUE, NULL);
                    g_object_set(G_OBJECT(button[1]), "hexpand", TRUE, NULL);
                    argument = g_array_new(FALSE, FALSE, sizeof(void*));
                    g_array_append_val(argument, button[0]);
                    g_array_append_val(argument, button[1]);
                }
                break;
                case T_File:
                case T_Char:
                case T_String:
                    inputWidget = gtk_entry_new();
                    g_object_set(G_OBJECT(inputWidget), "hexpand", TRUE, NULL);
                    argument = inputWidget;
                    break;
                default:
                    break;
                }

                if (inputWidget) {
                    GtkWidget* label = gtk_label_new(s);
                    g_object_set(label, "xalign", 0.0f, NULL);
                    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);
                    gtk_grid_attach(GTK_GRID(grid), inputWidget, 1, i, 1, 1);
                    FcitxConfigBindValue(self->gconfig.configFile, cgdesc->groupName, codesc->optionName, NULL, sync_filter, argument);
                }
            }
        }

        FcitxConfigBindSync(&self->gconfig);
    }

    if (self->parser) {
        GHashTable* subconfigs = self->parser->subconfigs;
        if (g_hash_table_size(subconfigs) != 0) {
            GtkWidget *grid = gtk_grid_new();
            GtkWidget *plabel = gtk_label_new(_("Other"));
            GtkWidget *scrollwnd = gtk_scrolled_window_new(NULL, NULL);
            GtkWidget *viewport = gtk_viewport_new(NULL, NULL);

            gtk_container_set_border_width(GTK_CONTAINER(grid), 4);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwnd), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            gtk_container_add(GTK_CONTAINER(scrollwnd), viewport);
            gtk_container_add(GTK_CONTAINER(viewport), grid);
            gtk_notebook_append_page(GTK_NOTEBOOK(configNotebook),
                                     scrollwnd,
                                     plabel);

            HashForeachContext context;
            context.i = 0;
            context.grid = grid;
            context.widget = self;
            g_hash_table_foreach(subconfigs, hash_foreach_cb, &context);
        }
    }

    gtk_widget_set_size_request(configNotebook, 500, -1);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(configNotebook), TRUE);
}
FcitxConfigWidget*
fcitx_config_widget_new(FcitxConfigFileDesc* cfdesc, const gchar* prefix, const gchar* name, const char* subconfig)
{
    FcitxConfigWidget* widget =
        g_object_new(FCITX_TYPE_CONFIG_WIDGET,
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
fcitx_config_widget_set_property(GObject      *gobject,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    FcitxConfigWidget* config_widget = FCITX_CONFIG_WIDGET(gobject);
    switch (prop_id) {
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
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}

static void set_none_font_clicked(GtkWidget *button, gpointer arg)
{
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), "");
}

void sync_filter(FcitxGenericConfig* gconfig, FcitxConfigGroup *group, FcitxConfigOption *option, void *value, FcitxConfigSync sync, void *arg)
{
    FcitxConfigOptionDesc *codesc = option->optionDesc;
    if (!codesc)
        return;
    if (sync == Raw2Value) {
        switch (codesc->type) {
        case T_I18NString:
            break;
        case T_Integer: {
            int value = atoi(option->rawValue);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(arg), value);
        }
        break;
        case T_Color: {
            int r = 0, g = 0, b = 0;
            char scolor[9];
            sscanf(option->rawValue, "%d %d %d", &r, &g, &b);
            r = RoundColor(r);
            g = RoundColor(g);
            b = RoundColor(b);
            snprintf(scolor, 8 , "#%02X%02X%02X", r, g, b);
            GdkRGBA color;
            gdk_rgba_parse(&color, scolor);
#if GTK_CHECK_VERSION(3,3,0)
            gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(arg), &color);
#else
            gtk_color_button_set_rgba(GTK_COLOR_BUTTON(arg), &color);
#endif
        }
        break;
        case T_Boolean: {
            gboolean bl;
            if (strcmp(option->rawValue, "True") == 0)
                bl = TRUE;
            else
                bl = FALSE;

            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arg), bl);
        }
        break;
        case T_Font: {
            gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), option->rawValue);
        }
        break;
        case T_Enum: {
            FcitxConfigEnum* cenum = &codesc->configEnum;
            int index = 0, i;
            for (i = 0; i < cenum->enumCount; i++) {
                if (strcmp(cenum->enumDesc[i], option->rawValue) == 0) {
                    index = i;
                }
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(arg), index);
        }
        break;
        case T_Hotkey: {
            FcitxHotkey hotkey[2];
            int j;
            FcitxHotkeySetKey(option->rawValue, hotkey);
            GArray *array = (GArray*) arg;

            for (j = 0; j < 2; j ++) {
                GtkWidget *button = g_array_index(array, GtkWidget*, j);
                keygrab_button_set_key(KEYGRAB_BUTTON(button), hotkey[j].sym, hotkey[j].state);
                if (hotkey[j].desc)
                    free(hotkey[j].desc);
            }
        }
        break;
        case T_File:
        case T_Char:
        case T_String: {
            gtk_entry_set_text(GTK_ENTRY(arg), option->rawValue);
        }
        break;
        }
    } else {
        if (codesc->type != T_I18NString && option->rawValue) {
            free(option->rawValue);
            option->rawValue = NULL;
        }
        switch (codesc->type) {
        case T_I18NString:
            break;
        case T_Integer: {
            int value;
            value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(arg));
            option->rawValue = g_strdup_printf("%d", value);
        }
        break;
        case T_Color: {
            int r = 0, g = 0, b = 0;
            GdkRGBA color;
#if GTK_CHECK_VERSION(3,3,0)
            gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(arg), &color);
#else
            gtk_color_button_get_rgba(GTK_COLOR_BUTTON(arg), &color);
#endif
            r = color.red * 256;
            g = color.green * 256;
            b = color.blue * 256;
            r = RoundColor(r);
            g = RoundColor(g);
            b = RoundColor(b);
            option->rawValue = g_strdup_printf("%d %d %d", r, g, b);
        }
        break;
        case T_Boolean: {
            gboolean bl;
            bl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg));
            if (bl)
                option->rawValue = strdup("True");
            else
                option->rawValue = strdup("False");
        }
        break;
        case T_Font: {
            const char *font  = gtk_font_button_get_font_name(GTK_FONT_BUTTON(arg));
            PangoFontDescription *fontdesc = pango_font_description_from_string(font);
            if (fontdesc) {
                const char *family = pango_font_description_get_family(fontdesc);
                if (family)
                    option->rawValue = strdup(family);
                else
                    option->rawValue = strdup("");
                pango_font_description_free(fontdesc);
            } else
                option->rawValue = strdup("");
        }
        break;
        case T_Enum: {
            FcitxConfigEnum* cenum = &codesc->configEnum;
            int index = 0;
            index = gtk_combo_box_get_active(GTK_COMBO_BOX(arg));
            option->rawValue = strdup(cenum->enumDesc[index]);
        }
        break;
        case T_Hotkey: {
            GArray *array = (GArray*) arg;
            GtkWidget *button;
            guint key;
            GdkModifierType mods;
            char *strkey[2] = { NULL, NULL };
            int j = 0, k = 0;

            for (j = 0; j < 2 ; j ++) {
                button = g_array_index(array, GtkWidget*, j);
                keygrab_button_get_key(KEYGRAB_BUTTON(button), &key, &mods);
                strkey[k] = FcitxHotkeyGetKeyString(key, mods);
                if (strkey[k])
                    k ++;
            }
            if (strkey[1])
                option->rawValue = g_strdup_printf("%s %s", strkey[0], strkey[1]);
            else if (strkey[0]) {
                option->rawValue = strdup(strkey[0]);
            } else
                option->rawValue = strdup("");

            for (j = 0 ; j < k ; j ++)
                free(strkey[j]);

        }
        break;
        case T_File:
        case T_Char:
        case T_String: {
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

    if (action == CONFIG_WIDGET_DEFAULT) {
        FcitxConfigResetConfigToDefaultValue(&config_widget->gconfig);
        FcitxConfigBindSync(&config_widget->gconfig);
    } else if (action == CONFIG_WIDGET_SAVE) {
        FILE* fp = FcitxXDGGetFileUserWithPrefix(config_widget->prefix, config_widget->name, "w", NULL);

        if (fp) {
            FcitxConfigSaveConfigFileFp(fp, &config_widget->gconfig, config_widget->cfdesc);
            fclose(fp);

            GError* error;
            gchar* argv[3];
            argv[0] = EXEC_PREFIX "/bin/fcitx-remote";
            argv[1] = "-r";
            argv[2] = 0;
            g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
        }
    }
}

void  fcitx_config_widget_finalize(GObject *object)
{
    FcitxConfigWidget* config_widget = FCITX_CONFIG_WIDGET(object);
    g_free(config_widget->name);
    g_free(config_widget->prefix);
    sub_config_parser_free(config_widget->parser);
    G_OBJECT_CLASS(fcitx_config_widget_parent_class)->finalize(object);
}

void hash_foreach_cb(gpointer       key,
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

    GtkWidget *inputWidget = GTK_WIDGET(fcitx_sub_config_widget_new(subconfig));

    gtk_grid_attach(GTK_GRID(context->grid), label, 0, i, 1,  1);
    gtk_grid_attach(GTK_GRID(context->grid), inputWidget, 1, i, 1, 1);
    context->i ++;
}

gboolean fcitx_config_widget_response_cb(GtkDialog *dialog,
        gint response,
        gpointer user_data)
{
    if (response == GTK_RESPONSE_OK) {
        FcitxConfigWidget* config_widget = (FcitxConfigWidget*) user_data;
        fcitx_config_widget_response(config_widget, CONFIG_WIDGET_SAVE);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return FALSE;
}

GtkWidget* fcitx_config_dialog_new(FcitxAddon* addon, GtkWindow* parent)
{
    gchar* config_desc_name = g_strdup_printf("%s.desc", addon->name);
    FcitxConfigFileDesc* cfdesc = get_config_desc(config_desc_name);
    g_free(config_desc_name);
    gboolean configurable = (gboolean)(cfdesc != NULL || strlen(addon->subconfig) != 0);
    if (!configurable) {
        return NULL;
    }
    GtkWidget* dialog = gtk_dialog_new_with_buttons(addon->generalname,
                                                    parent,
                                                    GTK_DIALOG_MODAL,
                                                    GTK_STOCK_CANCEL,
                                                    GTK_RESPONSE_CANCEL,
                                                    GTK_STOCK_OK,
                                                    GTK_RESPONSE_OK,
                                                    NULL
                                                );

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                         GTK_RESPONSE_OK,
                                         GTK_RESPONSE_CANCEL,
                                         -1);
    gchar* config_file_name = g_strdup_printf("%s.config", addon->name);
    FcitxConfigWidget* config_widget = fcitx_config_widget_new(cfdesc, "conf", config_file_name, addon->subconfig);
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(config_widget), TRUE, TRUE, 0);
    g_free(config_file_name);
    gtk_widget_set_size_request(GTK_WIDGET(config_widget), -1, 400);

    g_signal_connect(dialog, "response",
                    G_CALLBACK(fcitx_config_widget_response_cb),
                    config_widget);
    return dialog;
}
