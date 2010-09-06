#include <dirent.h>
#include <sys/stat.h>
#include "utarray.h"
#include "skin_stuff.h"
#include "xdg.h"

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


