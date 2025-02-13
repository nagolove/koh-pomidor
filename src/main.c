// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_script.h"
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
#include <string.h>

// }}}

Sound snd_beep;
Color color_background_clear = GRAY;
static double last_time = 0.;
static Camera2D cam = {
    .zoom = 1.,
};

static void update(void) {
    if (IsKeyPressed(KEY_SPACE)) {
        PlaySound(snd_beep);
    }

    BeginDrawing();
    BeginMode2D(cam);
    ClearBackground(color_background_clear);
    EndMode2D();
    EndDrawing();
}

#if !defined(PLATFORM_WEB)

void sig_handler(int sig) {
    printf("sig_handler: %d signal catched\n", sig);
    /*
    // XXX:
    if (__STDC_VERSION__ >=201710L) {
    } else
        printf("sig_handler: %s signal catched\n", strsignal(sig));
    */
    koh_backtrace_print();
    KOH_EXIT(EXIT_FAILURE);
}
#endif

int main(int argc, char **argv) {
#if !defined(PLATFORM_WEB)
    signal(SIGSEGV, sig_handler);
#endif

    SetTraceLogCallback(koh_log_custom);

    koh_hashers_init();
    logger_init();

    const char *wnd_name = "lapsha";

    SetTraceLogLevel(LOG_WARNING);

#ifdef PLATFORM_WEB
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Set MSAA 4X hint before windows creation
    InitWindow(screenWidth_web, screenHeight_web, wnd_name);
    SetTraceLogLevel(LOG_ALL);
#else
    //SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_FULLSCREEN_MODE);  // Set MSAA 4X hint before windows creation
    SetConfigFlags(
        FLAG_WINDOW_UNDECORATED |
        FLAG_WINDOW_TOPMOST |
        FLAG_WINDOW_ALWAYS_RUN
    );  
       
    SetWindowPosition(40, 40);
    InitWindow(300, 60, wnd_name);
    SetWindowPosition(40, 40);

    SetTraceLogLevel(LOG_ALL);
    // FIXME: Работает только на моей конфигурации, сделать опцией
    // К примеру отрабатывать только на флаг -DDEV
#ifndef LAPTOP
    SetWindowPosition(GetMonitorPosition(1).x, 0);
#endif
    //dotool_setup_display(testing_ctx);
#endif

    SetExitKey(KEY_NULL);

    logger_register_functions();

    koh_common_init();

    // XXX: Требуется включение и выключение
    InitAudioDevice();

    snd_beep = LoadSound("assets/beepgen.wav");

    koh_render_init();

    // stage_init(ss);

    last_time = GetTime();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update, 60, 1);
#else
    //dotool_send_signal(testing_ctx);

    SetTargetFPS(120 * 3);
    while (!WindowShouldClose() && !koh_cmn()->quit) {
        update();
    }

#endif

    koh_music_shutdown();       // добавить в систему инициализации
    koh_render_shutdown();// добавить в систему инициализации
    koh_common_shutdown();// добавить в систему инициализации
    CloseWindow();// добавить в систему инициализации

    //dotool_free(testing_ctx);
    logger_shutdown();

    return EXIT_SUCCESS;
}
