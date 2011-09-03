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

#ifndef _SUB_CONFIG_PARSER_H
#define _SUB_CONFIG_PARSER_H
#include <glib/ghash.h>

typedef enum
{
    SC_None,
    SC_ConfigFile,
    SC_NativeFile
} SubConfigType;

typedef struct
{
    SubConfigType type;
    gchar* configdesc;
    gchar* nativepath;
    gchar** patternlist;
} FcitxSubConfigPattern;

typedef struct
{
    gchar* name;
    SubConfigType type;
    GList* filelist;
    gchar* nativepath;
    gchar* configdesc;
} FcitxSubConfig;

typedef struct
{
    GHashTable* subconfigs;
    gchar* domain;
} FcitxSubConfigParser;

FcitxSubConfigParser* sub_config_parser_new(const gchar* subconfig);
void sub_config_parser_free(FcitxSubConfigParser* parser);
FcitxSubConfig* sub_config_new(const gchar* name, FcitxSubConfigPattern* pattern);
void sub_config_free(FcitxSubConfig* subconfig);

#endif