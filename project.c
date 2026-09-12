#ifndef FUNKIN_MOON_SLD3_H
#define FUNKIN_MOON_SLD3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#define SLD3_VERSION_MAJOR 3
#define SLD3_VERSION_MINOR 0
#define SLD3_VERSION_PATCH 0

#define SLD3_MAX_PATH 512
#define SLD3_MAX_NAME 128
#define SLD3_HASH_SIZE 2048

typedef enum {
    SLD3_LOG_DEBUG = 0,
    SLD3_LOG_INFO,
    SLD3_LOG_WARN,
    SLD3_LOG_ERROR,
    SLD3_LOG_FATAL
} sld3_log_level_t;

typedef enum {
    SLD3_ASSET_UNKNOWN = 0,
    SLD3_ASSET_AUDIO_OGG,
    SLD3_ASSET_AUDIO_WAV,
    SLD3_ASSET_IMAGE_PNG,
    SLD3_ASSET_TEXT_JSON,
    SLD3_ASSET_TEXT_LUA,
    SLD3_ASSET_FONT_TTF,
    SLD3_ASSET_BINARY
} sld3_asset_type_t;

typedef enum {
    SLD3_FLAG_NONE       = 0,
    SLD3_FLAG_PRELOAD    = 1 << 0,
    SLD3_FLAG_STREAM     = 1 << 1,
    SLD3_FLAG_ENCRYPTED  = 1 << 2,
    SLD3_FLAG_CACHE_LOCK = 1 << 3,
    SLD3_FLAG_ASYNC_LOAD = 1 << 4
} sld3_asset_flags_t;

typedef enum {
    SLD3_STATE_UNLOADED = 0,
    SLD3_STATE_LOADING,
    SLD3_STATE_READY,
    SLD3_STATE_FAILED
} sld3_asset_state_t;

typedef struct sld3_asset {
    char path[SLD3_MAX_PATH];
    char key[SLD3_MAX_NAME];
    sld3_asset_type_t type;
    uint32_t flags;
    sld3_asset_state_t state;
    size_t size_bytes;
    uint32_t checksum;
    void *raw_data;
    uint32_t ref_count;
    uint64_t last_accessed_frame;
} sld3_asset_t;

typedef struct {
    char title[SLD3_MAX_NAME];
    char version[32];
    uint32_t target_fps;
    uint32_t width;
    uint32_t height;
    bool vsync;
    bool fullscreen;
    size_t memory_budget_bytes;
} sld3_render_config_t;

typedef struct sld3_node {
    sld3_asset_t asset;
    struct sld3_node *next;
} sld3_node_t;

typedef void (*sld3_log_callback_t)(sld3_log_level_t level, const char *msg);

typedef struct {
    sld3_render_config_t config;
    sld3_node_t *buckets[SLD3_HASH_SIZE];
    size_t total_assets;
    size_t allocated_memory;
    uint64_t frame_counter;
    sld3_log_callback_t logger;
    bool initialized;
} sld3_engine_t;

void sld3_engine_init(sld3_engine_t *engine, const char *title, const char *version);
void sld3_engine_configure_display(sld3_engine_t *engine, uint32_t width, uint32_t height, uint32_t fps, bool vsync, bool fullscreen);
void sld3_engine_set_memory_budget(sld3_engine_t *engine, size_t bytes);
void sld3_engine_set_logger(sld3_engine_t *engine, sld3_log_callback_t logger);
void sld3_log(sld3_engine_t *engine, sld3_log_level_t level, const char *fmt, ...);

bool sld3_asset_register(sld3_engine_t *engine, const char *key, const char *path, sld3_asset_type_t type, uint32_t flags);
sld3_asset_t *sld3_asset_retain(sld3_engine_t *engine, const char *key);
bool sld3_asset_release(sld3_engine_t *engine, const char *key);
bool sld3_asset_preload_all(sld3_engine_t *engine);

void sld3_engine_tick(sld3_engine_t *engine);
void sld3_engine_garbage_collect_lru(sld3_engine_t *engine);
void sld3_engine_shutdown(sld3_engine_t *engine);

#endif

#ifdef FUNKIN_MOON_SLD3_IMPLEMENTATION

static uint32_t sld3_hash_key(const char *str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 16777619u;
    }
    return hash % SLD3_HASH_SIZE;
}

static uint32_t sld3_compute_checksum(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

void sld3_log(sld3_engine_t *engine, sld3_log_level_t level, const char *fmt, ...) {
    if (!engine || !engine->logger) return;
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    engine->logger(level, buffer);
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
    engine->config.memory_budget_bytes = 1024 * 1024 * 1024;
    engine->allocated_memory = 0;
    engine->total_assets = 0;
    engine->frame_counter = 0;
    engine->logger = NULL;
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

void sld3_engine_set_memory_budget(sld3_engine_t *engine, size_t bytes) {
    if (engine && engine->initialized) {
        engine->config.memory_budget_bytes = bytes;
    }
}

void sld3_engine_set_logger(sld3_engine_t *engine, sld3_log_callback_t logger) {
    if (engine && engine->initialized) {
        engine->logger = logger;
    }
}

void sld3_engine_tick(sld3_engine_t *engine) {
    if (engine && engine->initialized) {
        engine->frame_counter++;
    }
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
    new_node->asset.state = SLD3_STATE_UNLOADED;
    new_node->asset.raw_data = NULL;
    new_node->asset.size_bytes = 0;
    new_node->asset.ref_count = 0;
    new_node->asset.last_accessed_frame = engine->frame_counter;

    new_node->next = engine->buckets[idx];
    engine->buckets[idx] = new_node;
    engine->total_assets++;

    sld3_log(engine, SLD3_LOG_INFO, "Registered asset: %s", key);
    return true;
}

static bool sld3_internal_load_asset(sld3_engine_t *engine, sld3_asset_t *asset) {
    if (asset->state == SLD3_STATE_READY) return true;

    FILE *f = fopen(asset->path, "rb");
    if (!f) {
        asset->state = SLD3_STATE_FAILED;
        sld3_log(engine, SLD3_LOG_ERROR, "Failed to open asset: %s", asset->path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (engine->allocated_memory + size > engine->config.memory_budget_bytes) {
        sld3_engine_garbage_collect_lru(engine);
        if (engine->allocated_memory + size > engine->config.memory_budget_bytes) {
            fclose(f);
            asset->state = SLD3_STATE_FAILED;
            sld3_log(engine, SLD3_LOG_ERROR, "Memory budget exceeded for asset: %s", asset->key);
            return false;
        }
    }

    void *data = malloc(size);
    if (!data) {
        fclose(f);
        asset->state = SLD3_STATE_FAILED;
        return false;
    }

    fread(data, 1, size, f);
    fclose(f);

    asset->raw_data = data;
    asset->size_bytes = size;
    asset->checksum = sld3_compute_checksum((const uint8_t *)data, size);
    asset->state = SLD3_STATE_READY;
    engine->allocated_memory += size;

    sld3_log(engine, SLD3_LOG_INFO, "Loaded asset: %s (%zu bytes)", asset->key, size);
    return true;
}

sld3_asset_t *sld3_asset_retain(sld3_engine_t *engine, const char *key) {
    if (!engine || !engine->initialized || !key) return NULL;
    
    uint32_t idx = sld3_hash_key(key);
    sld3_node_t *curr = engine->buckets[idx];
    while (curr) {
        if (strcmp(curr->asset.key, key) == 0) {
            if (curr->asset.state == SLD3_STATE_UNLOADED) {
                if (!sld3_internal_load_asset(engine, &curr->asset)) {
                    return NULL;
                }
            }
            curr->asset.ref_count++;
            curr->asset.last_accessed_frame = engine->frame_counter;
            return &curr->asset;
        }
        curr = curr->next;
    }
    return NULL;
}

bool sld3_asset_release(sld3_engine_t *engine, const char *key) {
    if (!engine || !engine->initialized || !key) return false;
    
    uint32_t idx = sld3_hash_key(key);
    sld3_node_t *curr = engine->buckets[idx];
    while (curr) {
        if (strcmp(curr->asset.key, key) == 0) {
            if (curr->asset.ref_count > 0) {
                curr->asset.ref_count--;
                curr->asset.last_accessed_frame = engine->frame_counter;
                return true;
            }
            return false;
        }
        curr = curr->next;
    }
    return false;
}

bool sld3_asset_preload_all(sld3_engine_t *engine) {
    if (!engine || !engine->initialized) return false;

    bool success = true;
    for (size_t i = 0; i < SLD3_HASH_SIZE; i++) {
        sld3_node_t *curr = engine->buckets[i];
        while (curr) {
            if ((curr->asset.flags & SLD3_FLAG_PRELOAD) && curr->asset.state == SLD3_STATE_UNLOADED) {
                if (!sld3_internal_load_asset(engine, &curr->asset)) {
                    success = false;
                }
            }
            curr = curr->next;
        }
    }
    return success;
}

void sld3_engine_garbage_collect_lru(sld3_engine_t *engine) {
    if (!engine || !engine->initialized) return;

    sld3_asset_t *lru_asset = NULL;
    uint64_t oldest_frame = UINT64_MAX;

    for (size_t i = 0; i < SLD3_HASH_SIZE; i++) {
        sld3_node_t *curr = engine->buckets[i];
        while (curr) {
            if (curr->asset.ref_count == 0 && 
                curr->asset.state == SLD3_STATE_READY && 
                !(curr->asset.flags & SLD3_FLAG_CACHE_LOCK)) {
                
                if (curr->asset.last_accessed_frame < oldest_frame) {
                    oldest_frame = curr->asset.last_accessed_frame;
                    lru_asset = &curr->asset;
                }
            }
            curr = curr->next;
        }
    }

    if (lru_asset) {
        engine->allocated_memory -= lru_asset->size_bytes;
        free(lru_asset->raw_data);
        lru_asset->raw_data = NULL;
        lru_asset->size_bytes = 0;
        lru_asset->state = SLD3_STATE_UNLOADED;
        sld3_log(engine, SLD3_LOG_INFO, "Evicted LRU asset: %s", lru_asset->key);
    }
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
    engine->allocated_memory = 0;
    engine->frame_counter = 0;
    engine->initialized = false;
}

#endif
