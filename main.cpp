#include <vector>
#include <cstdio>
#include <windows.h>

#define SDL_MAIN_HANDLED 1
#include "SDL.h"


// what to do first

// i need a map
class Map {
    typedef std::vector<std::vector<int>> vector2d;
    vector2d grid;
    // npcs

    // walls
};

struct GridPos {
    int i, j;

    GridPos() : i(0), j(0) {}
    GridPos(int _i, int _j) : i(_i), j(_i) {}
};

class Player {
    GridPos position;
    int health;
    int runEnergy;

    Player() : position(0, 0), health(0), runEnergy(0) {}
};

void init() {
    printf("Doing SDL_Init\n");
    SDL_Init(SDL_INIT_EVERYTHING);
}

// int main() {
//     init();
//     return 0;
// }

int main(int argc, char* argv[]) {
    init();

    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, 640, 480, SDL_WINDOW_SHOWN );

    if ( !win ) {
        printf("Failed to create a window! Error: %s\n", SDL_GetError());
    }

    SDL_Surface* winSurface = SDL_GetWindowSurface( win );

    // do drawing
    SDL_FillRect( winSurface, NULL, SDL_MapRGB( winSurface->format, 90, 255, 120 ));

    SDL_UpdateWindowSurface( win );

    printf("starting sleep\n");
    Sleep(5000);
    printf("OUT OF sleep\n");

    SDL_DestroyWindow( win );
    win = NULL;
    winSurface = NULL;

    SDL_Quit();
    return 0;
}