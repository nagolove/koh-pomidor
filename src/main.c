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
#include "beep2.h"
// }}}

Sound snd_beep;
Color color_background_clear = GRAY;
static Camera2D cam = {
    .zoom = 1.,
};
TimerMan *tm = NULL;
const int scr_w = 1920, scr_h = 30;

float times1[2] = {
    25. * 60., 
    5 * 60.
}, times2[2] = {
    25. * 60., 
    5 * 60.
};

float *next_times[2] = {
    times1, 
    times2,
};

Color colors[] = {
    RED,
    GREEN,
};
int i = 0, times_i = 0;
bool stop = false,
     is_sleep = false;

static bool tmr_on_update(struct Timer *tmr);
static void tmr_on_stop(struct Timer *tmr);

static void init() {
    stop = false;
    PlaySound(snd_beep);
    timerman_add(tm, (TimerDef) {
        .duration = next_times[times_i][i],
        .on_update = tmr_on_update,
        .on_stop = tmr_on_stop,
    });
}

static void tmr_on_stop(struct Timer *tmr) {
    i = (i + 1) % 2;
    times_i = (times_i + 1) % 2;
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

union TmrData {
    float t;
    void *p;
};

static const float next_timer_duration = 5.f;
static bool stop_next = false;

static bool tmr_on_update_next(struct Timer *tmr) {
    char buf[128] = {};
    union TmrData u = { .p = tmr->data, };
    float duration = u.t;

    //printf("tmr_on_update_next: duration %f\n", duration);

    sprintf(buf, "next - %.0f minutes", duration / 60.);
    const int font_size = 30;
    Vector2 m = MeasureTextEx(GetFontDefault(), buf, font_size, 0);
    Vector2 pos = {
        (scr_w - m.x) / 2.,
        (scr_h - m.y) / 2.,
    };
    DrawTextEx(GetFontDefault(), buf, pos, font_size, 0., BLACK);
    return stop_next;
}

void next_timer(int i, float sec) {
    assert(i == 1 || i == 0);
    int next_times_i = (times_i + 1) % 2;

    stop_next = false;
    next_times[next_times_i][i] = sec;
    union TmrData u = { .t = sec };
    timerman_add(tm, (TimerDef) {
        .duration = next_timer_duration,
        .on_update = tmr_on_update_next,
        .data = u.p,
    });
}

static void update(void) {

    if (IsKeyPressed(KEY_S)) {
        is_sleep = !is_sleep;
        timerman_pause(tm, is_sleep);
    }

    if (IsKeyPressed(KEY_SPACE)) {
        stop = true;
    }

    if (IsKeyPressed(KEY_ONE)) {
        next_timer(0, 10. * 60);
    }
    if (IsKeyPressed(KEY_TWO)) {
        next_timer(0, 20. * 60);
    }
    if (IsKeyPressed(KEY_THREE)) {
        next_timer(0, 25. * 60);
    }
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_ONE)) {
        next_timer(1, 5. * 60);
    }
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_TWO)) {
        next_timer(1, 10. * 60);
    }
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_THREE)) {
        next_timer(1, 15. * 60);
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

    Wave wav = LoadWaveFromMemory(
        ".wav", assets_beepgen_wav, assets_beepgen_wav_len
    );

    snd_beep = LoadSoundFromWave(wav);
    UnloadWave(wav);

    koh_render_init();

    tm = timerman_new(20, "timers");
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
