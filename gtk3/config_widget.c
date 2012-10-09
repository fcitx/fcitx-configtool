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
#include <ctype.h>

#include <fcitx-config/fcitx-config.h>
#include <fcitx-utils/uthash.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>

#include "config_widget.h"
#include "keygrab.h"
#include "sub_config_widget.h"
#include "configdesc.h"
#include "dummy_config.h"

#define _(s) gettext(s)
#define D_(d, x) dgettext (d, x)
#define RoundColor(c) ((c)>=0?((c)<=255?c:255):0)

G_DEFINE_TYPE(FcitxConfigWidget, fcitx_config_widget, GTK_TYPE_GRID)

typedef struct {
    int i;
    FcitxConfigWidget* widget;
    GtkWidget* grid;
    int j;
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


static void
fcitx_config_widget_check_can_use_simple(FcitxConfigWidget* self);

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
    gtk_orientable_set_orientation(GTK_ORIENTABLE(self), GTK_ORIENTATION_VERTICAL);
    self->argmap = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static void
sync_hotkey(KeyGrabButton* button, gpointer user_data)
{
    guint key1, key2;
    GdkModifierType mod1, mod2;
    keygrab_button_get_key(button, &key1, &mod1);
    keygrab_button_get_key(KEYGRAB_BUTTON(user_data), &key2, &mod2);

    if (key1 != key2 || mod1 != mod2)
        keygrab_button_set_key(KEYGRAB_BUTTON(user_data), key1, mod1);
}

static void
fcitx_config_widget_create_option_widget(
    FcitxConfigWidget *self,
    FcitxConfigGroupDesc* cgdesc,
    FcitxConfigOptionDesc* codesc,
    char** label,
    char** tooltip,
    GtkWidget** inputWidget,
    void** newarg)
{
    FcitxConfigFileDesc* cfdesc = self->cfdesc;
    FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
    void* oldarg = NULL;
    void* argument = NULL;
    char* name = g_strdup_printf("%s/%s", cgdesc->groupName, codesc->optionName);
    oldarg = g_hash_table_lookup(self->argmap, name);

    if (codesc->desc && strlen(codesc->desc) != 0) {
        *label = strdup(D_(cfdesc->domain, codesc->desc));
    } else {
        *label = strdup(D_(cfdesc->domain, codesc->optionName));
    }

    if (codesc2->longDesc && codesc2->longDesc[0]) {
        *tooltip = strdup(D_(cfdesc->domain, codesc2->longDesc));
    }

    switch (codesc->type) {
    case T_Integer:
        *inputWidget = gtk_spin_button_new_with_range(
            codesc2->constrain.integerConstrain.min,
            codesc2->constrain.integerConstrain.max,
            1.0);
        g_object_set(*inputWidget, "hexpand", TRUE, NULL);
        if (oldarg) {
            g_object_bind_property(*inputWidget, "value", oldarg, "value", G_BINDING_BIDIRECTIONAL);
        }
        else
            argument = *inputWidget;
        break;
    case T_Color:
        *inputWidget = gtk_color_button_new();
        g_object_set(*inputWidget, "hexpand", TRUE, NULL);
        if (oldarg) {
            g_object_bind_property(*inputWidget, "color", oldarg, "color", G_BINDING_BIDIRECTIONAL);
        }
        else
            argument = *inputWidget;
        break;
    case T_Boolean:
        *inputWidget = gtk_check_button_new();
        g_object_set(*inputWidget, "hexpand", TRUE, NULL);
        if (oldarg) {
            g_object_bind_property(*inputWidget, "active", oldarg, "active", G_BINDING_BIDIRECTIONAL);
        }
        else
            argument = *inputWidget;
        break;
    case T_Font: {
        *inputWidget = gtk_grid_new();
        g_object_set(*inputWidget, "hexpand", TRUE, NULL);
        GtkWidget* arg = gtk_font_button_new();
        GtkWidget *button = gtk_button_new_with_label(_("Clear font setting"));
        g_object_set(arg, "hexpand", TRUE, NULL);
        gtk_grid_attach(GTK_GRID(*inputWidget), arg, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(*inputWidget), button, 1, 0, 2, 1);
        gtk_font_button_set_use_size(GTK_FONT_BUTTON(arg), FALSE);
        gtk_font_button_set_show_size(GTK_FONT_BUTTON(arg), FALSE);
        g_signal_connect(G_OBJECT(button), "clicked", (GCallback) set_none_font_clicked, arg);
        if (oldarg) {
            g_object_bind_property(arg, "font-name", oldarg, "font-name", G_BINDING_BIDIRECTIONAL);
        }
        else
            argument = arg;
    }
    break;
    case T_Enum: {
        int i;
        FcitxConfigEnum *e = &codesc->configEnum;
        *inputWidget = gtk_combo_box_text_new();
        for (i = 0; i < e->enumCount; i ++) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(*inputWidget), D_(cfdesc->domain, e->enumDesc[i]));
        }
        g_object_set(*inputWidget, "hexpand", TRUE, NULL);
        if (oldarg) {
            g_object_bind_property(*inputWidget, "active", oldarg, "active", G_BINDING_BIDIRECTIONAL);
        }
        else
            argument = *inputWidget;
    }
    break;
    case T_Hotkey: {
        GtkWidget *button[2];
        button[0] = keygrab_button_new();
        button[1] = keygrab_button_new();
        *inputWidget = gtk_grid_new();
        gtk_grid_attach(GTK_GRID(*inputWidget), button[0], 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(*inputWidget), button[1], 1, 0, 2, 1);
        g_object_set(G_OBJECT(button[0]), "hexpand", TRUE, NULL);
        g_object_set(G_OBJECT(button[1]), "hexpand", TRUE, NULL);
        if (oldarg) {
            GArray* array = oldarg;
            int j;
            for (j = 0; j < 2; j ++) {
                GtkWidget *oldbutton = g_array_index(array, GtkWidget*, j);
                g_signal_connect(oldbutton, "changed", (GCallback) sync_hotkey, button[j]);
                g_signal_connect(button[j], "changed", (GCallback) sync_hotkey, oldbutton);
            }
        }
        else {
            argument = g_array_new(FALSE, FALSE, sizeof(void*));
            g_array_append_val(argument, button[0]);
            g_array_append_val(argument, button[1]);
        }
    }
    break;
    case T_File:
    case T_Char:
    case T_String:
        *inputWidget = gtk_entry_new();
        g_object_set(*inputWidget, "hexpand", TRUE, NULL);
        if (oldarg) {
            g_object_bind_property(*inputWidget, "text", oldarg, "text", G_BINDING_BIDIRECTIONAL);
        }
        else
            argument = *inputWidget;
        break;
    default:
        break;
    }


    if (argument) {
        g_hash_table_insert(self->argmap, name, argument);
        *newarg = argument;
    }
    else
        g_free(name);
}

static void
fcitx_config_widget_check_can_use_simple(FcitxConfigWidget* self)
{
    int count = 0;
    int simpleCount = 0;
    if (self->cfdesc) {
        HASH_FOREACH(cgdesc, self->cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            if (cgdesc->optionsDesc == NULL)
                continue;
            else {
                HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                    FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
                    if (!codesc2->advance)
                        simpleCount++;
                    count ++;
                }
            }
        }
    }

    int subconfig_count = self->parser ? g_hash_table_size(self->parser->subconfigs) : 0;
    /* if option is quite few */
    if (count + subconfig_count <= 10) {
        self->fullUiType = CW_Simple;
    }
    else {
        self->fullUiType = CW_Full;
    }
    if (simpleCount + subconfig_count <= 10) {
        self->simpleUiType = CW_Simple;
    }
    else
        self->simpleUiType = CW_Full;

    if (count == simpleCount)
        self->simpleUiType = CW_NoShow;
}

static
GtkWidget*
fcitx_config_widget_create_simple_ui(FcitxConfigWidget* self, gboolean skipAdvance)
{
    FcitxConfigFileDesc* cfdesc = self->cfdesc;
    GtkWidget *configGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(configGrid), 3);
    int i = 0;
    if (cfdesc) {
        FcitxConfigGroupDesc *cgdesc = NULL;
        FcitxConfigOptionDesc *codesc = NULL;
        for (cgdesc = cfdesc->groupsDesc;
                cgdesc != NULL;
                cgdesc = (FcitxConfigGroupDesc*)cgdesc->hh.next) {
            codesc = cgdesc->optionsDesc;
            if (codesc == NULL)
                continue;
            else {
                int count = 0;
                HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                    FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
                    if (!skipAdvance || !codesc2->advance)
                        count++;
                }
                if (!count)
                    continue;
            }

            gchar* s = g_strdup_printf("<b>%s</b>", D_(cfdesc->domain, cgdesc->groupName));
            GtkWidget *plabel = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(plabel), s);
            g_free(s);
            g_object_set(plabel, "xalign", 0.0f, "yalign", 0.5f, NULL);
            gtk_grid_attach(GTK_GRID(configGrid), plabel, 0, i, 3, 1);
            i ++;

            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
                if (skipAdvance && codesc2->advance)
                    continue;
                GtkWidget *inputWidget = NULL;
                void *argument = NULL;
                char* s = NULL, *tooltip = NULL;
                fcitx_config_widget_create_option_widget(self, cgdesc, codesc, &s, &tooltip, &inputWidget, &argument);

                if (inputWidget) {
                    GtkWidget* label = gtk_label_new(s);
                    g_object_set(label, "xalign", 1.0f, NULL);
                    gtk_grid_attach(GTK_GRID(configGrid), label, 1, i, 1, 1);
                    gtk_grid_attach(GTK_GRID(configGrid), inputWidget, 2, i, 1, 1);
                    gtk_widget_set_tooltip_text(GTK_WIDGET(label),
                                                tooltip);
                    gtk_widget_set_tooltip_text(GTK_WIDGET(inputWidget),
                                                tooltip);
                    i++;
                    if (argument)
                        dummy_config_bind(self->config, cgdesc->groupName, codesc->optionName, sync_filter, argument);
                }
                g_free(s);
                g_free(tooltip);
            }
        }
    }

    if (self->parser) {
        GHashTable* subconfigs = self->parser->subconfigs;
        if (g_hash_table_size(subconfigs) != 0) {
            GtkWidget *plabel = gtk_label_new(NULL);
            gchar* markup = g_strdup_printf("<b>%s</b>", "Other");
            gtk_label_set_markup(GTK_LABEL(plabel), markup);
            g_free(markup);
            g_object_set(plabel, "xalign", 0.0f, NULL);
            gtk_grid_attach(GTK_GRID(configGrid), plabel, 0, i, 3, 1);
            i ++;

            HashForeachContext context;
            context.i = i;
            context.j = 1;
            context.grid = configGrid;
            context.widget = self;
            g_hash_table_foreach(subconfigs, hash_foreach_cb, &context);
            i = context.i;
        }
    }
    if (i >= 2) {

        GtkWidget *plabel = gtk_label_new(NULL);
        gtk_widget_set_size_request(plabel, 20, 20);
        gtk_grid_attach(GTK_GRID(configGrid), plabel, 0, 2, 1, 1);
        i ++;
    }
    gtk_widget_set_hexpand(configGrid, TRUE);
    gtk_widget_set_vexpand(configGrid, TRUE);
    return configGrid;
}

static
GtkWidget*
fcitx_config_widget_create_full_ui(FcitxConfigWidget* self)
{
    FcitxConfigFileDesc* cfdesc = self->cfdesc;
    GtkWidget *configNotebook = gtk_notebook_new();
    if (cfdesc) {
        FcitxConfigGroupDesc *cgdesc = NULL;
        FcitxConfigOptionDesc *codesc = NULL;
        for (cgdesc = cfdesc->groupsDesc;
                cgdesc != NULL;
                cgdesc = (FcitxConfigGroupDesc*)cgdesc->hh.next) {
            codesc = cgdesc->optionsDesc;
            if (codesc == NULL)
                continue;

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
            gtk_notebook_append_page(GTK_NOTEBOOK(configNotebook),
                                     scrollwnd,
                                     plabel);

            int i = 0;
            for (; codesc != NULL;
                    codesc = (FcitxConfigOptionDesc*)codesc->hh.next, i++) {
                GtkWidget *inputWidget = NULL;
                void *argument = NULL;
                char* s = NULL, *tooltip = NULL;
                fcitx_config_widget_create_option_widget(self, cgdesc, codesc, &s, &tooltip, &inputWidget, &argument);

                if (inputWidget) {
                    GtkWidget* label = gtk_label_new(s);
                    g_object_set(label, "xalign", 0.0f, NULL);
                    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);
                    gtk_grid_attach(GTK_GRID(grid), inputWidget, 1, i, 1, 1);
                    gtk_widget_set_tooltip_text(GTK_WIDGET(label),
                                                tooltip);
                    gtk_widget_set_tooltip_text(GTK_WIDGET(inputWidget),
                                                tooltip);
                    if (argument)
                        dummy_config_bind(self->config, cgdesc->groupName, codesc->optionName, sync_filter, argument);
                }
                g_free(s);
                g_free(tooltip);
            }
        }
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
            context.j = 0;
            context.grid = grid;
            context.widget = self;
            g_hash_table_foreach(subconfigs, hash_foreach_cb, &context);
        }
    }

    gtk_widget_set_size_request(configNotebook, 500, -1);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(configNotebook), TRUE);
    gtk_widget_set_hexpand(configNotebook, TRUE);
    gtk_widget_set_vexpand(configNotebook, TRUE);

    return configNotebook;
}

static void
_fcitx_config_widget_toggle_simple_full(GtkToggleButton* button, gpointer user_data)
{
    FcitxConfigWidget *self = user_data;
    if (gtk_toggle_button_get_active(button)) {
        if (gtk_widget_get_parent(self->simpleWidget))
            gtk_container_remove(GTK_CONTAINER(self), self->simpleWidget);
        gtk_grid_attach(GTK_GRID(self), self->fullWidget, 0, 0, 1, 1);
        gtk_widget_show_all(self->fullWidget);
    }
    else {
        if (gtk_widget_get_parent(self->fullWidget))
            gtk_container_remove(GTK_CONTAINER(self), self->fullWidget);
        gtk_grid_attach(GTK_GRID(self), self->simpleWidget, 0, 0, 1, 1);
        gtk_widget_show_all(self->simpleWidget);
    }
}

static void
fcitx_config_widget_setup_ui(FcitxConfigWidget *self)
{
    self->config = dummy_config_new(self->cfdesc);
    fcitx_config_widget_check_can_use_simple(self);

    if (self->cfdesc) {
        bindtextdomain(self->cfdesc->domain, LOCALEDIR);
        bind_textdomain_codeset(self->cfdesc->domain, "UTF-8");
        FILE *fp;
        fp = FcitxXDGGetFileWithPrefix(self->prefix, self->name, "r", NULL);
        dummy_config_load(self->config, fp);

        if (fp)
            fclose(fp);
    }
    if (self->simpleUiType != CW_NoShow) {
        if (self->simpleUiType == CW_Simple)
            self->simpleWidget = fcitx_config_widget_create_simple_ui(self, true);
        else
            self->simpleWidget = fcitx_config_widget_create_full_ui(self);
        g_object_ref(self->simpleWidget);
    }

    if (self->fullUiType != CW_NoShow) {
        if (self->fullUiType == CW_Simple)
            self->fullWidget = fcitx_config_widget_create_simple_ui(self, false);
        else
            self->fullWidget = fcitx_config_widget_create_full_ui(self);
        gtk_grid_attach(GTK_GRID(self), self->fullWidget, 0, 0, 1, 1);
        g_object_ref(self->fullWidget);
    }

    if (self->simpleWidget && self->fullWidget)
    {
        self->advanceCheckBox = gtk_check_button_new();
        gtk_grid_attach(GTK_GRID(self), self->advanceCheckBox, 0, 1, 1, 1);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->advanceCheckBox), FALSE);
        gtk_button_set_label(GTK_BUTTON(self->advanceCheckBox), _("Show Advance Option"));
        g_signal_connect(self->advanceCheckBox, "toggled", (GCallback) _fcitx_config_widget_toggle_simple_full, self);
        _fcitx_config_widget_toggle_simple_full(GTK_TOGGLE_BUTTON(self->advanceCheckBox), self);
    }

    if (self->config)
        dummy_config_sync(self->config);
}

FcitxConfigWidget*
fcitx_config_widget_new(FcitxConfigFileDesc* cfdesc, const gchar* prefix, const gchar* name, const gchar* subconfig)
{
    FcitxConfigWidget* widget =
        g_object_new(FCITX_TYPE_CONFIG_WIDGET,
                     "cfdesc", cfdesc,
                     "prefix", prefix,
                     "name", name,
                     "subconfig", subconfig,
                     NULL);
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
            int i = *(int*) value;
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(arg), i);
        }
        break;
        case T_Color: {
            int r = 0, g = 0, b = 0;
            FcitxConfigColor* rawcolor = (FcitxConfigColor*) value;
            r = RoundColor(rawcolor->r * 255);
            g = RoundColor(rawcolor->g * 255);
            b = RoundColor(rawcolor->b * 255);
            char scolor[10];
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
            boolean *bl = (boolean*) value;

            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arg), *bl);
        }
        break;
        case T_Font: {
            gtk_font_button_set_font_name(GTK_FONT_BUTTON(arg), *(char**) value);
        }
        break;
        case T_Enum: {
            int index = *(int*) value;
            gtk_combo_box_set_active(GTK_COMBO_BOX(arg), index);
        }
        break;
        case T_Hotkey: {
            FcitxHotkey* hotkey = (FcitxHotkey*) value;
            int j;
            GArray *array = (GArray*) arg;

            for (j = 0; j < 2; j ++) {
                GtkWidget *button = g_array_index(array, GtkWidget*, j);
                keygrab_button_set_key(KEYGRAB_BUTTON(button), hotkey[j].sym, hotkey[j].state);
            }
        }
        break;
        case T_File:
        case T_Char:
        case T_String: {
            gtk_entry_set_text(GTK_ENTRY(arg), *(char**) value);
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
            int* i = (int*) value;
            *i = gtk_spin_button_get_value(GTK_SPIN_BUTTON(arg));;
        }
        break;
        case T_Color: {
            GdkRGBA color;
#if GTK_CHECK_VERSION(3,3,0)
            gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(arg), &color);
#else
            gtk_color_button_get_rgba(GTK_COLOR_BUTTON(arg), &color);
#endif
            FcitxConfigColor* rawcolor = (FcitxConfigColor*) value;
            rawcolor->r = color.red;
            rawcolor->g = color.green;
            rawcolor->b = color.blue;
        }
        break;
        case T_Boolean: {
            boolean* bl = (boolean*) value;
            *bl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg));
        }
        break;
        case T_Font: {
            char** fontname = (char**) value;
            const char *font  = gtk_font_button_get_font_name(GTK_FONT_BUTTON(arg));
            UT_array* array = fcitx_utils_split_string(font, ' ');
            if (utarray_len(array) > 0) {
                char** s = (char**) utarray_back(array);
                char* p = *s;
                while (*p) {
                    if (!isdigit(*p))
                        break;
                    p++;
                }
                if (*p == '\0')
                    utarray_pop_back(array);
                fcitx_utils_string_swap(fontname, fcitx_utils_join_string_list(array, ' '));
            }
            else {
                fcitx_utils_string_swap(fontname, font);
            }
            fcitx_utils_free_string_list(array);
        }
        break;
        case T_Enum: {
            int* index = (int*) value;
            *index = gtk_combo_box_get_active(GTK_COMBO_BOX(arg));
        }
        break;
        case T_Hotkey: {
            GArray *array = (GArray*) arg;
            FcitxHotkey* hotkey = value;
            int j = 0;

            for (j = 0; j < 2 ; j ++) {
                GtkWidget *button = g_array_index(array, GtkWidget*, j);
                keygrab_button_get_key(KEYGRAB_BUTTON(button), &hotkey[j].sym, &hotkey[j].state);
                char* keystring = FcitxHotkeyGetKeyString(hotkey[j].sym, hotkey[j].state);
                fcitx_utils_string_swap(&hotkey[j].desc, keystring);
                fcitx_utils_free(keystring);
            }
        }
        break;
        case T_File:
        case T_Char:
        case T_String: {
            char** str = (char**) value;
            fcitx_utils_string_swap(str, gtk_entry_get_text(GTK_ENTRY(arg)));
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
        FcitxConfigResetConfigToDefaultValue(&config_widget->config->config);
        dummy_config_sync(config_widget->config);
    } else if (action == CONFIG_WIDGET_SAVE) {
        FILE* fp = FcitxXDGGetFileUserWithPrefix(config_widget->prefix, config_widget->name, "w", NULL);

        if (fp) {
            FcitxConfigSaveConfigFileFp(fp, &config_widget->config->config, config_widget->cfdesc);
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
    FcitxConfigWidget* self = FCITX_CONFIG_WIDGET(object);
    g_free(self->name);
    g_free(self->prefix);
    sub_config_parser_free(self->parser);
    if (self->simpleWidget)
        g_object_unref(self->simpleWidget);
    if (self->fullWidget)
        g_object_unref(self->fullWidget);
    dummy_config_free(self->config);
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
    g_object_set(G_OBJECT(label),"xalign", 1.0f,  "yalign", 0.0f, NULL);

    GtkWidget *inputWidget = GTK_WIDGET(fcitx_sub_config_widget_new(subconfig));

    gtk_grid_attach(GTK_GRID(context->grid), label, context->j, i, 1,  1);
    gtk_grid_attach(GTK_GRID(context->grid), inputWidget, context->j + 1, i, 1, 1);
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
