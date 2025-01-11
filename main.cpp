#include <cstdio>
#include <array>
#include <windows.h>
#include <string>
#include <iostream>

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_image.h"
#include "time.h"

#include "World.hpp"

using namespace std;

void init() {
    IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG);
    SDL_Init(SDL_INIT_EVERYTHING);
    //A- Note the global timer starts at SDL initilization
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
    player1.born = SDL_GetTicks();

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

    //A- Using "gravity" as a constant deceleration force
    const float GRAVITY = 1.0f;

    Uint32 totalFrames = 0;
    Uint32 FPSTicks = SDL_GetTicks();
    //A- Change target fps = fps cap. Change if you want to change the fps
    const Uint32 TARGET_FPS = 20;
    const Uint32 TICKS_PER_FRAME = 1000 / TARGET_FPS;

    //A- Velocity like position should be tracked by the player struct but i'm not doing that rn
    float tempXelocity = 0.0f;
    float tempYelocity = 0.0f;

    while (runLoop) {
        //A- Timing starts at beginnig of core loop
        //A- SDL_GetTicks() is the global timer in ms
        Uint32 startTick = SDL_GetTicks();
        //A- A "Frame" is just one itteration of the core game loop. So FPS is just how many times the game is looping per second.
        //A- Since this is the start of the (main) loop we can increase frames by 1 then use that number later to get our FPS
        //A- Disabled cause the FPS trackers are broken
        while (SDL_PollEvent(&ev) != 0) {
            switch (ev.type) {
            case SDL_QUIT:
                runLoop = HandleQuitEv();
                break;
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = HandleKeyDnEv(ev.key);
                switch (key) {
                    //A- Since (I) want the player to be moving without direct input, WASD changes the player's velocity instead of their position
                case SDLK_w:
                    tempYelocity -= 5.0f;
                    break;
                case SDLK_a:
                    tempXelocity -= 5.0f;
                    break;
                case SDLK_s:
                    tempYelocity += 5.0f;
                    break;
                case SDLK_d:
                    tempXelocity += 5.0f;
                    break;
                default:
                    break;
                }
            }
            default:
                break;
            }
        }
        float avgFPS = totalFrames / (((Uint32)SDL_GetTicks() - FPSTicks) / 1000.f);
        if (avgFPS > 2000000)
        {
            avgFPS = 0;
        }
        //A- Movement needs to happen independent of inputs, so another ""loop"" to move the player is needed
        //A- Note that when I made this an actual loop instead of an if it didn't really work as intended
        //A- Movement should also only happen if there's an actual reason to move (velocity not being 0)
        if (tempXelocity != 0 || tempYelocity != 0) {

            //A- This block supposedly keeps physics independent of the current FPS
            //A- Very important as otherwise faster FPS = faster gravity (bad)
            //A- That said this wasn't actually needed & just made things inconsistant.
            //Uint32 time = SDL_GetTicks();
            //float dT = (time - player1.lastUpdate) / 1000.0f;
            float deceleration = GRAVITY /* * dT */;
            player1.MoveVert(static_cast<int>(tempYelocity));
            player1.MoveHoriz(static_cast<int>(tempXelocity));

            //A- The problem with x y velocity is we're dealing with + and - numbers
            //A- Deceleration variable will always be a positive number
            //A- We need Deceleration to always work towards 0, so that no matter what direction we're moving we slow down.
            //A- While also making sure that we don't cross over 0 in our calculations
            //A- (Like 5 velocity - 10 deceleration would equal -5 velocity, we don't want that)

            //A- So first check if we would overshoot by adding deceleration
            if ((tempXelocity + deceleration) < 0) {
                //A- If we're still under 0 after adding, we can safely add
                tempXelocity += deceleration;
            }
            //A- Same for subtracting from a positive velocity
            else if ((tempXelocity - deceleration) > 0) {
                tempXelocity -= deceleration;
            }
            //A- If either case would overshoot, just set it to 0
            else {
                tempXelocity = 0;
            }
            //A- Repeat for Y axis
            if ((tempYelocity + deceleration) < 0) {
                tempYelocity += deceleration;
            }
            else if ((tempYelocity - deceleration) > 0) {
                tempYelocity -= deceleration;
            }
            else {
                tempYelocity = 0;
            }
            //A- Update for (disabled) dT
            //player1.lastUpdate = time;
        }
        display.Update(player1);
        totalFrames++;
        Uint32 endTick = SDL_GetTicks();
        //A- If frame finished early (which it will)
        int frameTicks = endTick - startTick;
        if (frameTicks < TICKS_PER_FRAME){
            //A- Wait according to FPS cap
            SDL_Delay(TICKS_PER_FRAME - frameTicks);
        }
        cout << avgFPS << endl;
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