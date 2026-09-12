#ifndef FUNKIN_PATHS_H
#define FUNKIN_PATHS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PATHS_MAX_BUFFER 1024
#define PATHS_DEFAULT_FOLDER "assets"

typedef struct {
    char current_level[128];
    char root_folder[256];
} funkin_paths_config_t;

void paths_init(const char *root);
void paths_set_level(const char *level);

const char *paths_get_path(const char *file, const char *type, const char *library);
const char *paths_file(const char *file, const char *library);

const char *paths_txt(const char *key, const char *library);
const char *paths_xml(const char *key, const char *library);
const char *paths_json(const char *key, const char *library);
const char *paths_lua(const char *key, const char *library);

const char *paths_image(const char *key, const char *library);
const char *paths_sound(const char *key, const char *library);
const char *paths_music(const char *key, const char *library);
const char *paths_inst(const char *song);
const char *paths_voices(const char *song);

const char *paths_font(const char *key);

#endif

#ifdef FUNKIN_PATHS_IMPLEMENTATION

static funkin_paths_config_t g_paths = {
    .current_level = "",
    .root_folder = PATHS_DEFAULT_FOLDER
};

static char g_path_buffer[PATHS_MAX_BUFFER];

void paths_init(const char *root) {
    if (root && strlen(root) > 0) {
        strncpy(g_paths.root_folder, root, sizeof(g_paths.root_folder) - 1);
    } else {
        strncpy(g_paths.root_folder, PATHS_DEFAULT_FOLDER, sizeof(g_paths.root_folder) - 1);
    }
    g_paths.current_level[0] = '\0';
}

void paths_set_level(const char *level) {
    if (level) {
        strncpy(g_paths.current_level, level, sizeof(g_paths.current_level) - 1);
    } else {
        g_paths.current_level[0] = '\0';
    }
}

const char *paths_get_path(const char *file, const char *type, const char *library) {
    g_path_buffer[0] = '\0';

    if (library && strlen(library) > 0) {
        snprintf(g_path_buffer, sizeof(g_path_buffer), "%s/%s/%s", g_paths.root_folder, library, file);
        return g_path_buffer;
    }

    if (strlen(g_paths.current_level) > 0) {
        snprintf(g_path_buffer, sizeof(g_path_buffer), "%s/%s/%s", g_paths.root_folder, g_paths.current_level, file);
        return g_path_buffer;
    }

    snprintf(g_path_buffer, sizeof(g_path_buffer), "%s/shared/%s", g_paths.root_folder, file);
    return g_path_buffer;
}

const char *paths_file(const char *file, const char *library) {
    return paths_get_path(file, NULL, library);
}

const char *paths_txt(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "data/%s.txt", key);
    return paths_get_path(file_name, "TEXT", library);
}

const char *paths_xml(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "images/%s.xml", key);
    return paths_get_path(file_name, "TEXT", library);
}

const char *paths_json(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "data/%s.json", key);
    return paths_get_path(file_name, "TEXT", library);
}

const char *paths_lua(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "scripts/%s.lua", key);
    return paths_get_path(file_name, "TEXT", library);
}

const char *paths_image(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "images/%s.png", key);
    return paths_get_path(file_name, "IMAGE", library);
}

const char *paths_sound(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "sounds/%s.ogg", key);
    return paths_get_path(file_name, "SOUND", library);
}

const char *paths_music(const char *key, const char *library) {
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "music/%s.ogg", key);
    return paths_get_path(file_name, "MUSIC", library);
}

const char *paths_inst(const char *song) {
    char formatted_song[128];
    char file_name[256];
    
    strncpy(formatted_song, song, sizeof(formatted_song) - 1);
    for (int i = 0; formatted_song[i]; i++) {
        if (formatted_song[i] == ' ') formatted_song[i] = '-';
    }

    snprintf(file_name, sizeof(file_name), "songs/%s/Inst.ogg", formatted_song);
    return paths_get_path(file_name, "MUSIC", NULL);
}

const char *paths_voices(const char *song) {
    char formatted_song[128];
    char file_name[256];
    
    strncpy(formatted_song, song, sizeof(formatted_song) - 1);
    for (int i = 0; formatted_song[i]; i++) {
        if (formatted_song[i] == ' ') formatted_song[i] = '-';
    }

    snprintf(file_name, sizeof(file_name), "songs/%s/Voices.ogg", formatted_song);
    return paths_get_path(file_name, "MUSIC", NULL);
}

const char *paths_font(const char *key) {
    g_path_buffer[0] = '\0';
    snprintf(g_path_buffer, sizeof(g_path_buffer), "%s/fonts/%s", g_paths.root_folder, key);
    return g_path_buffer;
}

#endif
