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
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <fcitx-utils/uthash.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <stdlib.h>
#include <stdio.h>

#include "configdesc.h"

typedef struct ConfigDescSet {
    char *filename;
    FcitxConfigFileDesc *cfdesc;
    UT_hash_handle hh;
} ConfigDescSet;

static ConfigDescSet* cdset = NULL;

FcitxConfigFileDesc *get_config_desc(char *filename)
{
    ConfigDescSet *desc = NULL;
    HASH_FIND_STR(cdset, filename, desc);
    if (!desc) {
        FILE * tmpfp = FcitxXDGGetFileWithPrefix("configdesc", filename, "r", NULL);
        if (tmpfp) {
            desc = malloc(sizeof(ConfigDescSet));
            memset(desc, 0 , sizeof(ConfigDescSet));
            desc->filename = strdup(filename);
            desc->cfdesc = FcitxConfigParseConfigFileDescFp(tmpfp);
            fclose(tmpfp);

            HASH_ADD_KEYPTR(hh, cdset, desc->filename, strlen(desc->filename), desc);
        } else
            return NULL;
    }

    return desc->cfdesc;
}
