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

#include <dbus/dbus-glib.h>
#include <fcitx/module/ipc/ipc.h>
#include <string.h>
#include <stdio.h>
#include "im.h"

#define TYPE_IM \
    dbus_g_type_get_struct("GValueArray", G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INVALID)

#define TYPE_ARRAY_IM \
    dbus_g_type_get_collection("GPtrArray", TYPE_IM)

static void _fcitx_inputmethod_item_foreach_cb(gpointer       data,
        gpointer       user_data);

static FcitxIMItem*
_value_to_item (const GValue *value)
{
    const gchar *name, *unique_name, *langcode;
    gboolean enable;
    GType type;

    type = G_VALUE_TYPE (value);
    g_assert(dbus_g_type_is_struct (type));
    g_assert(4 == dbus_g_type_get_struct_size (type));
    g_assert(G_TYPE_STRING == dbus_g_type_get_struct_member_type (type, 0));
    g_assert(G_TYPE_STRING == dbus_g_type_get_struct_member_type (type, 1));
    g_assert(G_TYPE_STRING == dbus_g_type_get_struct_member_type (type, 2));
    g_assert(G_TYPE_BOOLEAN == dbus_g_type_get_struct_member_type (type, 3));

    GValue cvalue = { 0, };

    FcitxIMItem* item = g_malloc0(sizeof(FcitxIMItem));

    g_value_init (&cvalue, dbus_g_type_get_struct_member_type (type, 0));
    dbus_g_type_struct_get_member (value, 0, &cvalue);
    name = g_value_get_string(&cvalue);
    item->name = strdup(name);
    g_value_unset (&cvalue);

    g_value_init (&cvalue, dbus_g_type_get_struct_member_type (type, 1));
    dbus_g_type_struct_get_member (value, 1, &cvalue);
    unique_name = g_value_get_string(&cvalue);
    item->unique_name = strdup(unique_name);
    g_value_unset (&cvalue);

    g_value_init (&cvalue, dbus_g_type_get_struct_member_type (type, 2));
    dbus_g_type_struct_get_member (value, 2, &cvalue);
    langcode = g_value_get_string(&cvalue);
    item->langcode = strdup(langcode);
    g_value_unset (&cvalue);

    g_value_init (&cvalue, dbus_g_type_get_struct_member_type (type, 3));
    dbus_g_type_struct_get_member (value, 3, &cvalue);
    enable = g_value_get_boolean(&cvalue);
    item->enable = enable;
    g_value_unset (&cvalue);
    return item;
}

static void
_item_to_value (FcitxIMItem* item, GValue *value)
{
    g_value_init (value, TYPE_IM);
    GValueArray *va = g_value_array_new (4);
    g_value_array_append (va, NULL);
    g_value_init(&va->values[0], G_TYPE_STRING);
    g_value_set_string(&va->values[0], item->name);
    g_value_array_append (va, NULL);
    g_value_init(&va->values[1], G_TYPE_STRING);
    g_value_set_string(&va->values[1], item->unique_name);
    g_value_array_append (va, NULL);
    g_value_init(&va->values[2], G_TYPE_STRING);
    g_value_set_string(&va->values[2], item->langcode);
    g_value_array_append (va, NULL);
    g_value_init(&va->values[3], G_TYPE_BOOLEAN);
    g_value_set_boolean(&va->values[3], item->enable);
    g_value_take_boxed (value, va);
}

static void
_collection_iterator (const GValue *value,
                      gpointer user_data)
{
    GPtrArray *children = user_data;

    g_ptr_array_add (children, _value_to_item (value));
}

void fcitx_inputmethod_set_imlist(DBusGProxy* proxy, GPtrArray* array)
{
    GValue value = {0,};
    g_value_init(&value, TYPE_ARRAY_IM);
    g_value_take_boxed (&value, dbus_g_type_specialized_construct (
            G_VALUE_TYPE (&value)));
    DBusGTypeSpecializedAppendContext ctx;

    dbus_g_type_specialized_init_append (&value, &ctx);
    g_ptr_array_foreach(array, _fcitx_inputmethod_item_foreach_cb, &ctx);
    dbus_g_type_specialized_collection_end_append (&ctx);
    dbus_g_proxy_call_no_reply(proxy, "Set", G_TYPE_STRING, FCITX_IM_DBUS_INTERFACE, G_TYPE_STRING, "IMList", G_TYPE_VALUE, &value, G_TYPE_INVALID);
    g_value_unset(&value);
}

GPtrArray* fcitx_inputmethod_get_imlist(DBusGProxy* proxy)
{
    GPtrArray *array = NULL;
    GValue value = { 0, };

    GError* error = NULL;
    dbus_g_proxy_call(proxy, "Get", &error, G_TYPE_STRING, FCITX_IM_DBUS_INTERFACE, G_TYPE_STRING, "IMList", G_TYPE_INVALID, G_TYPE_VALUE, &value, G_TYPE_INVALID);

    if (error) {
        g_warning("%s", error->message);
        g_error_free(error);
        return NULL;
    }

    array = g_ptr_array_new();
    GType type = G_VALUE_TYPE (&value);

    if (dbus_g_type_is_collection (type))
    {
        dbus_g_type_collection_value_iterate (&value, _collection_iterator,
                                              array);
    }
    g_value_unset(&value);

    return array;
}

void fcitx_inputmethod_item_free(gpointer data)
{
    FcitxIMItem* item = data;
    g_free(item->name);
    g_free(item->unique_name);
    g_free(item->langcode);
    g_free(data);
}

void _fcitx_inputmethod_item_foreach_cb(gpointer       data,
                                        gpointer       user_data)
{
    FcitxIMItem* item = data;
    DBusGTypeSpecializedAppendContext* ctx = user_data;

    GValue v = { 0 };
    _item_to_value(item, &v);
    dbus_g_type_specialized_collection_append (ctx, &v);
}
