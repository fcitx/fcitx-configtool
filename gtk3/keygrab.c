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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>
#include "keygrab.h"
#include <fcitx-config/hotkey.h>

#define _(s) gettext(s)

enum {
    KEYGRAB_BUTTON_CHANGED,
    LAST_SIGNAL
};
static gint keygrab_button_signals[LAST_SIGNAL] = { 0 };
static void keygrab_button_init(KeyGrabButton* self);
static void keygrab_button_class_init(KeyGrabButtonClass *keygrabbuttonclass);
static void begin_key_grab(KeyGrabButton* self, gpointer v);
static void end_key_grab(KeyGrabButton *self);
static GtkWidget* popup_new(GtkWidget* parent, const gchar* text, gboolean mouse);
static void on_key_release_event(GtkWidget *self, GdkEventKey *event, gpointer v);

G_DEFINE_TYPE(KeyGrabButton, keygrab_button, GTK_TYPE_BUTTON)

static void keygrab_button_init(KeyGrabButton *self)
{
    self->allow_modifier_only = FALSE;
    self->disallow_modifier_less = FALSE;
}

static void keygrab_button_class_init(KeyGrabButtonClass *keygrabbuttonclass)
{
    GObjectClass *object_class;
    object_class = (GObjectClass*)keygrabbuttonclass;
    keygrab_button_signals[KEYGRAB_BUTTON_CHANGED] = g_signal_new("changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL, NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0, NULL);
}
//创建新的自定义控件
GtkWidget* keygrab_button_new(void)
{
    KeyGrabButton *self = KEYGRAB_BUTTON(g_object_new(TYPE_KEYGRAB_BUTTON,
                                                      "label", _("Empty"),
                                                      NULL));
    self->key = 0;
    self->mods = 0;
    gtk_widget_set_size_request(GTK_WIDGET(self), 150, -1);

    g_signal_connect(G_OBJECT(self), "clicked", (GCallback) begin_key_grab, NULL);
    return GTK_WIDGET(self);
}


void begin_key_grab(KeyGrabButton* self, gpointer v)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(self);
    b->popup = popup_new(GTK_WIDGET(self), _("Please press the new key combination"), FALSE);
    gtk_widget_add_events(GTK_WIDGET(b->popup), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
    gtk_widget_show_all(b->popup);
    gtk_window_present(GTK_WINDOW(b->popup));
    b->handler = g_signal_connect(G_OBJECT(b->popup), "key-release-event", (GCallback)on_key_release_event, b);

    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(b->popup));
    GdkDisplay* display = gdk_window_get_display (window);
    GdkSeat* seat = gdk_display_get_default_seat (display);
    while (gdk_seat_grab (seat, window,
                     GDK_SEAT_CAPABILITY_ALL, FALSE,
                     NULL, NULL, NULL, NULL) != GDK_GRAB_SUCCESS)
        usleep(100);
}

void end_key_grab(KeyGrabButton *self)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(self);
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(b->popup));
    GdkDisplay* display = gdk_window_get_display (window);
    GdkSeat* seat = gdk_display_get_default_seat (display);
    gdk_seat_ungrab(seat);
    g_signal_handler_disconnect(b->popup, b->handler);
    gtk_widget_destroy(b->popup);
}

void on_key_release_event(GtkWidget *self, GdkEventKey *event, gpointer v)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(v);
    guint key;
    GdkModifierType mods = event->state & gtk_accelerator_get_default_mod_mask();

    if ((event->keyval == GDK_KEY_Escape
            || event->keyval == GDK_KEY_Return) && !mods) {
        end_key_grab(b);
        keygrab_button_set_key(b, 0, 0);
        return;
    }

    key = gdk_keyval_to_upper(event->keyval);
    if (key == GDK_KEY_ISO_Left_Tab)
        key = GDK_KEY_Tab;

    if (b->disallow_modifier_less && mods == 0) {
        return;
    }

    if (!b->allow_modifier_only &&
        (key == GDK_KEY_Shift_L
      || key == GDK_KEY_Shift_R
      || key == GDK_KEY_Control_L
      || key == GDK_KEY_Control_R
      || key == GDK_KEY_Alt_L
      || key == GDK_KEY_Alt_R
      || key == GDK_KEY_Super_L
      || key == GDK_KEY_Shift_R)) {
        return;
    }

    keygrab_button_set_key(b, key, mods);
    end_key_grab(b);
}

void keygrab_button_set_allow_modifier_only(KeyGrabButton* self, gboolean allow)
{
    self->allow_modifier_only = allow;
}

void keygrab_button_set_disallow_modifier_less(KeyGrabButton* self, gboolean disallow)
{
    self->disallow_modifier_less = disallow;
}

void keygrab_button_set_key(KeyGrabButton* self, guint key, GdkModifierType mods)
{
    if (mods & GDK_SUPER_MASK) {
        mods &= ~GDK_SUPER_MASK;
        mods |= FcitxKeyState_Super;
    }
    gchar *label;
    if (self->key != key || self->mods != mods) {
        self->key = key;
        self->mods = mods;
        g_signal_emit(G_OBJECT(self), keygrab_button_signals[KEYGRAB_BUTTON_CHANGED], 0);
    }
    else {
        return;
    }

    label = FcitxHotkeyGetReadableKeyString(key, mods);

    if (label == NULL || strlen(label) == 0) {
        gtk_button_set_label(GTK_BUTTON(self), _("Empty"));
    } else {
        gtk_button_set_label(GTK_BUTTON(self), label);
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
    gtk_window_set_position(GTK_WINDOW(w), mouse ? GTK_WIN_POS_MOUSE : GTK_WIN_POS_CENTER_ALWAYS);
    if (parent)
        gtk_window_set_transient_for(GTK_WINDOW(w), GTK_WINDOW(gtk_widget_get_toplevel(parent)));
    gtk_window_set_modal(GTK_WINDOW(w), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(w), TRUE);
    if (text) {
        GtkWidget* label = gtk_label_new(text);
        gtk_container_add(GTK_CONTAINER(w), label);
    }

    return w;
}
