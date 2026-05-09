#include "security.hpp"
#include <thread>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <elf.h>
#include <link.h>
#include <dlfcn.h>
#include <android/log.h>
#include <cstdarg>
#include <cstdio>

namespace protect {

    static void append_txtf(const char* tag, const char* fmt, ...) {
        return;
    }

    void Security::run() {
        // protection thread disabled
        return;
    }

    void Security::monitor_loop() {
        append_txtf(oxorany("security"), oxorany("monitor_loop started"));
        static int slow_tick = 0;
        while (true) {
            if (check_debugger()) trigger_defense(oxorany("check_debugger"));
            if (check_frida()) trigger_defense(oxorany("check_frida"));
            if (check_ports()) trigger_defense(oxorany("check_ports"));
            if (check_timing()) trigger_defense(oxorany("check_timing"));
            if (check_integrity()) trigger_defense(oxorany("check_integrity"));
            if (slow_tick++ >= 5) {
                if (check_filename()) trigger_defense(oxorany("check_filename"));
                if (check_environment()) trigger_defense(oxorany("check_environment"));
                if (check_processes()) trigger_defense(oxorany("check_processes"));
                slow_tick = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 + (rand() % 500)));
        }
    }

    std::string Security::read_file(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }

    bool Security::check_debugger() {
        std::string status = read_file(oxorany("/proc/self/status"));
        if (status.empty()) return false;

        std::string tracer_pid_str = oxorany("TracerPid:");
        size_t pos = status.find(tracer_pid_str);
        if (pos != std::string::npos) {
            size_t value_pos = pos + tracer_pid_str.length();
            int tracer_pid = 0;
            while (value_pos < status.length() && (status[value_pos] == ' ' || status[value_pos] == '\t')) {
                value_pos++;
            }
            
            if (value_pos < status.length()) {
                tracer_pid = std::atoi(&status[value_pos]);
                if (tracer_pid != 0) {
                    append_txtf(oxorany("check_debugger"), oxorany("TracerPid=%d"), tracer_pid);
                    return true;
                }
            }
        }
        return false;
    }

    bool Security::check_frida() {
        std::string maps = read_file(oxorany("/proc/self/maps"));
        if (maps.empty()) return false;
        const char* suspicious[] = {
            oxorany("frida-agent"),
            oxorany("frida-gadget"),
            oxorany("gum-js-loop"),
            oxorany("linjector"),
            oxorany("riru"),
            oxorany("zygisk")
        };

        for (const char* pattern : suspicious) {
            if (maps.find(pattern) != std::string::npos) {
                append_txtf(oxorany("check_frida"), oxorany("maps match: %s"), pattern);
                return true;
            }
        }
        
        return false;
    }

    bool Security::check_ports() {
        std::string tcp = read_file(oxorany("/proc/net/tcp"));
        if (tcp.empty()) return false;
        
        if (tcp.find(oxorany(":69A2")) != std::string::npos) {
             append_txtf(oxorany("check_ports"), oxorany("found :69A2 in /proc/net/tcp"));
             return true;
        }

        return false;
    }

    bool Security::check_timing() {
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile int x = 0;
        for (int i = 0; i < 1000; i++) {
            x += i;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        if (duration > 10000) { 
            append_txtf(oxorany("check_timing"), oxorany("duration_us=%lld"), duration);
            return true;
        }
        return false;
    }

    bool Security::check_filename() {
        std::string cmdline = read_file(oxorany("/proc/self/cmdline"));
        if (cmdline.empty()) return false;
        size_t first_null = cmdline.find('\0');
        std::string proc_name = (first_null != std::string::npos) ? cmdline.substr(0, first_null) : cmdline;

        size_t last_slash = proc_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            proc_name = proc_name.substr(last_slash + 1);
        }

        bool name_match = (proc_name == oxorany("HASLINE") || proc_name == oxorany("HASLINE.sh"));
        
        if (!name_match) {
            append_txtf(oxorany("check_filename"), oxorany("process=%s"), proc_name.c_str());
            return true;
        }

        return false;
    }

    bool Security::check_integrity() {
        
        #if defined(__x86_64__) || defined(__i386__)
            unsigned char* funcs[] = { 
                (unsigned char*)monitor_loop, 
                (unsigned char*)check_debugger, 
                (unsigned char*)erase_header 
            };
            
            for (auto f : funcs) {
                for (int i = 0; i < 16; i++) {
                    if (f[i] == 0xCC) {
                        append_txtf(oxorany("check_integrity"), oxorany("x86 bp at %p"), f + i);
                        return true;
                    }
                }
            }
        #elif defined(__aarch64__)
            unsigned int* funcs[] = { 
                (unsigned int*)monitor_loop, 
                (unsigned int*)check_debugger, 
                (unsigned int*)erase_header 
            };
             for (auto f : funcs) {
                for (int i = 0; i < 4; i++) {
                    if (f[i] == 0xD4200000) {
                        append_txtf(oxorany("check_integrity"), oxorany("arm64 brk at %p"), f + i);
                        return true;
                    }
                }
            }
        #endif

        return false;
    }

    bool Security::check_environment() {
        const char* preload = getenv(oxorany("LD_PRELOAD"));
        if (preload != NULL && strlen(preload) > 0) {
            append_txtf(oxorany("check_environment"), oxorany("LD_PRELOAD=%s"), preload);
            return true;
        }
        return false;
    }

    bool Security::check_processes() {
        DIR* dir = opendir(oxorany("/proc"));
        if (!dir) return false;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
                char cmdline_path[64];
                snprintf(cmdline_path, sizeof(cmdline_path), oxorany("/proc/%s/cmdline"), entry->d_name);
                
                std::string cmdline = read_file(cmdline_path);
                if (cmdline.empty()) continue;

                const char* blacklist[] = {
                    oxorany("frida-server"),
                    oxorany("android_server"),
                    oxorany("gdbserver"),
                    oxorany("lldb-server"),
                    oxorany("com.topjohnwu.magisk"),
                    oxorany("org.cheatengine"),
                    oxorany("catch_.me_if_you_can")
                };

                for (const char* tool : blacklist) {
                    if (cmdline.find(tool) != std::string::npos) {
                        append_txtf(oxorany("check_processes"), oxorany("tool=%s pid=%s"), tool, entry->d_name);
                        closedir(dir);
                        return true;
                    }
                }
            }
        }
        closedir(dir);
        return false;
    }

    void Security::erase_header() {
        
        #if defined(__aarch64__) || defined(__arm__) || defined(__x86_64__) || defined(__i386__)
            Dl_info info;
            if (dladdr((void*)erase_header, &info) && info.dli_fbase) {
                void* base_addr = info.dli_fbase;
                size_t page_size = sysconf(_SC_PAGESIZE);
            
                if (mprotect(base_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {

                    std::memset(base_addr, 0, 16); 
                    
                    mprotect(base_addr, page_size, PROT_READ | PROT_EXEC);
                }
            }
        #endif
    }

    void Security::trigger_defense(const char* reason) {
        _exit(0);
    }
}
