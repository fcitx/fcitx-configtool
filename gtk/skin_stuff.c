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

#include <dirent.h>
#include <sys/stat.h>
#include <fcitx-utils/utarray.h>
#include "skin_stuff.h"
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

#include "config.h"

static UT_array* skinBuf = NULL;

UT_array* loadSkinDir()
{
    if (!skinBuf)
    {
        skinBuf = malloc(sizeof(UT_array));
        utarray_init(skinBuf, &ut_str_icd);
    }
    else
    {
        utarray_clear(skinBuf);
    }
    int i ; 
    DIR *dir;
    struct dirent *drt;
    struct stat fileStat;
    size_t len;
    char pathBuf[PATH_MAX];
    char **skinPath = GetXDGPath(&len, "XDG_CONFIG_HOME", ".config", "fcitx/skin" , DATADIR, "fcitx/skin" );
    for(i = 0; i< len; i++)
    {
        dir = opendir(skinPath[i]);
        if (dir == NULL)
            continue;

        while((drt = readdir(dir)) != NULL)
        {
            if (strcmp(drt->d_name , ".") == 0 || strcmp(drt->d_name, "..") == 0)
                continue;
            sprintf(pathBuf,"%s/%s",skinPath[i],drt->d_name);

            if( stat(pathBuf,&fileStat) == -1)
            {
                continue;
            }
            if ( fileStat.st_mode & S_IFDIR)
            {
                /* check duplicate name */
                int j = 0;
                for(;j<skinBuf->i;j++)
                {
                    char **name = (char**) utarray_eltptr(skinBuf, j);
                    if (strcmp(*name, drt->d_name) == 0)
                        break;
                }
                if (j == skinBuf->i)
                {
                    char *temp = drt->d_name;
                    utarray_push_back(skinBuf, &temp);
                }
            }
        }

        closedir(dir);
    }

    FreeXDGPath(skinPath);

	return skinBuf;
}


