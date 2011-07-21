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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>
#include "keygrab.h"
#include <fcitx-config/hotkey.h>

#define _(s) gettext(s)
//定义枚举类型，说明信号的名称和次序
enum {
    KEYGRAB_BUTTON_CHANGED,
    KEYGRAB_BUTTON_CURRENT_CHANGED,
    LAST_SIGNAL
};
static gint keygrab_button_signals[LAST_SIGNAL] = { 0 };
static void keygrab_button_init(KeyGrabButton *keygrab_button);
static void keygrab_button_class_init(KeyGrabButtonClass *keygrabbuttonclass);
static void changed(void);
static void current_changed(void);
static void begin_key_grab(KeyGrabButton* self, gpointer v);
static void end_key_grab(KeyGrabButton *self);
static GtkWidget* popup_new(GtkWidget* parent, const gchar* text, gboolean mouse);
static void on_key_press_event(GtkWidget *self, GdkEventKey *event, gpointer v);

//注册自定义控件
GtkType keygrab_button_get_type(void)
{
    static GtkType keygrab_button_type = 0;
    if(!keygrab_button_type)
    {
        GtkTypeInfo keygrab_button_info = {
            "KeyGrabButton",
            sizeof(KeyGrabButton),
            sizeof(KeyGrabButtonClass),
            (GtkClassInitFunc)keygrab_button_class_init,
            (GtkObjectInitFunc)keygrab_button_init,
            NULL,
            NULL
        };
        keygrab_button_type = gtk_type_unique(GTK_TYPE_BUTTON, &keygrab_button_info);
    }
    return keygrab_button_type;
}

static void keygrab_button_init(KeyGrabButton *keygrabbutton)
{
    keygrab_button_set_key(keygrabbutton, 0, 0);
    gtk_signal_connect(GTK_OBJECT(keygrabbutton), "clicked", GTK_SIGNAL_FUNC(begin_key_grab), NULL);
}

static void keygrab_button_class_init(KeyGrabButtonClass *keygrabbuttonclass)
{
    GtkObjectClass *object_class;
    object_class = (GtkObjectClass*)keygrabbuttonclass;
    keygrab_button_signals[KEYGRAB_BUTTON_CHANGED] = g_signal_new("changed",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET(KeyGrabButtonClass, changed),
                    NULL,NULL,
                    g_cclosure_marshal_VOID__VOID,
                    G_TYPE_NONE, 0, NULL);
    keygrab_button_signals[KEYGRAB_BUTTON_CURRENT_CHANGED] = g_signal_new("current-changed",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET(KeyGrabButtonClass, current_changed),
                    NULL,NULL,
                    g_cclosure_marshal_VOID__VOID,
                    G_TYPE_NONE, 0, NULL);
}
//创建新的自定义控件
GtkWidget* keygrab_button_new(void)
{
    return GTK_WIDGET(g_object_new(TYPE_KEYGRAB_BUTTON,0));
}


void begin_key_grab(KeyGrabButton* self, gpointer v)
{
    gtk_widget_add_events(GTK_WIDGET(self), GDK_KEY_PRESS_MASK);
    KeyGrabButton* b = KEYGRAB_BUTTON(self);
    b->popup = popup_new(GTK_WIDGET(self), _("Please press the new key combination"), FALSE);
    gtk_widget_show_all(b->popup);
    b->handler = gtk_signal_connect(GTK_OBJECT(b->popup), "key-press-event", GTK_SIGNAL_FUNC(on_key_press_event), b);

    while(gdk_keyboard_grab(gtk_widget_get_window(GTK_WIDGET(b->popup)), FALSE, GDK_CURRENT_TIME) != GDK_GRAB_SUCCESS)
           usleep(100); 
}

void end_key_grab(KeyGrabButton *self)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(self);
    gdk_keyboard_ungrab(gtk_get_current_event_time());
    gtk_signal_disconnect(b->popup, b->handler);
    gtk_widget_destroy(b->popup);
}

void on_key_press_event(GtkWidget *self, GdkEventKey *event, gpointer v)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(v);
    guint key;
    GdkModifierType mods = event->state & gtk_accelerator_get_default_mod_mask();

#if GTK_MINOR_VERSION < 22 
    if ((event->keyval == GDK_Escape
            || event->keyval == GDK_Return) && !mods)
#else
    if ((event->keyval == GDK_KEY_Escape
            || event->keyval == GDK_KEY_Return) && !mods)
#endif
    {
#if GTK_MINOR_VERSION < 22
        if (event->keyval == GDK_Escape)
#else
        if (event->keyval == GDK_KEY_Escape)
#endif
            gtk_signal_emit_by_name(GTK_OBJECT(b), "changed", b->key, b->mods);
        end_key_grab(b);
        keygrab_button_set_key(b, 0, 0);
        return;
    }

    key = gdk_keyval_to_upper(event->keyval);
#if GTK_MINOR_VERSION < 22
    if (key == GDK_ISO_Left_Tab)
        key = GDK_Tab;
#else
    if (key == GDK_KEY_ISO_Left_Tab)
        key = GDK_KEY_Tab;
#endif

    if (gtk_accelerator_valid(key, mods)
            || (key == GDK_Tab && mods))
    {
        keygrab_button_set_key(b, key, mods);
        end_key_grab(b);
        b->key = key;
        b->mods = mods;
        gtk_signal_emit_by_name(GTK_OBJECT(b), "changed", b->key, b->mods);
        return;
    }

    keygrab_button_set_key(b, key, mods);
}

void keygrab_button_set_key(KeyGrabButton* self, guint key, GdkModifierType mods)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(self);
    gchar *label;
    b->key = key;
    b->mods = mods;

    label = GetKeyString(key, mods);

    if (label == NULL || strlen(label) == 0)
    {
        gtk_button_set_label(GTK_BUTTON(b), _("Disabled"));
    }
    else
    {
        gchar* lb = label;
        gtk_button_set_label(GTK_BUTTON(b), lb);
    }

    if (label)
        free(label);
}

void keygrab_button_get_key(KeyGrabButton* self, guint* key, GdkModifierType* mods)
{
    if (key)
        *key = self->key;
    if (mods)
        *mods = self->mods;
}

GtkWidget* popup_new(GtkWidget* parent, const gchar* text, gboolean mouse)
{
    GtkWidget* w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(w), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_position(GTK_WINDOW(w), mouse && GTK_WIN_POS_MOUSE || GTK_WIN_POS_CENTER_ALWAYS);
    if (parent)
        gtk_window_set_transient_for(GTK_WINDOW(w), GTK_WINDOW(gtk_widget_get_toplevel(parent)));
    gtk_window_set_modal(GTK_WINDOW(w), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(w), TRUE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(w), TRUE);
    if (text)
    {
        GtkWidget* label = gtk_label_new(text);
        GtkWidget* align = gtk_alignment_new(0, 0, 1, 1);
        gtk_alignment_set_padding(GTK_ALIGNMENT(align), 20, 20, 20, 20);
        gtk_container_add(GTK_CONTAINER(align), label);
        gtk_container_add(GTK_CONTAINER(w), align);
    }

    return w;
}
