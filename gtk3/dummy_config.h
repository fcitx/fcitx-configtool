#ifndef DUMMY_CONFIG_H
#define DUMMY_CONFIG_H

#include <glib.h>
#include <fcitx-config/fcitx-config.h>

typedef struct _DummyConfig {
    GHashTable* dummy_value;
    FcitxGenericConfig config;
    FcitxConfigFile* cfile;
    FcitxConfigFileDesc* cfdesc;
} DummyConfig;

DummyConfig* dummy_config_new(FcitxConfigFileDesc* cfdesc);
void dummy_config_free(DummyConfig* self);
void dummy_config_load(DummyConfig* self, FILE* fp);
void dummy_config_bind(DummyConfig* self, char* group, char* option, FcitxSyncFilter filter, void* arg);
gboolean dummy_config_valid(DummyConfig* self);
void dummy_config_sync(DummyConfig* self);


#endif // DUMMY_CONFIG_H