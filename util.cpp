#include "util.hpp"

int Wrap(int oldIndex, int change, int bound) {
    int newIndex = oldIndex + change;
    if (newIndex >= bound) {
        return newIndex % bound;
    }
    else if (newIndex < 0) {
        return bound + (newIndex % bound);
    }
    else {
        return newIndex;
    }
}

