#include <cstdio>
#include <cstring>
#include <thread>
#include <memory>
#include <atomic>
#include <string>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <errno.h>
#include "imgui.h"

#define TOUCH_MODE 2

namespace touch {
    struct input {
        std::string path;
        int fd;
        bool canBeUsed{false};
        input_absinfo absX{}, absY{};
        float absXMultiplier{0}, absYMultiplier{0};
#if TOUCH_MODE == 2
        bool trackingIDPresent{false};
#endif
        std::atomic<bool> running{false};
        std::atomic<bool> grabbed{false};

        input(const char *_path, int32_t _screen_w, int32_t _screen_h) {
            path = _path;
            fd = open(_path, O_RDWR);
            if (fd == -1)
                return;
            canBeUsed = ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &absX) != -1 && ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &absY) != -1 && absX.maximum > 0 && absY.maximum > 0;
            if (!canBeUsed) {
                close();
                return;
            }

#if TOUCH_MODE == 2
            input_absinfo absTrackingID{};
            trackingIDPresent = ioctl(fd, EVIOCGABS(ABS_MT_TRACKING_ID), &absTrackingID) != -1 && absTrackingID.minimum <= 0 && absTrackingID.maximum > 0;
#endif

            calculateMultipliers(_screen_w, _screen_h);
            running.store(true);
        }

        input(const input&) = delete;
        input& operator=(const input&) = delete;

        void setGrab(bool enable) {
            if (!canBeUsed || fd == -1) return;
            int v = enable ? 1 : 0;
            if (ioctl(fd, EVIOCGRAB, v) != -1) {
                grabbed.store(enable);
            }
        }

        void close() {
            running.store(false);
            if (fd != -1) {
                ::close(fd);
                fd = -1;
            }
        }

        void calculateMultipliers(int32_t _screen_w, int32_t _screen_h) {
            auto max = fmax((float) absX.maximum, (float) absY.maximum);
            auto min = fmin((float) absX.maximum, (float) absY.maximum);
            absXMultiplier = (float) _screen_w / (_screen_w > _screen_h ? max : min);
            absYMultiplier = (float) _screen_h / (_screen_w > _screen_h ? min : max);
        }
    };

    uint8_t orientation;
    ImGuiIO *imgui_io;
    std::vector<std::unique_ptr<input>> inputs;

    void input_thread(input *in) {
        sleep(1);
        if (!imgui_io)
            imgui_io = &ImGui::GetIO();
        input_event events[64]{0};
        while (in && in->running.load() && in->fd != -1 && imgui_io && imgui_io->BackendRendererUserData) {
            auto event_readed_count = read(in->fd, events, sizeof(events));
            if (event_readed_count == -1) {
                if (errno == EINTR) continue;
                usleep(1000);
                continue;
            }
            if (event_readed_count == 0) {
                usleep(1000);
                continue;
            }
            event_readed_count /= (ssize_t) sizeof(input_event);

            static bool isDown = false;
            static float x = 0.0f, y = 0.0f;
            for (long j = 0; j < event_readed_count; j++) {
                auto &event = events[j];
                if (event.type == EV_ABS) {
#if TOUCH_MODE == 2
                    if (in->trackingIDPresent && event.code == ABS_MT_TRACKING_ID) {
                        isDown = event.value != -1;
                        continue;
                    }
#elif TOUCH_MODE == 0
                    if (event.code == ABS_MT_TRACKING_ID) {
                        isDown = event.value != -1;
                        continue;
                    }
#endif
                    if (event.code == ABS_MT_POSITION_X) {
                        x = (float) event.value;
                        continue;
                    }
                    if (event.code == ABS_MT_POSITION_Y) {
                        y = (float) event.value;
                        continue;
                    }
                }
                if (event.code == SYN_REPORT && imgui_io) {
#if TOUCH_MODE == 2
                    imgui_io->MouseDown[0] = in->trackingIDPresent ? isDown : event_readed_count > 2;
#elif TOUCH_MODE == 0
                    imgui_io->MouseDown[0] = isDown;
#else
                    imgui_io->MouseDown[0] = event_readed_count > 2;
#endif
                    if (imgui_io->MouseDown[0]) {
                        switch (orientation) {
                            case 1:
                                imgui_io->MousePos.x = y;
                                imgui_io->MousePos.y = in->absX.maximum - x;
                                break;
                            case 2:
                                imgui_io->MousePos.x = in->absX.maximum - x;
                                imgui_io->MousePos.y = in->absY.maximum - y;
                                break;
                            case 3:
                                imgui_io->MousePos.x = in->absY.maximum - y;
                                imgui_io->MousePos.y = x;
                                break;
                            default:
                                imgui_io->MousePos.x = x;
                                imgui_io->MousePos.y = y;
                                break;
                        }
                        imgui_io->MousePos.x *= in->absXMultiplier;
                        imgui_io->MousePos.y *= in->absYMultiplier;
                    }
                }
            }
        }
    }

    bool init(int32_t _screen_w, int32_t _screen_h, uint8_t _orientation) {
        orientation = _orientation;

        bool result = false;

        const char *path = "/dev/input/";
        auto dir = opendir(path);
        dirent *ptr;
        while ((ptr = readdir(dir)))
            if (strstr(ptr->d_name, "event")) {
                char event_path[128];
                sprintf(event_path, "%s%s", path, ptr->d_name);
                auto in = std::make_unique<input>(event_path, _screen_w, _screen_h);
                if (in->canBeUsed) {
                    result = true;
                    auto *p = in.get();
                    inputs.push_back(std::move(in));
                    std::thread(input_thread, p).detach();
                }
            }
        closedir(dir);

        return result;
    }

    void update(int32_t _screen_w, int32_t _screen_h, uint8_t _orientation) {
        orientation = _orientation;

        for (auto &in: inputs) {
            if (!in) continue;
            in->calculateMultipliers(_screen_w, _screen_h);
        }
    }

    void updateOrientation(uint8_t _orientation) {
        orientation = _orientation;
    }

    void setGrab(bool enable) {
        for (auto &in: inputs) {
            if (!in) continue;
            in->setGrab(enable);
        }
    }

    void shutdown() {
        setGrab(false);
        for (auto &in: inputs) {
            if (!in) continue;
            in->close();
        }
        inputs.clear();
        imgui_io = nullptr;
    }
}
