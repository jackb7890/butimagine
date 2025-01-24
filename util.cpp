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
        //A- This returns oldIndex + change, which is what newIndex already is.
        //A- Lmk if there's a reason why it shouldn't be return newIndex;
        return oldIndex + change;
    }
}

