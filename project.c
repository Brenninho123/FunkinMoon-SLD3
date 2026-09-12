#ifndef FUNKIN_MOON_SLD3_H
#ifndef FUNKIN_MOON_SLD3_IMPLEMENTATION
#define FUNKIN_MOON_SLD3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PATH_LENGTH 512
#define MAX_ASSETS 1024

typedef enum {
    ASSET_TYPE_AUDIO_OGG,
    ASSET_TYPE_AUDIO_MP3,
    ASSET_TYPE_IMAGE_PNG,
    ASSET_TYPE_DATA_JSON,
    ASSET_TYPE_UNKNOWN
} AssetType;

typedef struct {
    char path[MAX_PATH_LENGTH];
    char identifier[128];
    AssetType type;
    size_t size_bytes;
    bool is_preloaded;
} AssetEntry;

typedef struct {
    char title[128];
    char package_id[128];
    char version[32];
    int window_width;
    int window_height;
    int target_fps;
    bool fullscreen;
    bool vsync;
    
    AssetEntry registry[MAX_ASSETS];
    size_t asset_count;
} EngineConfig;

void engine_config_init(EngineConfig *config, const char *title, const char *pkg, const char *ver);
void engine_config_set_display(EngineConfig *config, int width, int height, int fps, bool vsync, bool fullscreen);
bool engine_config_register_asset(EngineConfig *config, const char *path, const char *id, AssetType type, bool preload);
const AssetEntry* engine_config_find_asset(const EngineConfig *config, const char *id);
void engine_config_free(EngineConfig *config);

#endif
#endif

#ifdef FUNKIN_MOON_SLD3_IMPLEMENTATION

void engine_config_init(EngineConfig *config, const char *title, const char *pkg, const char *ver) {
    if (!config) return;
    
    memset(config, 0, sizeof(EngineConfig));
    
    if (title) strncpy(config->title, title, sizeof(config->title) - 1);
    if (pkg) strncpy(config->package_id, pkg, sizeof(config->package_id) - 1);
    if (ver) strncpy(config->version, ver, sizeof(config->version) - 1);
    
    config->window_width = 1280;
    config->window_height = 720;
    config->target_fps = 60;
    config->vsync = false;
    config->fullscreen = false;
    config->asset_count = 0;
}

void engine_config_set_display(EngineConfig *config, int width, int height, int fps, bool vsync, bool fullscreen) {
    if (!config) return;
    
    config->window_width = width;
    config->window_height = height;
    config->target_fps = fps;
    config->vsync = vsync;
    config->fullscreen = fullscreen;
}

bool engine_config_register_asset(EngineConfig *config, const char *path, const char *id, AssetType type, bool preload) {
    if (!config || !path || !id) return false;
    if (config->asset_count >= MAX_ASSETS) return false;

    AssetEntry *entry = &config->registry[config->asset_count];
    
    strncpy(entry->path, path, sizeof(entry->path) - 1);
    strncpy(entry->identifier, id, sizeof(entry->identifier) - 1);
    entry->type = type;
    entry->is_preloaded = preload;
    entry->size_bytes = 0;

    config->asset_count++;
    return true;
}

const AssetEntry* engine_config_find_asset(const EngineConfig *config, const char *id) {
    if (!config || !id) return NULL;

    for (size_t i = 0; i < config->asset_count; i++) {
        if (strcmp(config->registry[i].identifier, id) == 0) {
            return &config->registry[i];
        }
    }
    return NULL;
}

void engine_config_free(EngineConfig *config) {
    if (!config) return;
    memset(config, 0, sizeof(EngineConfig));
}

#endif
