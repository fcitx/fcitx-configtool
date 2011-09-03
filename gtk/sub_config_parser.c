#include "config.h"

#include "sub_config_parser.h"
#include <glib/gstrfuncs.h>
#include <glib/gpattern.h>
#include <string.h>
#include <fcitx-config/xdg.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcitx-utils/log.h>

static void sub_config_pattern_free(void* pattern);
static GList* sub_config_pattern_get_filelist(FcitxSubConfigPattern* pattern);
static GList* get_files_by_pattern(const gchar* dirpath, FcitxSubConfigPattern* pattern, int index);

static SubConfigType parse_type(const gchar* str);

static SubConfigType parse_type(const gchar* str)
{
    if (strcmp(str, "native") == 0)
    {
        return SC_NativeFile;
    }
    if (strcmp(str, "configfile") == 0)
    {
        return SC_ConfigFile;
    }
    return SC_None;
}

FcitxSubConfigParser* sub_config_parser_new(const gchar* subconfig)
{
    if (subconfig == NULL)
        return NULL;

    FcitxSubConfigParser* parser = g_malloc0(sizeof(FcitxSubConfigParser));
    parser->subconfigs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, sub_config_pattern_free);
    gchar** strv = g_strsplit(subconfig, ",", 0);

    gchar** str;
    for (str = &strv[0]; *str != NULL; str++)
    {
        if (strchr(*str, ':') == NULL)
            continue;

        gchar** items = g_strsplit(*str, ":", 0);

        if (g_strv_length(items) < 2)
            goto end;
        if (strlen(items[0]) == 0)
            goto end;

        if (strcmp(items[1], "domain") == 0)
        {
            parser->domain = g_strdup(items[0]);
            goto end;
        }

        SubConfigType type = parse_type(items[1]);
        if (type == SC_None)
            goto end;
        if (g_hash_table_lookup (parser->subconfigs, items[0]) != NULL)
            continue;

        if (type == SC_ConfigFile)
        {
            if (g_strv_length(items) != 4)
                goto end;
            if (strlen(items[2]) == 0 || items[2][0] == '/')
                goto end;
        }
        else if (type == SC_NativeFile)
        {
            if (g_strv_length(items) != 3)
                goto end;
            if (strchr(items[2], '*') != NULL)
                goto end;
        }

        gchar** paths = g_strsplit(items[2], "/", 0);
        if (paths[0] == 0)
        {
            g_strfreev(paths);
            goto end;
        }
        gchar** path;
        for (path = &paths[0]; *path != NULL; path++)
        {
            if (strlen(*path) == 0)
                break;
            if (strcmp(*path, ".") == 0)
                break;
            if (strcmp(*path, "..") == 0)
                break;
        }
        if (*path != NULL)
        {
            g_strfreev(paths);
            goto end;
        }
        FcitxSubConfigPattern* pattern = g_malloc0(sizeof(FcitxSubConfigPattern));
        pattern->type = type;
        pattern->patternlist = paths;
        if (type == SC_ConfigFile)
            pattern->configdesc = g_strdup(items[3]);
        else if (type == SC_NativeFile)
            pattern->nativepath = g_strdup(items[2]);

        g_hash_table_insert(parser->subconfigs, g_strdup(items[0]), pattern);
end:
        g_strfreev(items);
    }
    g_strfreev(strv);
    if (g_hash_table_size(parser->subconfigs) == 0 || parser->domain == NULL)
    {
        sub_config_parser_free(parser);
        parser = NULL;
    }

    return parser;
}

void sub_config_parser_free(FcitxSubConfigParser* parser)
{
    if (parser == NULL)
        return;

    g_hash_table_destroy(parser->subconfigs);
    if (parser->domain)
        g_free(parser->domain);
}

void sub_config_pattern_free(void* data)
{
    FcitxSubConfigPattern* pattern = data;
    if (pattern->patternlist)
        g_strfreev(pattern->patternlist);

    if (pattern->configdesc)
        g_free(pattern->configdesc);

    if (pattern->nativepath)
        g_free(pattern->nativepath);

    g_free(pattern);
}

FcitxSubConfig* sub_config_new(const gchar* name, FcitxSubConfigPattern* pattern)
{
    if (pattern->type == SC_None)
        return NULL;

    FcitxSubConfig* subconfig = g_malloc0(sizeof(FcitxSubConfig));
    subconfig->type = pattern->type;
    subconfig->configdesc = g_strdup(pattern->configdesc);
    subconfig->nativepath = g_strdup(pattern->nativepath);
    subconfig->name = g_strdup(name);
    subconfig->filelist = sub_config_pattern_get_filelist(pattern);

    return subconfig;
}

void sub_config_free(FcitxSubConfig* subconfig)
{
    if (!subconfig)
        return;

    g_free(subconfig->configdesc);
    g_free(subconfig->nativepath);
    g_free(subconfig->name);
    g_list_free_full(subconfig->filelist, g_free);
    g_free(subconfig);
}

GList* sub_config_pattern_get_filelist(FcitxSubConfigPattern* pattern)
{
    size_t size, i;
    GList* result = NULL;
    char** xdgpath = GetXDGPath(&size, "XDG_CONFIG_HOME", ".config" , PACKAGE , DATADIR, PACKAGE);

    for (i = 0; i < size; i ++)
    {
        char* dirpath = realpath(xdgpath[i], NULL);

        GList* list = get_files_by_pattern(dirpath, pattern, 0), *l;

        for(l = g_list_first(list);
            l != NULL;
            l = l->next)
        {
            if (strncmp(dirpath, (gchar*) l->data, strlen(dirpath)) == 0)
            {
                gchar* filename = (gchar*) l->data;
                gchar* name = filename + strlen(dirpath);
                while (name[0] == '/')
                    name ++;
                result = g_list_append(result, g_strdup(name));
            }
        }
        g_list_free_full(list, g_free);

        free(dirpath);
    }

    FreeXDGPath(xdgpath);

    return result;
}

GList* get_files_by_pattern(const gchar* dirpath, FcitxSubConfigPattern* pattern, int index)
{
    GList* result = NULL;

    DIR* dir = opendir(dirpath);
    if (!dir)
        return result;

    gchar* filter = pattern->patternlist[index];

    struct dirent* drt;
    GPatternSpec * patternspec = g_pattern_spec_new(filter);
    while ((drt = readdir(dir)) != NULL)
    {
        if (strcmp(drt->d_name , ".") == 0 || strcmp(drt->d_name, "..") == 0)
            continue;

        if (!g_pattern_match_string(patternspec, drt->d_name))
            continue;

        if (pattern->patternlist[index + 1] == 0)
        {
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", dirpath, drt->d_name);
            path[PATH_MAX - 1] = '\0';
            struct stat statbuf;
            if (stat(path, &statbuf) == 0)
            {
                result = g_list_append(result, realpath(path, NULL));
            }
        }
        else
        {
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", dirpath, drt->d_name);
            path[PATH_MAX - 1] = '\0';
            GList* r = get_files_by_pattern(path, pattern, index + 1);
            result = g_list_concat(result, r);
        }
    }

    closedir(dir);
    g_pattern_spec_free(patternspec);
    return result;
}