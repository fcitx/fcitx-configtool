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

#include "im.h"
#include <dbus/dbus-glib.h>
#include <fcitx/module/ipc/ipc.h>
#include <string.h>
#include <stdio.h>

#define TYPE_IM \
    dbus_g_type_get_struct("GValueArray", G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INVALID)

#define TYPE_ARRAY_IM \
    dbus_g_type_get_collection("GPtrArray", TYPE_IM)

static void _fcitx_inputmethod_item_foreach_cb(gpointer       data,
        gpointer       user_data);

static GVariant *g_value_to_g_variant (const GValue *value);

static void
_collection_iterator (const GValue *value,
                      gpointer user_data)
{
    GPtrArray *children = user_data;

    g_ptr_array_add (children, g_value_to_g_variant (value));
}

static void
_map_iterator (const GValue *kvalue,
               const GValue *vvalue,
               gpointer user_data)
{
    GPtrArray *children = user_data;

    g_ptr_array_add (children,
                     g_variant_new_dict_entry (
                         g_value_to_g_variant (kvalue),
                         g_value_to_g_variant (vvalue)));
}

static GVariant *
g_value_to_g_variant (const GValue *value)
{
    GType type;

    type = G_VALUE_TYPE (value);

    if (dbus_g_type_is_collection (type))
    {
        GVariant *variant;
        GPtrArray *children;

        children = g_ptr_array_new ();
        dbus_g_type_collection_value_iterate (value, _collection_iterator,
                                              children);

        variant = g_variant_new_array (NULL, (GVariant **) children->pdata,
                                       children->len);
        g_ptr_array_free (children, TRUE);

        return variant;
    }
    else if (dbus_g_type_is_map (type))
    {
        GVariant *variant;
        GPtrArray *children;

        children = g_ptr_array_new ();
        dbus_g_type_map_value_iterate (value, _map_iterator, children);

        variant = g_variant_new_array (NULL, (GVariant **) children->pdata,
                                       children->len);
        g_ptr_array_free (children, TRUE);

        return variant;
    }
    else if (dbus_g_type_is_struct (type))
    {
        GVariant *variant, **children;
        guint size, i;

        size = dbus_g_type_get_struct_size (type);
        children = g_new0 (GVariant *, size);

        for (i = 0; i < size; i++)
        {
            GValue cvalue = { 0, };

            g_value_init (&cvalue, dbus_g_type_get_struct_member_type (type, i));
            dbus_g_type_struct_get_member (value, i, &cvalue);

            children[i] = g_value_to_g_variant (&cvalue);
            g_value_unset (&cvalue);
        }

        variant = g_variant_new_tuple (children, size);
        g_free (children);

        return variant;
    }
    else if (type == G_TYPE_BOOLEAN)
        return g_variant_new_boolean (g_value_get_boolean (value));
    else if (type == G_TYPE_UCHAR)
        return g_variant_new_byte (g_value_get_uchar (value));
    else if (type == G_TYPE_INT)
        return g_variant_new_int32 (g_value_get_int (value));
    else if (type == G_TYPE_UINT)
        return g_variant_new_uint32 (g_value_get_uint (value));
    else if (type == G_TYPE_INT64)
        return g_variant_new_int64 (g_value_get_int64 (value));
    else if (type == G_TYPE_UINT64)
        return g_variant_new_uint64 (g_value_get_uint64 (value));
    else if (type == G_TYPE_DOUBLE)
        return g_variant_new_double (g_value_get_double (value));
    else if (type == G_TYPE_STRING)
        return g_variant_new_string (g_value_get_string (value));
    else if (type == G_TYPE_STRV)
        return g_variant_new_strv (g_value_get_boxed (value), -1);
    else if (type == DBUS_TYPE_G_OBJECT_PATH)
        return g_variant_new_object_path (g_value_get_boxed (value));
    else if (type == G_TYPE_VALUE)
        return g_variant_new_variant (
                   g_value_to_g_variant (g_value_get_boxed (value)));
    else
    {
        g_error ("Unknown type: %s", g_type_name (type));
    }
}

void fcitx_inputmethod_set_imlist(DBusGProxy* proxy, GPtrArray* array)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sssb)"));
    g_ptr_array_foreach(array, _fcitx_inputmethod_item_foreach_cb, &builder);
    GVariant* variant = g_variant_builder_end(&builder);
    GValue value = {0,};
    dbus_g_value_parse_g_variant(variant, &value);
    dbus_g_proxy_call_no_reply(proxy, "Set", G_TYPE_STRING, FCITX_IM_DBUS_INTERFACE, G_TYPE_STRING, "IMList", G_TYPE_VALUE, &value, G_TYPE_INVALID);
    g_variant_unref(variant);
}

GPtrArray* fcitx_inputmethod_get_imlist(DBusGProxy* proxy)
{
    GPtrArray *array = NULL;
    GVariant* value = NULL;
    GValue result = { 0, };
    GVariantIter *iter;
    gchar *name, *unique_name, *langcode;
    gboolean enable;

    GError* error = NULL;
    dbus_g_proxy_call(proxy, "Get", &error, G_TYPE_STRING, FCITX_IM_DBUS_INTERFACE, G_TYPE_STRING, "IMList", G_TYPE_INVALID, G_TYPE_VALUE, &result, G_TYPE_INVALID);

    if (error) {
        g_warning("%s", error->message);
        g_error_free(error);
    } {
        value = g_value_to_g_variant(&result);
    }

    if (value) {
        array = g_ptr_array_new();
        g_variant_get(value, "a(sssb)", &iter);
        while (g_variant_iter_next(iter, "(sssb)", &name, &unique_name, &langcode, &enable, NULL)) {
            FcitxIMItem* item = g_malloc0(sizeof(FcitxIMItem));
            item->enable = enable;
            item->name = strdup(name);
            item->unique_name = strdup(unique_name);
            item->langcode = strdup(langcode);
            g_ptr_array_add(array, item);
            g_free(name);
            g_free(unique_name);
            g_free(langcode);
        }
        g_variant_iter_free(iter);

        g_variant_unref(value);
    }

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
    GVariantBuilder* builder = user_data;

    g_variant_builder_add(builder, "(sssb)", item->name, item->unique_name, item->langcode, item->enable);
}
