#pragma once

#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "oxorany.hpp"

namespace protect {

    class Security {
    public:
        static void run();

    private:
        static void monitor_loop();
        static bool check_debugger();
        static bool check_frida();
        static bool check_ports();
        static void erase_header();
        static bool check_timing();
        static bool check_filename();
        static bool check_integrity();
        static bool check_environment();   
        static bool check_processes();  
        static void trigger_defense(const char* reason);
        static std::string read_file(const char* path);
    };

}
