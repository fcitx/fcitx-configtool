#include "uthash.h"
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <stdlib.h>
#include <stdio.h>

#include "configdesc.h"

typedef struct ConfigDescSet
{
    char *filename;
    ConfigFileDesc *cfdesc;
    UT_hash_handle hh;
} ConfigDescSet;

static ConfigDescSet* cdset = NULL;

ConfigFileDesc *get_config_desc(char *filename)
{
    ConfigDescSet *desc = NULL;
    HASH_FIND_STR(cdset, filename, desc);
    if (!desc)
    {
        FILE * tmpfp = GetXDGFileData(filename, "r", NULL);
        desc = malloc(sizeof(ConfigDescSet));
        memset(desc, 0 ,sizeof(ConfigDescSet));
        desc->filename = strdup(filename);
        desc->cfdesc = ParseConfigFileDescFp(tmpfp);
        fclose(tmpfp);

        HASH_ADD_KEYPTR(hh, cdset, desc->filename, strlen(desc->filename), desc);
    }

    return desc->cfdesc;
}
