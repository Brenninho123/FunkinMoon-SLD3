#ifndef FUNKIN_MOON_SLD3_H
#define FUNKIN_MOON_SLD3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define SLD3_VERSION_MAJOR 1
#define SLD3_VERSION_MINOR 0
#define SLD3_VERSION_PATCH 0

#define SLD3_MAX_PATH 512
#define SLD3_MAX_NAME 128
#define SLD3_MAX_ASSETS 2048
#define SLD3_HASH_SIZE 256

typedef enum {
    SLD3_ASSET_UNKNOWN = 0,
    SLD3_ASSET_AUDIO_OGG,
    SLD3_ASSET_AUDIO_WAV,
    SLD3_ASSET_IMAGE_PNG,
    SLD3_ASSET_TEXT_JSON,
    SLD3_ASSET_TEXT_LUA,
    SLD3_ASSET_FONT_TTF
} sld3_asset_type_t;

typedef enum {
    SLD3_FLAG_NONE       = 0,
    SLD3_FLAG_PRELOAD    = 1 << 0,
    SLD3_FLAG_STREAM     = 1 << 1,
    SLD3_FLAG_ENCRYPTED  = 1 << 2,
    SLD3_FLAG_CACHE_LOCK = 1 << 3
} sld3_asset_flags_t;

typedef struct {
    char path[SLD3_MAX_PATH];
    char key[SLD3_MAX_NAME];
    sld3_asset_type_t type;
    uint32_t flags;
    size_t size_bytes;
    uint32_t checksum;
    void *raw_data;
} sld3_asset_t;

typedef struct {
    char title[SLD3_MAX_NAME];
    char version[32];
    uint32_t target_fps;
    uint32_t width;
    uint32_t height;
    bool vsync;
    bool fullscreen;
} sld3_render_config_t;

typedef struct sld3_node {
    sld3_asset_t asset;
    struct sld3_node *next;
} sld3_node_t;

typedef struct {
    sld3_render_config_t config;
    sld3_node_t *buckets[SLD3_HASH_SIZE];
    size_t total_assets;
    bool initialized;
} sld3_engine_t;

void sld3_engine_init(sld3_engine_t *engine, const char *title, const char *version);
void sld3_engine_configure_display(sld3_engine_t *engine, uint32_t width, uint32_t height, uint32_t fps, bool vsync, bool fullscreen);
bool sld3_asset_register(sld3_engine_t *engine, const char *key, const char *path, sld3_asset_type_t type, uint32_t flags);
sld3_asset_t *sld3_asset_get(sld3_engine_t *engine, const char *key);
bool sld3_asset_unload(sld3_engine_t *engine, const char *key);
void sld3_engine_shutdown(sld3_engine_t *engine);

#endif

#ifdef FUNKIN_MOON_SLD3_IMPLEMENTATION

static uint32_t sld3_hash_key(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % SLD3_HASH_SIZE;
}

void sld3_engine_init(sld3_engine_t *engine, const char *title, const char *version) {
    if (!engine) return;
    memset(engine, 0, sizeof(sld3_engine_t));
    if (title) strncpy(engine->config.title, title, SLD3_MAX_NAME - 1);
    if (version) strncpy(engine->config.version, version, 31);
    
    engine->config.width = 1280;
    engine->config.height = 720;
    engine->config.target_fps = 60;
    engine->config.vsync = false;
    engine->config.fullscreen = false;
    engine->total_assets = 0;
    engine->initialized = true;
}

void sld3_engine_configure_display(sld3_engine_t *engine, uint32_t width, uint32_t height, uint32_t fps, bool vsync, bool fullscreen) {
    if (!engine || !engine->initialized) return;
    engine->config.width = width;
    engine->config.height = height;
    engine->config.target_fps = fps;
    engine->config.vsync = vsync;
    engine->config.fullscreen = fullscreen;
}

bool sld3_asset_register(sld3_engine_t *engine, const char *key, const char *path, sld3_asset_type_t type, uint32_t flags) {
    if (!engine || !engine->initialized || !key || !path) return false;
    
    uint32_t idx = sld3_hash_key(key);
    sld3_node_t *curr = engine->buckets[idx];
    while (curr) {
        if (strcmp(curr->asset.key, key) == 0) return false;
        curr = curr->next;
    }

    sld3_node_t *new_node = (sld3_node_t *)malloc(sizeof(sld3_node_t));
    if (!new_node) return false;

    memset(new_node, 0, sizeof(sld3_node_t));
    strncpy(new_node->asset.key, key, SLD3_MAX_NAME - 1);
    strncpy(new_node->asset.path, path, SLD3_MAX_PATH - 1);
    new_node->asset.type = type;
    new_node->asset.flags = flags;
    new_node->asset.raw_data = NULL;
    new_node->asset.size_bytes = 0;

    new_node->next = engine->buckets[idx];
    engine->buckets[idx] = new_node;
    engine->total_assets++;

    return true;
}

sld3_asset_t *sld3_asset_get(sld3_engine_t *engine, const char *key) {
    if (!engine || !engine->initialized || !key) return NULL;
    
    uint32_t idx = sld3_hash_key(key);
    sld3_node_t *curr = engine->buckets[idx];
    while (curr) {
        if (strcmp(curr->asset.key, key) == 0) {
            return &curr->asset;
        }
        curr = curr->next;
    }
    return NULL;
}

bool sld3_asset_unload(sld3_engine_t *engine, const char *key) {
    if (!engine || !engine->initialized || !key) return false;
    
    uint32_t idx = sld3_hash_key(key);
    sld3_node_t *curr = engine->buckets[idx];
    sld3_node_t *prev = NULL;

    while (curr) {
        if (strcmp(curr->asset.key, key) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                engine->buckets[idx] = curr->next;
            }
            if (curr->asset.raw_data) {
                free(curr->asset.raw_data);
            }
            free(curr);
            engine->total_assets--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

void sld3_engine_shutdown(sld3_engine_t *engine) {
    if (!engine || !engine->initialized) return;

    for (size_t i = 0; i < SLD3_HASH_SIZE; i++) {
        sld3_node_t *curr = engine->buckets[i];
        while (curr) {
            sld3_node_t *temp = curr;
            curr = curr->next;
            if (temp->asset.raw_data) {
                free(temp->asset.raw_data);
            }
            free(temp);
        }
        engine->buckets[i] = NULL;
    }

    engine->total_assets = 0;
    engine->initialized = false;
}

#endif
