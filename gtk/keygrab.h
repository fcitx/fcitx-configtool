#ifndef __OUR_ITEM_H__
#define __OUR_ITEM_H__
#include <gtk/gtk.h>
//定义类型宏和转换宏
#define TYPE_KEYGRAB_BUTTON   (keygrab_button_get_type())
#define KEYGRAB_BUTTON(obj)   (GTK_CHECK_CAST(obj,TYPE_KEYGRAB_BUTTON,KeyGrabButton))
//定义实例结构和类结构
typedef struct _KeyGrabButton KeyGrabButton;
typedef struct _KeyGrabButtonClass KeyGrabButtonClass;
struct _KeyGrabButton {
    GtkButton parent; //父控件为横向盒状容器
    GtkWidget* popup;
    gulong handler;
    guint key;
    GdkModifierType mods;
};
struct _KeyGrabButtonClass {
    GtkButtonClass parent_class;
    void (*changed)(int, int);
    void (*current_changed)(int, int);
};

GtkType keygrab_button_get_type(void);
GtkWidget* keygrab_button_new(void);
gchar *accelerator_to_fcitx_hotkey(const gchar* str);
void keygrab_button_set_key(KeyGrabButton* self, guint key, GdkModifierType mods);
void keygrab_button_get_key(KeyGrabButton* self, guint* key, GdkModifierType* mods);
#endif //__OUR_ITEM_H__

