#pragma once
#include <vector>
#include <assert.h>

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

};

int Wrap(int oldIndex, int change, int bound);

struct Log {
    template<typename First, typename ...Args>
    static void emit(First str, Args... args) {
        #ifdef LOG
            printf(str, args...);
        #else
            return;
        #endif
    }
};