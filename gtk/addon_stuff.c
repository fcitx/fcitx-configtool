#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcitx-config/xdg.h>

#include "utarray.h"
#include "uthash.h"
#include "addon_stuff.h"

typedef struct StringHashSet {
    char *name;
    UT_hash_handle hh;
} StringHashSet;

static UT_array* addonBuf = NULL;

/*
 * 读取码表输入法的名称和文件路径
 */
UT_array* LoadAddonInfo (void)
{
    if (!addonBuf)
    {
        addonBuf = malloc(sizeof(UT_array));
        utarray_init(addonBuf, &ut_str_icd);
    }
    else
    {
        utarray_clear(addonBuf);
    }

    char **addonPath;
    size_t len;
    char pathBuf[PATH_MAX];
    int i = 0;
    DIR *dir;
    struct dirent *drt;
    struct stat fileStat;

    StringHashSet* sset = NULL;

    addonPath = GetXDGPath(&len, "XDG_CONFIG_HOME", ".config", "fcitx/addon" , DATADIR, "fcitx/data/addon" );

    for(i = 0; i< len; i++)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s", addonPath[i]);
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
            snprintf(pathBuf, sizeof(pathBuf), "%s/%s", addonPath[i], drt->d_name );

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
        utarray_push_back(addonBuf, &temp);
    }

    FreeXDGPath(addonPath);

    StringHashSet *curStr;
    while(sset)
    {
        curStr = sset;
        HASH_DEL(sset, curStr);
        free(curStr->name);
        free(curStr);
    }

    return addonBuf;
}

