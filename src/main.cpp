#include "Android_draw/draw.h"
#include "Android_touch/Touch.hpp"
#include "ui/theme/theme.hpp"
#include "ui/menu.hpp"
#include "ui/bar.hpp"
#include "ui/cfg.hpp"
#include "other/memory.hpp"
#include "func/visuals.hpp"
#include "func/aimbot.hpp"
#include "protect/oxorany.hpp"
#include "protect/security.hpp"
#include <cstdio>
#include <algorithm>
#include <unistd.h>

static void print_status(const char* status) {
    printf(oxorany("\033[2J\033[H\033[1;38;2;162;144;225m[]\033[0m \033[1;37m%s\033[0m\n"), status);
}

int main(int argc, char** argv) {
    protect::Security::run();

    screen_config();

    g_sw = std::max(displayInfo.height, displayInfo.width);
    g_sh = std::min(displayInfo.height, displayInfo.width);

    native_window_screen_x = g_sw;
    native_window_screen_y = g_sh;

    if (!initGUI_draw(native_window_screen_x, native_window_screen_y, true)) return -1;
    setInputPassThrough(true);

    touch::init(displayInfo.width, displayInfo.height, (uint8_t)displayInfo.orientation);

    print_status(oxorany("please start game"));

    static float alpha = 0.f;
    static bool prev_run = false;

    while (true) {
        screen_config();
        g_sw = std::max(displayInfo.height, displayInfo.width);
        g_sh = std::min(displayInfo.height, displayInfo.width);
        native_window_screen_x = (int)g_sw;
        native_window_screen_y = (int)g_sh;

        drawBegin();

        bool run = valid();

        if (run && proc::lib == 0) {
            proc::lib = get_lib();
        }

#if defined(__x86_64__)
        bool is_landscape = (displayInfo.orientation == 0 || displayInfo.orientation == 2);
#else
        bool is_landscape = (displayInfo.orientation == 1 || displayInfo.orientation == 3);
#endif

        if (run && proc::lib != 0 && !prev_run) {
            print_status(oxorany("game detected"));
            prev_run = true;
        } else if (!run && prev_run) {
            print_status(oxorany("game closed"));
            proc::lib = 0;
            prev_run = false;
        }

        if (is_landscape) {
            ImGuiIO& io = ImGui::GetIO();
            float dt = io.DeltaTime;
            if (dt <= 0.f || dt > 0.1f) dt = 0.016f;

            float target = run ? 1.f : 0.f;
            float spd = run ? 4.f : 6.f;

            if (alpha < target) {
                alpha += dt * spd;
                if (alpha > target) alpha = target;
            } else if (alpha > target) {
                alpha -= dt * spd;
                if (alpha < target) alpha = target;
            }

            ui::bar::set_game_alpha(alpha);

            
            static bool last_stream_proof = cfg::misc::stream_proof;
            static bool first_run = true;
            if (first_run || last_stream_proof != cfg::misc::stream_proof) {
                updateStreamProof(cfg::misc::stream_proof);
                last_stream_proof = cfg::misc::stream_proof;
                first_run = false;
            }

            if (ui::bar::g_open) {
                drawBackgroundDim(alpha * 0.5f); 
                drawSnow(alpha);                
                
                int radius = (int)(15.0f * alpha); 
                if (radius > 255) radius = 255;
                setBlurRadius(radius);
            } else {
                setBlurRadius(0);
            }

            if (alpha > 0.001f) {
                if (proc::lib != 0) {
                    aimbot::handle();
                    visuals::draw();
                }
            }
            ui::bar::render();
            ui::menu::render();
        }

        bool vis = ui::bar::g_open;
        drawEnd();
        usleep(vis ? 1500 : 4000);
    }

    shutdown();
    return 0;
}
