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

#ifndef KEYGRAB_H
#define KEYGRAB_H

#include <gtk/gtk.h>
//定义类型宏和转换宏
#define TYPE_KEYGRAB_BUTTON   (keygrab_button_get_type())
#define KEYGRAB_BUTTON(obj)   (G_TYPE_CHECK_INSTANCE_CAST(obj,TYPE_KEYGRAB_BUTTON,KeyGrabButton))
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

GType keygrab_button_get_type(void);
GtkWidget* keygrab_button_new(void);
gchar *accelerator_to_fcitx_hotkey(const gchar* str);
void keygrab_button_set_key(KeyGrabButton* self, guint key, GdkModifierType mods);
void keygrab_button_get_key(KeyGrabButton* self, guint* key, GdkModifierType* mods);

#endif
