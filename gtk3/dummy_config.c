#include "dummy_config.h"
#include <fcitx-config/hotkey.h>

DummyConfig* dummy_config_new(FcitxConfigFileDesc* cfdesc)
{
    DummyConfig* self = g_new0(DummyConfig, 1);
    self->cfdesc = cfdesc;
    self->config.configFile = NULL;
    self->dummy_value = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, (GDestroyNotify) free);
    /* malloc necessary value */
    HASH_FOREACH(cgdesc, self->cfdesc->groupsDesc, FcitxConfigGroupDesc) {
        HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
            gchar* name = g_strdup_printf("%s/%s", cgdesc->groupName, codesc->optionName);
            void* value = g_hash_table_lookup(self->dummy_value, name);
            if (value) {
                g_free(name);
                continue;
            }
            switch (codesc->type)
            {
#define OPTION_TYPE_CASE(NAME, TYPE) \
    case T_##NAME: \
        value = fcitx_utils_new(TYPE); \
        break;
                OPTION_TYPE_CASE(Integer, int);
                OPTION_TYPE_CASE(Boolean, boolean);
                OPTION_TYPE_CASE(Char, char);
                OPTION_TYPE_CASE(Color, FcitxConfigColor);
                OPTION_TYPE_CASE(Enum, int);
                OPTION_TYPE_CASE(File, char*);
                OPTION_TYPE_CASE(Font, char*);
                OPTION_TYPE_CASE(Hotkey, FcitxHotkeys);
                OPTION_TYPE_CASE(String, char*);
                OPTION_TYPE_CASE(I18NString, char*);
                default:
                    break;
            }
            if (value)
                g_hash_table_insert(self->dummy_value, name, value);
            else
                g_free(name);
        }
    }
    return self;
}


void dummy_config_free(DummyConfig* self)
{
    FcitxConfigFree(&self->config);
    g_hash_table_destroy(self->dummy_value);
    g_free(self);
}

void dummy_config_load(DummyConfig* self, FILE* fp)
{
    if (!self->config.configFile) {
        self->config.configFile = FcitxConfigParseConfigFileFp(fp, self->cfdesc);

        HASH_FOREACH(cgdesc, self->cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                gchar* name = g_strdup_printf("%s/%s", cgdesc->groupName, codesc->optionName);
                void* value = g_hash_table_lookup(self->dummy_value, name);
                g_free(name);
                if (!value)
                    continue;
                // assert(self->dummyValue[name]);
                FcitxConfigBindValue(self->config.configFile, cgdesc->groupName, codesc->optionName, value, NULL, NULL);
            }
        }
    }
    else {
        self->config.configFile = FcitxConfigParseIniFp(fp, self->config.configFile);
    }

}

void dummy_config_sync(DummyConfig* self)
{
    FcitxConfigBindSync(&self->config);
}

gboolean dummy_config_valid(DummyConfig* self)
{
    return self->config.configFile != NULL;
}

void dummy_config_bind(DummyConfig* self, char* group, char* option, FcitxSyncFilter filter, void* arg)
{
    if (!self->config.configFile)
        return;
    gchar* name = g_strdup_printf("%s/%s", group, option);
    void* value = g_hash_table_lookup(self->dummy_value, name);
    g_free(name);
    if (!value)
        return;

    // assert(self->dummyValue[name]);
    FcitxConfigBindValue(self->config.configFile, group, option, value, filter, arg);

}