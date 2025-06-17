#pragma once

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include <vector>
#include <assert.h>
#include <typeinfo>

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