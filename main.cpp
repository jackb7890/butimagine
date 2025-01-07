#include <cstdio>
#include <windows.h>

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include "World.hpp"

void init() {
    SDL_Init(SDL_INIT_EVERYTHING);
}

void cleanup() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    init();
    Map map;
    map.SetStartMap();

    // not sure how I feel about the map updating through the player class but fuck it right
    Player player1(MAP_WIDTH/2, MAP_HEIGHT/2, &map);

    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN );
    if ( !win ) {
        printf("Failed to create a window! Error: %s\n", SDL_GetError());
    }

    Display display(win);
    printf("A\n");

    display.Update(map); // first draw of the map the screen (should include player initial pos)
    printf("A\n");
    Sleep(1500);
    printf("A\n");
    printf("Z\n");

    player1.MoveHoriz(50);
    display.Update(player1);
    Sleep(1500);

    player1.MoveVert(40);
    display.Update(player1);
    Sleep(1500);

    cleanup();
    return 0;
}


// stuff I made, and stopped using, but don't wanna throw away yet so I can reference it
namespace Junkyard {
    void DrawPlayer(Player player, SDL_Surface* winSurface) {
        SDL_Rect rect = SDL_Rect {player.position.x, player.position.y, player.width, player.height};
        SDL_FillRect( winSurface, &rect, SDL_MapRGB( winSurface->format, 255, 90, 120 ));
    }

    void InitSurface(Player player1, SDL_Surface* winSurface) {
        const int stride = 5;
        for (int i = 0; i < MAP_WIDTH; i+=stride) {
            for (int j = 0; j < MAP_HEIGHT; j+=stride) {
                SDL_Rect rect {i, j, stride, stride};
                SDL_FillRect(winSurface, &rect, i*MAP_HEIGHT + j);
            }
        }
        DrawPlayer(player1, winSurface);
    }
};