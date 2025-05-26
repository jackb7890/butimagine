#pragma once
#include <vector>
#include <assert.h>
#include <typeinfo>

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

int Wrap(int oldIndex, int change, int bound);

struct Log {
    template<typename First, typename ...Args>
    static void emit(First str, Args... args) {
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

#define RegisterTypeIndex(type, ind) \
template <class type>  \
int constexpr operator()() {\
    return ind;               \
}

template <typename T>
struct IndexOf{
    // poison
    template <typename T>
    static constexpr int operator()() {
        static_assert<false>();
    }

    RegisterTypeIndex(MapEntity, 0)
    RegisterTypeIndex(Wall, 1)
    RegisterTypeIndex(Player, 2)
};

template <typename W>
struct TypeDetails {
    static const int hash = typeid(W).hash_code();
    static const int index = IndexOf<W>();

    // poison
    template <typename T>
    static constexpr int IndexOf() {
        static_assert<false>();
    }

    template <class MapEntity>
    static constexpr int IndexOf() {
        return 0;
    }
    template <class Wall>
    int constexpr IndexOf() {
        return 1;
    }
    template <class Player>
    int constexpr IndexOf() {
        return 2;
    }
};