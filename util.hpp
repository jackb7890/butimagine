#pragma once

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include <vector>
#include <assert.h>
#include <typeinfo>

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

struct GridPos {
    int x, y;

    inline GridPos() : x(0), y(0) {}
    inline GridPos(int _x, int _y) : x(_x), y(_y) {}
};

struct GridDimension {
    int width;
    int depth;

    inline GridDimension() : width(0), depth(0) {}
    inline GridDimension(int _w, int _d) : width(_w), depth(_d) {}
};

struct HitBox {
    GridPos origin;
    GridDimension dim;

    inline HitBox() :
        origin(0, 0), dim(1, 1) {}
    inline HitBox(GridPos _pos, GridDimension _dim) :
        origin(_pos), dim(_dim) {}
    inline HitBox(int _x, int _y, int _w, int _d) :
        origin(_x, _y), dim(_w, _d) {}
};

class RGBColor {
    public:
    int r;
    int g;
    int b;

    inline RGBColor() :
        r(255), g(255), b(255) {}
    inline RGBColor(int _r, int _g, int _b) :
        r(_r), g(_g), b(_b) {
        // eventually make the asserts debug only so they don't slow down the program
        assert(0 <= r && r <= 255);
        assert(0 <= g && g <= 255);
        assert(0 <= b && b <= 255);

    }
    unsigned ConvertToSDL(SDL_Surface* surface);
};

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

    T& operator()(GridPos gp) {
        int i = gp.x;
        int j = gp.y;
        assert(i < iMax && j < jMax);
        return arr[jMax*i + j];
    }

    T& operator[](GridPos gp) {
        int i = gp.x;
        int j = gp.y;
        assert(i < iMax && j < jMax);
        return arr[jMax*i + j];
    }
};

int Wrap(int oldIndex, int change, int bound);

struct Log {
    template<typename First, typename ...Args>
    static void emit(First str, Args... args) {
        #if defined(LOG)
            printf(str, args...);
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

// Weird compile-time template bullshit
// End result is the RegisterClassIndex macro
// effectively registers a type with a id.
// The id is available at compile-time and runtime
// through the TypeDetails<> templated struct and its member 'index'.
// E.g. `cout << TypeDetails<Player>::index` would print 3.
// I'm using it to serialize our types and send them over network
// and you need a pre-determined code to build the correct type back up.
template <typename T>
struct IndexOf {
    static const int value = -1;
    constexpr IndexOf() {
        static_assert(false);
    }
};

#define RegisterClassIndex(type, index) \
class type; \
template <> \
struct IndexOf <type> { \
    static const int value = index; \
};

RegisterClassIndex(MapEntity, 1);
RegisterClassIndex(Wall, 2);
RegisterClassIndex(Player, 3);

template <typename T>
struct TypeDetails {
    static const int index = IndexOf<T>::value;
};

template <class T>
concept IndexRegisteredT = TypeDetails<T>::index > 0;