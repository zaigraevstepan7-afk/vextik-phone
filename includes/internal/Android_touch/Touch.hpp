#pragma once

#include <cstdint>

namespace touch {
    bool init(int32_t _screen_w, int32_t _screen_h, uint8_t _orientation);
    void update(int32_t _screen_w, int32_t _screen_h, uint8_t _orientation);
    void updateOrientation(uint8_t _orientation);
    void setGrab(bool enable);
    // Drain queued touch events into ImGui. MUST be called on the main/render
    // thread right before ImGui::NewFrame() so presses are delivered through
    // ImGui's event queue (no lost taps, no cross-thread races).
    void flush();
    void shutdown();
}

