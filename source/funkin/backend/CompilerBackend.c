#ifndef FUNKIN_COMPILER_BACKEND_H
#define FUNKIN_COMPILER_BACKEND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BACKEND_OPT_O0 = 0,
    BACKEND_OPT_O1,
    BACKEND_OPT_O2,
    BACKEND_OPT_O3,
    BACKEND_OPT_OS
} backend_opt_level_t;

typedef enum {
    BACKEND_ARCH_X86_64 = 0,
    BACKEND_ARCH_ARM64,
    BACKEND_ARCH_WASM32
} backend_arch_t;

typedef struct {
    backend_opt_level_t opt_level;
    backend_arch_t target_arch;
    bool enable_simd;
    bool enable_lto;
    bool generate_symbols;
    char custom_defines[512];
} backend_config_t;

void compiler_backend_init(backend_config_t *config);
void compiler_backend_set_optimization(backend_config_t *config, backend_opt_level_t level);
void compiler_backend_set_target(backend_config_t *config, backend_arch_t arch);
void compiler_backend_add_define(backend_config_t *config, const char *key, const char *value);
int compiler_backend_emit_args(const backend_config_t *config, char *buffer, size_t buffer_size);

#endif

#ifdef FUNKIN_COMPILER_BACKEND_IMPLEMENTATION

void compiler_backend_init(backend_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(backend_config_t));
    
    config->opt_level = BACKEND_OPT_O3;
    config->target_arch = BACKEND_ARCH_X86_64;
    config->enable_simd = true;
    config->enable_lto = false;
    config->generate_symbols = false;
    config->custom_defines[0] = '\0';
}

void compiler_backend_set_optimization(backend_config_t *config, backend_opt_level_t level) {
    if (config) {
        config->opt_level = level;
    }
}

void compiler_backend_set_target(backend_config_t *config, backend_arch_t arch) {
    if (config) {
        config->target_arch = arch;
    }
}

void compiler_backend_add_define(backend_config_t *config, const char *key, const char *value) {
    if (!config || !key) return;
    
    char define_buf[128];
    if (value && strlen(value) > 0) {
        snprintf(define_buf, sizeof(define_buf), "-D%s=%s ", key, value);
    } else {
        snprintf(define_buf, sizeof(define_buf), "-D%s ", key);
    }

    if (strlen(config->custom_defines) + strlen(define_buf) < sizeof(config->custom_defines)) {
        strcat(config->custom_defines, define_buf);
    }
}

int compiler_backend_emit_args(const backend_config_t *config, char *buffer, size_t buffer_size) {
    if (!config || !buffer || buffer_size == 0) return -1;

    char opt_flag[8] = "-O3";
    switch (config->opt_level) {
        case BACKEND_OPT_O0: strncpy(opt_flag, "-O0", sizeof(opt_flag)); break;
        case BACKEND_OPT_O1: strncpy(opt_flag, "-O1", sizeof(opt_flag)); break;
        case BACKEND_OPT_O2: strncpy(opt_flag, "-O2", sizeof(opt_flag)); break;
        case BACKEND_OPT_O3: strncpy(opt_flag, "-O3", sizeof(opt_flag)); break;
        case BACKEND_OPT_OS: strncpy(opt_flag, "-Os", sizeof(opt_flag)); break;
    }

    char arch_flag[32] = "";
    switch (config->target_arch) {
        case BACKEND_ARCH_X86_64: strncpy(arch_flag, "-m64", sizeof(arch_flag)); break;
        case BACKEND_ARCH_ARM64: strncpy(arch_flag, "-arch arm64", sizeof(arch_flag)); break;
        case BACKEND_ARCH_WASM32: strncpy(arch_flag, "-target wasm32", sizeof(arch_flag)); break;
    }

    snprintf(buffer, buffer_size, "%s %s %s %s %s %s",
        opt_flag,
        arch_flag,
        config->enable_simd ? "-msse4.2 -mavx" : "",
        config->enable_lto ? "-flto" : "",
        config->generate_symbols ? "-g" : "-s",
        config->custom_defines
    );

    return 0;
}

#endif
