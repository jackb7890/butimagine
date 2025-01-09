#include <cstdio>
#include <array>
#include <windows.h>

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include "World.hpp"
#include "SDL_image.h"

void init() {
    IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG);
    SDL_Init(SDL_INIT_EVERYTHING);
}

void cleanup() {
    SDL_Quit();
}

bool HandleQuitEv(void) {
    return false;
}

SDL_Keycode HandleKeyDnEv(SDL_KeyboardEvent ev) {
    return ev.keysym.sym;
}

int main(int argc, char* argv[]) {
    init();

    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN );
    if ( !win ) {
        printf("Failed to create a window! Error: %s\n", SDL_GetError());
    }

    Display display(win);

    Map map;
    map.SetStartMap();

    // not sure how I feel about the map updating through the player class but fuck it right
    Player player1(MAP_WIDTH/2, MAP_HEIGHT/2, map);

    // short walls are 25 long
    // long walls on bot/top are 50 long
    // long back wall is 75 long

    Wall lowerFront = Wall(205, 255, 25, true, map);
    Wall bottom = Wall(155, 280, 50, false, map);
    Wall back = Wall(155, 205, 75, true, map);
    Wall top = Wall(155, 205, 50, false, map);
    Wall upperFront = Wall(205, 205, 25, true, map);

    display.Update(map); // first draw of the map the screen (should include player initial pos)

    display.Update(lowerFront);
    display.Update(bottom);
    display.Update(back);
    display.Update(top);
    display.Update(upperFront);

    bool runLoop = true;
    SDL_Event ev;
    const int speed = 20;
    while (runLoop) {
        while (SDL_PollEvent(&ev) != 0) {
            switch(ev.type) {
                case SDL_QUIT:
                    runLoop = HandleQuitEv();
                    break;
                case SDL_KEYDOWN:
                {
                    SDL_Keycode key = HandleKeyDnEv(ev.key);
                    switch (key) {
                        case SDLK_w:
                            player1.MoveVert(-1 * speed);
                            break;
                        case SDLK_a:
                            player1.MoveHoriz(-1 * speed);
                            break;
                        case SDLK_s:
                            player1.MoveVert(1 * speed);
                            break;
                        case SDLK_d:
                            player1.MoveHoriz(1 * speed);
                            break;
                        default:
                            break;
                    }
                }
                default:
                    break;
            }
            display.Update(player1);
        }

        SDL_Delay(100);
    }

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