// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_stages.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#else
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include <dirent.h>
#endif

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

// include {{{

#include "cimgui.h"
#include "cimgui_impl.h"
#include "koh.h"
#include "raylib.h"
#include <assert.h>
#include "koh_common.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "koh_timerman.h"

// }}}

Sound snd_beep;
Color color_background_clear = GRAY;
static Camera2D cam = {
    .zoom = 1.,
};
TimerMan *tm = NULL;
const int scr_w = 300, scr_h = 60;
float times[] = {
    25. * 60., 
    5 * 60.
};
Color colors[] = {
    RED,
    GREEN,
};
int i = 0;
bool stop = false;

static bool tmr_on_update(struct Timer *tmr);
static void tmr_on_stop(struct Timer *tmr);

static void init() {
    stop = false;
    timerman_add(tm, (TimerDef) {
        .duration = times[i],
        .on_update = tmr_on_update,
        .on_stop = tmr_on_stop,
    });
}

static void tmr_on_stop(struct Timer *tmr) {
    i = (i + 1) % 2;
    init();
    /*trace("tmr_on_stop: i %d\n", i);*/
}

static bool tmr_on_update(struct Timer *tmr) {
    /*trace("tmr_on_update: amount %f, i %d\n", tmr->amount, i);*/
    DrawRectangle(0, 0, scr_w * (1. - tmr->amount), scr_h, colors[i]);
    const float thick = 4;
    DrawRectangleLinesEx((Rectangle) { 0., 0, scr_w, scr_h, }, thick, BLACK);
    return stop;
}
static void update(void) {
    if (IsKeyPressed(KEY_SPACE)) {
        /*trace("update: SPACE pressed\n");*/
        PlaySound(snd_beep);
        stop = true;
    }

    BeginDrawing();
    BeginMode2D(cam);
    ClearBackground(color_background_clear);
    timerman_update(tm);
    EndMode2D();
    EndDrawing();
}

void sig_handler(int sig) {
    printf("sig_handler: %d signal catched\n", sig);
    koh_backtrace_print();
    KOH_EXIT(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    signal(SIGSEGV, sig_handler);

    SetTraceLogCallback(koh_log_custom);

    koh_hashers_init();
    logger_init();

    SetTraceLogLevel(LOG_WARNING);

    SetConfigFlags(
        FLAG_WINDOW_UNDECORATED |
        FLAG_WINDOW_TOPMOST |
        FLAG_WINDOW_ALWAYS_RUN
    );  
       
    InitWindow(scr_w, scr_h, "pomidor");

    /*SetWindowPosition(40, 40);*/
    /*SetWindowPosition(GetMonitorPosition(1).x, 0);*/

    SetExitKey(KEY_NULL);

    koh_common_init();

    InitAudioDevice();

    snd_beep = LoadSound("assets/beepgen.wav");

    koh_render_init();

    tm = timerman_new(40, "timers");
    init();

    SetTargetFPS(120);
    while (!WindowShouldClose() && !koh_cmn()->quit) {
        update();
    }

    timerman_free(tm);
    koh_music_shutdown();       // добавить в систему инициализации
    koh_render_shutdown();// добавить в систему инициализации
    koh_common_shutdown();// добавить в систему инициализации
    CloseWindow();// добавить в систему инициализации
    logger_shutdown();

    return EXIT_SUCCESS;
}
