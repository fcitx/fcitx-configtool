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