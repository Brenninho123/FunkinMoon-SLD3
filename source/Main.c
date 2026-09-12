#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FUNKIN_MOON_SLD3_IMPLEMENTATION
#include "../project.c.txt"

typedef struct {
    double current_time;
    double delta_time;
    uint64_t frame_count;
    bool is_running;
} sld3_main_loop_t;

static sld3_engine_t g_engine;
static sld3_main_loop_t g_loop;

static void main_init(void) {
    sld3_engine_init(&g_engine, "Friday Night Funkin': Moon Engine - SLD3", "0.2.7");
    sld3_engine_configure_display(&g_engine, 1280, 720, 60, false, false);

    sld3_asset_register(&g_engine, "song_inst", "assets/songs/inst.ogg", SLD3_ASSET_AUDIO_OGG, SLD3_FLAG_STREAM);
    sld3_asset_register(&g_engine, "song_voices", "assets/songs/voices.ogg", SLD3_ASSET_AUDIO_OGG, SLD3_FLAG_STREAM);
    sld3_asset_register(&g_engine, "stage_data", "assets/shared/data/stage.json", SLD3_ASSET_TEXT_JSON, SLD3_FLAG_PRELOAD);
    sld3_asset_register(&g_engine, "mod_script", "assets/shared/scripts/mod.lua", SLD3_ASSET_TEXT_LUA, SLD3_FLAG_PRELOAD);

    g_loop.current_time = 0.0;
    g_loop.delta_time = 1.0 / 60.0;
    g_loop.frame_count = 0;
    g_loop.is_running = true;
}

static void main_update(double dt) {
    (void)dt;
    g_loop.frame_count++;
    g_loop.current_time += dt;
}

static void main_render(void) {

}

static void main_shutdown(void) {
    sld3_engine_shutdown(&g_engine);
    g_loop.is_running = false;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    main_init();

    while (g_loop.is_running && g_loop.frame_count < 3600) {
        main_update(g_loop.delta_time);
        main_render();
    }

    main_shutdown();
    return 0;
}
