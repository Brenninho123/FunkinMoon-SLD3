#ifndef BUILDING_COMPILE_H
#define BUILDING_COMPILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    PLATFORM_WINDOWS = 0,
    PLATFORM_LINUX,
    PLATFORM_MACOS,
    PLATFORM_UNKNOWN
} target_platform_t;

typedef struct {
    target_platform_t target;
    bool debug_mode;
    bool optimize_speed;
    char compiler_path[256];
    char output_name[128];
    char extra_flags[512];
} compile_config_t;

void compile_init(compile_config_t *config);
void compile_set_platform(compile_config_t *config, target_platform_t platform);
void compile_set_output(compile_config_t *config, const char *name);
void compile_add_flag(compile_config_t *config, const char *flag);
int compile_execute(const compile_config_t *config);

#endif

#ifdef BUILDING_COMPILE_IMPLEMENTATION

static target_platform_t compile_detect_host(void) {
#if defined(_WIN32) || defined(_WIN64)
    return PLATFORM_WINDOWS;
#elif defined(__APPLE__) || defined(__MACH__)
    return PLATFORM_MACOS;
#elif defined(__linux__)
    return PLATFORM_LINUX;
#else
    return PLATFORM_UNKNOWN;
#endif
}

void compile_init(compile_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(compile_config_t));
    
    config->target = compile_detect_host();
    config->debug_mode = false;
    config->optimize_speed = true;
    
    strncpy(config->compiler_path, "gcc", sizeof(config->compiler_path) - 1);
    strncpy(config->output_name, "FunkinMoon-SLD3", sizeof(config->output_name) - 1);
    config->extra_flags[0] = '\0';
}

void compile_set_platform(compile_config_t *config, target_platform_t platform) {
    if (config) {
        config->target = platform;
    }
}

void compile_set_output(compile_config_t *config, const char *name) {
    if (config && name) {
        strncpy(config->output_name, name, sizeof(config->output_name) - 1);
    }
}

void compile_add_flag(compile_config_t *config, const char *flag) {
    if (!config || !flag) return;
    if (strlen(config->extra_flags) + strlen(flag) + 2 < sizeof(config->extra_flags)) {
        strcat(config->extra_flags, " ");
        strcat(config->extra_flags, flag);
    }
}

int compile_execute(const compile_config_t *config) {
    if (!config) return -1;

    char command[2048];
    char binary_ext[16] = "";

    if (config->target == PLATFORM_WINDOWS) {
        strncpy(binary_ext, ".exe", sizeof(binary_ext) - 1);
    }

    snprintf(command, sizeof(command),
        "%s -std=c99 %s %s -Isource -I. source/Main.c -o bin/%s%s %s",
        config->compiler_path,
        config->optimize_speed ? "-O3" : "-O0",
        config->debug_mode ? "-g" : "-s",
        config->output_name,
        binary_ext,
        config->extra_flags
    );

#if defined(_WIN32)
    system("if not exist bin mkdir bin");
#else
    system("mkdir -p bin");
#endif

    return system(command);
}

#endif
