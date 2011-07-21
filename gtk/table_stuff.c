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

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/utarray.h>
#include <fcitx-utils/uthash.h>

#include "table_stuff.h"
#include "config.h"

typedef struct StringHashSet {
    char *name;
    UT_hash_handle hh;
} StringHashSet;

static UT_array* tableBuf = NULL;

/*
 * 读取码表输入法的名称和文件路径
 */
UT_array* LoadTableInfo (void)
{
    if (!tableBuf)
    {
        tableBuf = malloc(sizeof(UT_array));
        utarray_init(tableBuf, &ut_str_icd);
    }
    else
    {
        utarray_clear(tableBuf);
    }

    char **tablePath;
    size_t len;
    char pathBuf[PATH_MAX];
    int i = 0;
    DIR *dir;
    struct dirent *drt;
    struct stat fileStat;

	StringHashSet* sset = NULL;

    tablePath = GetXDGPath(&len, "XDG_CONFIG_HOME", ".config", "fcitx/table" , DATADIR, "fcitx/table" );

    for(i = 0; i< len; i++)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s", tablePath[i]);
        pathBuf[sizeof(pathBuf) - 1] = '\0';

        dir = opendir(pathBuf);
        if (dir == NULL)
            continue;

		/* collect all *.conf files */
        while((drt = readdir(dir)) != NULL)
        {
            size_t nameLen = strlen(drt->d_name);
            if (nameLen <= strlen(".conf") )
                continue;
            memset(pathBuf,0,sizeof(pathBuf));

            if (strcmp(drt->d_name + nameLen -strlen(".conf"), ".conf") != 0)
                continue;
            snprintf(pathBuf, sizeof(pathBuf), "%s/%s", tablePath[i], drt->d_name );

            if (stat(pathBuf, &fileStat) == -1)
                continue;

            if (fileStat.st_mode & S_IFREG)
            {
				StringHashSet *string;
				HASH_FIND_STR(sset, drt->d_name, string);
				if (!string)
				{
					char *bStr = strdup(drt->d_name);
					string = malloc(sizeof(StringHashSet));
                    memset(string, 0, sizeof(StringHashSet));
					string->name = bStr;
					HASH_ADD_KEYPTR(hh, sset, string->name, strlen(string->name), string);
				}
            }
        }

        closedir(dir);
    }

    StringHashSet* string;
    for (string = sset;
         string != NULL;
         string = (StringHashSet*)string->hh.next)
    {
        char *temp = string->name;
        utarray_push_back(tableBuf, &temp);
    }
		
    FreeXDGPath(tablePath);

	StringHashSet *curStr;
	while(sset)
	{
		curStr = sset;
		HASH_DEL(sset, curStr);
		free(curStr->name);
        free(curStr);
	}

    return tableBuf;
}

