#include "../../internal/Android_draw/draw.h"
#include "../../internal/ImGui/backends/imgui_impl_opengl3.h"
#include "../../internal/ImGui/backends/imgui_impl_android.h"
#include "../../internal/Android_touch/Touch.hpp"
#include "../../fonts/esp/esp.h"

#include "../../../src/ui/cfg.hpp"

EGLDisplay display = EGL_NO_DISPLAY;
EGLConfig config;
EGLSurface surface = EGL_NO_SURFACE;
EGLContext context = EGL_NO_CONTEXT;

ANativeWindow *native_window;
ImFont* fontBold;
ImFont* fontMedium;
ImFont* fontDesc;
ImFont* espFont;

int native_window_screen_x = 0;
int native_window_screen_y = 0;
android::ANativeWindowCreator::DisplayInfo displayInfo{0};
uint32_t orientation = 0;
bool g_Initialized = false;
ImGuiWindow *g_window = nullptr;

bool initGUI_draw(uint32_t _screen_x, uint32_t _screen_y, bool log) {
    orientation = displayInfo.orientation;

    if (!init_egl(_screen_x, _screen_y, log)) {
        return false;
    }

    if (!ImGui_init()) {
        return false;
    }

    return true;
}

bool init_egl(uint32_t _screen_x, uint32_t _screen_y, bool log) {
    ::native_window = android::ANativeWindowCreator::Create("Overlay", _screen_x, _screen_y, cfg::misc::stream_proof);

    ANativeWindow_acquire(native_window);
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        return false;
    }

    if (eglInitialize(display, 0, 0) != EGL_TRUE) {
        return false;
    }

    EGLint num_config = 0;
    const EGLint attribList[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_RED_SIZE, 5,
        EGL_BUFFER_SIZE, 32,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    const EGLint attrib_list[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        3,
        EGL_NONE
    };

    if (eglChooseConfig(display, attribList, &config, 1, &num_config) != EGL_TRUE) {
        return false;
    }

    EGLint egl_format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &egl_format);
    ANativeWindow_setBuffersGeometry(native_window, 0, 0, egl_format);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    if (context == EGL_NO_CONTEXT) {
        return false;
    }

    surface = eglCreateWindowSurface(display, config, native_window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        return false;
    }

    return true;
}

void screen_config() {
    displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

bool ImGui_init() {
    if (g_Initialized) {
        return true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_Init(native_window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;

    fontBold = io.Fonts->AddFontFromMemoryTTF(esp, sizeof(esp), 32, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    fontMedium = io.Fonts->AddFontFromMemoryTTF(esp, sizeof(esp), 28, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    fontDesc = io.Fonts->AddFontFromMemoryTTF(esp, sizeof(esp), 22, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.FontDefault = fontMedium;

    espFont = io.Fonts->AddFontFromMemoryTTF(esp, sizeof(esp), 18, nullptr, io.Fonts->GetGlyphRangesCyrillic());

    ImGui::GetStyle().ScaleAllSizes(3.0f);
    ::g_Initialized = true;
    return true;
}

void drawBegin() {
    screen_config();
    if (::orientation != displayInfo.orientation) {
        ::orientation = displayInfo.orientation;
        touch::update(displayInfo.width, displayInfo.height, displayInfo.orientation);
        if (g_window) {
            g_window->Pos.x = 100;
            g_window->Pos.y = 125;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(native_window_screen_x, native_window_screen_y);
    ImGui::NewFrame();
}

void drawEnd() {
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(display, surface);
}

void setBlurRadius(int radius) {
    if (native_window) {
        android::ANativeWindowCreator::SetBlurRadius(native_window, radius);
    }
}

void updateStreamProof(bool enable) {
    if (native_window && g_Initialized) {
        android::ANativeWindowCreator::SetSecure(native_window, enable);
    }
}

void setInputPassThrough(bool enable) {
    if (native_window && g_Initialized) {
        touch::setGrab(!enable);
    }
}

bool isBlurSupported() {
    return android::ANativeWindowCreator::SupportsBlur();
}

#include <vector>
#include <random>

struct Snowflake {
    float x, y;
    float size;
    float speed;
    float parallax;
    float opacity;
};

static std::vector<Snowflake> snowflakes;
static void initSnow() {
    if (!snowflakes.empty()) return;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_x(0, 3000); 
    std::uniform_real_distribution<float> dist_y(0, 2000);
    std::uniform_real_distribution<float> dist_size(1.0f, 3.5f);
    std::uniform_real_distribution<float> dist_speed(20.0f, 60.0f);
    std::uniform_real_distribution<float> dist_para(0.2f, 1.0f);
    
    for (int i = 0; i < 400; i++) {
        snowflakes.push_back({dist_x(gen), dist_y(gen), dist_size(gen), dist_speed(gen), dist_para(gen), 0.0f});
    }
}

void drawSnow(float alpha) {
    if (alpha <= 0.0f) return;
    initSnow();
    
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    float dt = ImGui::GetIO().DeltaTime;
    
    for (auto& s : snowflakes) {
        s.y += s.speed * s.parallax * dt;
        s.x += sinf(ImGui::GetTime() * s.parallax) * 10.0f * dt;
        
        if (s.y > native_window_screen_y) {
            s.y = -10.0f;
            s.x = (float)(rand() % native_window_screen_x);
        }
        
        ImVec2 pos(s.x, s.y);
        draw_list->AddCircleFilled(pos, s.size, IM_COL32(255, 255, 255, (int)(alpha * 180 * s.parallax)));
    }
}

void drawBackgroundDim(float alpha) {
    if (alpha <= 0.0f) return;
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)native_window_screen_x, (float)native_window_screen_y));
    
    if (ImGui::Begin("##BackgroundDim", nullptr, window_flags)) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(ImVec2(0, 0), ImVec2((float)native_window_screen_x, (float)native_window_screen_y), 
                                 IM_COL32(0, 0, 0, (int)(alpha * 255)));
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}

int getSDKVersion() {
    return android::ANativeWindowCreator::GetSDKVersion();
}

void shutdown() {
    if (!g_Initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;

    ANativeWindow_release(native_window);
    android::ANativeWindowCreator::Destroy(native_window);
    ::g_Initialized = false;
}
