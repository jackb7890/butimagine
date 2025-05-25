#pragma once

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include <vector>
#include <assert.h>
#include <typeinfo>
#include <string>

// SDL_event cheat sheet
/*
SDL_FIRSTEVENT	0x0000
SDL_QUIT	0x0100
SDL_APP_TERMINATING	0x0101
SDL_APP_LOWMEMORY	0x0102
SDL_APP_WILLENTERBACKGROUND	0x0103
SDL_APP_DIDENTERBACKGROUND	0x0104
SDL_APP_WILLENTERFOREGROUND	0x0105
SDL_APP_DIDENTERFOREGROUND	0x0106
SDL_LOCALECHANGED	0x0107
SDL_DISPLAYEVENT	0x0150
SDL_WINDOWEVENT	0x0200
SDL_SYSWMEVENT	0x0201
SDL_KEYDOWN	0x0300
SDL_KEYUP	0x0301
SDL_TEXTEDITING	0x0302
SDL_TEXTINPUT	0x0303
SDL_KEYMAPCHANGED	0x0304
SDL_TEXTEDITING_EXT	0x0305
SDL_MOUSEMOTION	0x0400
SDL_MOUSEBUTTONDOWN	0x0401
SDL_MOUSEBUTTONUP	0x0402
SDL_MOUSEWHEEL	0x0403
SDL_JOYAXISMOTION	0x0600
SDL_JOYBALLMOTION	0x0601
SDL_JOYHATMOTION	0x0602
SDL_JOYBUTTONDOWN	0x0603
SDL_JOYBUTTONUP	0x0604
SDL_JOYDEVICEADDED	0x0605
SDL_JOYDEVICEREMOVED	0x0606
SDL_JOYBATTERYUPDATED	0x0607
SDL_CONTROLLERAXISMOTION	0x0650
SDL_CONTROLLERBUTTONDOWN	0x0651
SDL_CONTROLLERBUTTONUP	0x0652
SDL_CONTROLLERDEVICEADDED	0x0653
SDL_CONTROLLERDEVICEREMOVED	0x0654
SDL_CONTROLLERDEVICEREMAPPED	0x0655
SDL_CONTROLLERTOUCHPADDOWN	0x0656
SDL_CONTROLLERTOUCHPADMOTION	0x0657
SDL_CONTROLLERTOUCHPADUP	0x0658
SDL_CONTROLLERSENSORUPDATE	0x0659
SDL_FINGERDOWN	0x0700
SDL_FINGERUP	0x0701
SDL_FINGERMOTION	0x0702
SDL_DOLLARGESTURE	0x0800
SDL_DOLLARRECORD	0x0801
SDL_MULTIGESTURE	0x0802
SDL_CLIPBOARDUPDATE	0x0900
SDL_DROPFILE	0x1000
SDL_DROPTEXT	0x1001
SDL_DROPBEGIN	0x1002
SDL_DROPCOMPLETE	0x1003
SDL_AUDIODEVICEADDED	0x1100
SDL_AUDIODEVICEREMOVED	0x1101
SDL_SENSORUPDATE	0x1200
SDL_RENDER_TARGETS_RESET	0x2000
SDL_RENDER_DEVICE_RESET	0x2001
SDL_POLLSENTINEL	0x7
SDL_USEREVENT	0x8000
SDL_LASTEVENT	0xFFFF
*/

// i mighta fucked up the orientation for this not sure

template<typename T>
struct Arr2d {
    size_t size;
    size_t iMax;
    size_t jMax;
    std::vector<T> arr;

    Arr2d() : iMax(0), jMax(0), size(0), arr(0) {}

    Arr2d(size_t _i, size_t _j) : iMax(_i), jMax(_j), size(_i*_j), arr(_i*_j) {}

    T& operator()(size_t i, size_t j) {
        assert(i < iMax && j < jMax);
        return arr[jMax*i + j];
    }

    T& operator()(GridPos pos) {
        assert(i < iMax && j < jMax);
        return arr[jMax*pos.x + pos.y];
    }

    T& operator[](GridPos pos) {
        assert(i < iMax && j < jMax);
        return arr[jMax*pos.x + pos.y];
    }

};

// util.cpp function declarations

int Wrap(int oldIndex, int change, int bound);
const char* MapEventIdToName(unsigned int id);

struct Log {

    std::string prefix1;
    int prefixLevel;

    static inline bool _loglevel = 1;

    Log() {
        prefixLevel = 0;
        prefix1 = "";
    };

    Log(std::string str) {
        prefix1 = str;
        prefixLevel = 1;
    }

    Log(const char * str) {
        prefix1 = std::string(str);
        prefixLevel = 1;
    }

    ~Log() {
        if (!prefixLevel) {
            Log::emit("%s End of Logger\n--------------------\n\n", prefix1.c_str());
        }
    }

    inline void SetPrefix(std::string str) {
        prefix1 = str;
        prefixLevel = 1;
    }

    inline void SetPrefix(const char * str) {
        prefix1 = std::string(str);
        prefixLevel = 1;
    }

    // Use me when logging through a specific Log object
    template<typename First, typename ...Args>
    void Emit(First str, Args... args) {
        if (prefixLevel) {
            Log::emit("%s", prefix1.c_str());
        }
        #if defined(LOG)
            Log::emit(str, args...);
        #else
            return;
        #endif
    }

    // Use me when logging through a Log object
    template<typename First, typename ...Args>
    void Error(First str, Args... args) {
        if (!prefixLevel) {
            Log::emit("%s", prefix1);
        }
        Log::emit(str, args...);
        #if defined(DEBUG)
            __debugbreak();
        #elif defined(QUIET_ERRORS)
            // just print and return
        #else
            abort();
        #endif
        return;
    }

    // Use me when logging you do not have a Log object (static)
    template<typename First, typename ...Args>
    static void emit(First str, Args... args) {
        if (!Log::_loglevel) {
            return;
        }
        #if defined(LOG)
            printf(str, args...);
        #elif defined(DBGBRK)
        #else
            return;
        #endif
    }

    template<typename First, typename ...Args>
    static void error(First str, Args... args) {
        printf(str, args...);
        #if defined(DEBUG)
            __debugbreak();
        #elif defined(QUIET_ERRORS)
            // just print and return
        #else
            abort();
        #endif
        return;
    }
};