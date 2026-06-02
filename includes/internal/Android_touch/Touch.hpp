#pragma once

#include <cstdint>

namespace touch {
    bool init(int32_t _screen_w, int32_t _screen_h, uint8_t _orientation);
    void update(int32_t _screen_w, int32_t _screen_h, uint8_t _orientation);
    void updateOrientation(uint8_t _orientation);
    void setGrab(bool enable);
    void shutdown();

    // Region based passthrough: the touchscreen is only grabbed for the
    // duration of a gesture that starts inside one of the UI regions below.
    // Every other touch is left untouched so the game receives it.
    // Coordinates are in ImGui screen space (same space as MousePos).
    void setBarRegion(bool valid, float x0, float y0, float x1, float y1);
    void setMenuRegion(bool valid, float x0, float y0, float x1, float y1);
}
