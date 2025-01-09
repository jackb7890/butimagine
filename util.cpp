#include "util.hpp"
#include <assert.h>

template<typename T>
T Arr2d::operator()<T<(size_t i, size_t j) {
    assert(i < iMax && j < jMax);
    return arr[jMax*i + j];
}