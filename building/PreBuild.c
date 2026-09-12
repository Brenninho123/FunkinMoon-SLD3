#ifndef BUILDING_PREBUILD_H
#define BUILDING_PREBUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char source_dir[256];
    char output_header[256];
    bool auto_create_dirs;
} prebuild_config_t;

void prebuild_init(prebuild_config_t *config);
int prebuild_ensure_directories(void);
int prebuild_generate_build_info(const char *output_file);
int prebuild_execute(const prebuild_config_t *config);

#endif

#ifdef BUILDING_PREBUILD_IMPLEMENTATION

void prebuild_init(prebuild_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(prebuild_config_t));
    
    strncpy(config->source_dir, "source", sizeof(config->source_dir) - 1);
    strncpy(config->output_header, "source/BuildInfo.h", sizeof(config->output_header) - 1);
    config->auto_create_dirs = true;
}

int prebuild_ensure_directories(void) {
#if defined(_WIN32)
    int res = system("if not exist bin mkdir bin && if not exist obj mkdir obj");
#else
    int res = system("mkdir -p bin obj");
#endif
    return res;
}

int prebuild_generate_build_info(const char *output_file) {
    if (!output_file) return -1;

    FILE *f = fopen(output_file, "w");
    if (!f) return -1;

    fprintf(f, "#ifndef SLD3_BUILD_INFO_H\n");
    fprintf(f, "#define SLD3_BUILD_INFO_H\n\n");
    fprintf(f, "#define SLD3_BUILD_TIMESTAMP __DATE__ \" \" __TIME__\n");
    fprintf(f, "#define SLD3_ENGINE_NAME \"FunkinMoon-SLD3\"\n");
    fprintf(f, "#define SLD3_ENGINE_VERSION \"0.2.7\"\n\n");
    fprintf(f, "#endif\n");

    fclose(f);
    return 0;
}

int prebuild_execute(const prebuild_config_t *config) {
    if (!config) return -1;

    if (config->auto_create_dirs) {
        if (prebuild_ensure_directories() != 0) return -1;
    }

    return prebuild_generate_build_info(config->output_header);
}

#endif
