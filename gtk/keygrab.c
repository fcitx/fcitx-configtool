#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>
#include "keygrab.h"

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
static void set_label(KeyGrabButton* self, guint key, GdkModifierType mods);

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

    if ((event->keyval == GDK_KEY_Escape
            || event->keyval == GDK_KEY_Return) && !mods)
    {
        if (event->keyval == GDK_KEY_Escape)
            gtk_signal_emit_by_name(GTK_OBJECT(b), "changed", b->key, b->mods);
        end_key_grab(b);
        set_label(b, 0, 0);
        return;
    }

    key = gdk_keyval_to_lower(event->keyval);
    if (key == GDK_KEY_ISO_Left_Tab)
        key = GDK_KEY_Tab;

    if (gtk_accelerator_valid(key, mods)
            || (key == GDK_KEY_Tab && mods))
    {
        set_label(b, key, mods);
        end_key_grab(b);
        b->key = key;
        b->mods = mods;
        gtk_signal_emit_by_name(GTK_OBJECT(b), "changed", b->key, b->mods);
        return;
    }

    set_label(b, key, mods);
}

void set_label(KeyGrabButton* self, guint key, GdkModifierType mods)
{
    KeyGrabButton* b = KEYGRAB_BUTTON(self);
    gchar *label;
    if (b->label)
    {
        if (key != 0 && mods != 0)
            gtk_signal_emit_by_name(GTK_OBJECT(b), "current-changed", key, mods);
        gtk_button_set_label(GTK_BUTTON(b), b->label);
        return;
    }
    if (key == 0 && mods == 0)
    {
        key = b->key;
        mods = b->mods;
    }

    label = gtk_accelerator_get_label(key, mods);

    if (strlen(label) == 0)
        label = _("Disabled");
    gchar* lb = accelerator_to_fcitx_hotkey(label);
    gtk_button_set_label(GTK_BUTTON(b), lb);
    free(lb);
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

gchar *accelerator_to_fcitx_hotkey(const gchar* str)
{
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    if (strstr(str, "Ctrl+"))
        strcat(buffer,"CTRL_");
    if (strstr(str, "Alt+"))
        strcat(buffer,"ALT_");
    if (strstr(str, "Shift+"))
        strcat(buffer,"SHIFT_");
    size_t l = strlen(str);
    size_t i = strlen(str) - 1;
    while(i > 0 && str[i] != '+')
        i --;
    if (str[i] == '+')
        i ++;
    size_t lb = strlen(buffer);
    while(i < l && lb < sizeof(buffer) - 1)
    {
        buffer[lb] = toupper(str[i]);
        i ++;
        lb ++;
    }
    
    return strdup(buffer);
}

gchar *fcitx_hotkey_to_accelerator(const gchar* str)
{
}
