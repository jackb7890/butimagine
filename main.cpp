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

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

void init() {
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("My window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MAP_WIDTH, MAP_HEIGHT, NULL);
    if (!window) {
        printf("Failed to create window! Error: %s\n", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer! Error: %s\n", SDL_GetError());
    }
    SDL_RenderClear(renderer);
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    renderer = NULL;
    window = NULL;
}

bool HandleQuitEv(void) {
    return false;
}

SDL_Keycode HandleKeyDnEv(SDL_KeyboardEvent ev) {
    return ev.keysym.sym;
}

int main(int argc, char* argv[]) {
    init();

    Map map;
    Display display(window, renderer, &map);

    HitBox dummyHB = { 0, 0, MAP_WIDTH, MAP_HEIGHT };
    RGBColor dummyC = { 40, 40, 40 };
    MapEntity* dummyBG = new MapEntity(dummyHB, dummyC, &map, false, true);
    
    HitBox testHB = { MAP_WIDTH / 3, MAP_HEIGHT / 2, 100, 100 };
    RGBColor testC = { 0, 200, 0 };
    MapEntity* testEntityOver = new MapEntity(testHB, testC, &map, false, true);
    
    HitBox testHB1 = { MAP_WIDTH / 3 + 100, MAP_HEIGHT / 2, 100, 100 };
    RGBColor testC1 = { 200, 200, 0 };
    MapEntity* testEntityUnder = new MapEntity(testHB1, testC1, &map, false, true);

    HitBox testHB2 = { MAP_WIDTH / 3 - 100, MAP_HEIGHT / 2, 100, 100 };
    RGBColor testC2 = { 0, 50, 200 };
    MapEntity* testEntityCol = new MapEntity(testHB2, testC2, &map, true, true);

    HitBox player1HitBox = {MAP_WIDTH/2, MAP_HEIGHT/2, 15, 15};
    RGBColor player1Color = {100, 100, 255};
    Player* player1 = new Player(player1HitBox, player1Color, &map);

    map.AddEntity(dummyBG);
    map.AddEntity(testEntityOver);
    map.AddEntity(player1);
    map.AddEntity(testEntityUnder);
    map.AddEntity(testEntityCol);

    display.Update(); // this updates the map stored within display
    
    bool runLoop = true;
    SDL_Event ev;

    Uint32 totalFrames = 0;
    Uint32 FPSTicks = SDL_GetTicks();
    //A- Target fps = fps cap. Change if you want to change the fps
    const Uint32 TARGET_FPS = 30;
    const Uint32 TICKS_PER_FRAME = 1000 / TARGET_FPS;

    player1->X_velocity = 0.0;
    player1->Y_velocity = 0.0;
    int speed = 15;
    const float GRAVITY = 10.0f;

    //A- Speed cap, if you want to use it.
    bool capSpeed = TRUE;
    int speedCap = 300;

    //A- An array of bools to track the 4 directions a player can move
    enum Velocity {XPOS, YPOS, XNEG, YNEG};
    std::array<bool, 4> vels;
    vels.fill(false);

    while (runLoop) {
        //A- Timing starts at beginning of core loop
        //A- SDL_GetTicks() is the global timer in ms
        Uint32 startTick = SDL_GetTicks();
        while (SDL_PollEvent(&ev) != 0) {
            switch (ev.type) {
            case SDL_QUIT:
                runLoop = HandleQuitEv();
                break;
            //A- Instead of directly increasing/decreasing velocity, pressing a key sets a flag to true, while releasing it sets the flag to false
            //A- We had an issue before where holding a key down didn't register until it's been held for a full second
            //A- With this method, A key is considred to always be held down until a key up input is read
            //A- Changing directions and holding multiple directions now works as intended
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = HandleKeyDnEv(ev.key);
                switch (key) {
                case SDLK_w:
                    vels[YNEG] = true;
                    break;
                case SDLK_a:
                    vels[XNEG] = true;
                    break;
                case SDLK_s:
                    vels[YPOS] = true;
                    break;
                case SDLK_d:
                    vels[XPOS] = true;
                    break;
                default:
                    break;
                }
            }
            break;
            case SDL_KEYUP:
            {
                SDL_Keycode key = HandleKeyDnEv(ev.key);
                switch (key) {
                case SDLK_w:
                    vels[YNEG] = false;
                    break;
                case SDLK_a:
                    vels[XNEG] = false;
                    break;
                case SDLK_s:
                    vels[YPOS] = false;
                    break;
                case SDLK_d:
                    vels[XPOS] = false;
                    break;
                default:
                    break;
                }
            }
            break;
            default:
                break;
            }
        }
        //A- Calc the average FPS, useful for debugging the framerate cap
        //A- Very first frame will be way higher than it should be, which is what the if corrects.
        float avgFPS = totalFrames / (((Uint32)SDL_GetTicks() - FPSTicks) / 1000.f);
        if (avgFPS > 2000000)
        {
            avgFPS = 0;
        }

        //A- Changes velocity accoring to what buttons are pressed
        if (vels[XPOS] && !vels[XNEG]) {
            player1->X_velocity += speed;
        }
        if (vels[XNEG] && !vels[XPOS]) {
            player1->X_velocity -= speed;
        }
        if (vels[YPOS] && !vels[YNEG]) {
            player1->Y_velocity += speed;
        }
        if (vels[YNEG] && !vels[YPOS]) {
            player1->Y_velocity -= speed;
        }

        // deceleration combinations
        // slow down when theres no buttons being pressed

        if (!vels[XPOS] && !vels[XNEG]) {
            if (player1->X_velocity >= 0 && player1->X_velocity < GRAVITY) {
                player1->X_velocity = 0.0;
            }
            else {
                player1->X_velocity -= player1->X_velocity > 0 ? GRAVITY : -GRAVITY;
            }
        }
        if (!vels[YPOS] && !vels[YNEG]) {
            if (player1->Y_velocity >= 0 && player1->Y_velocity < GRAVITY) {
                player1->Y_velocity = 0.0;
            }
            else {
                player1->Y_velocity -= player1->Y_velocity > 0 ? GRAVITY : -GRAVITY;
            }
        }
        //A- This block supposedly keeps physics independent of the current FPS
        //A- Very important as otherwise faster FPS = faster movement (bad)
        //A- Doesn't seem to work, or doesn't work as much as intended, still figuring out why
        //A- Currently player still moves faster at a faster FPS, but maybe not by as much(?)

        //A- Update^ Player seemingly moves faster/slower because inputs/velocity changes are also limited by framerate.
        //A- So both the "physics" and rendering are limited by framerate, when it's just supposed to be rendering.
        //A- No fix currently
        Uint32 time = SDL_GetTicks();
        float dT = (time - player1->lastUpdate) / 1000.0f;

        //A- Move the player if velocity isn't 0
        if (player1->X_velocity != 0.0 || player1->Y_velocity != 0.0) {
            if (capSpeed) {
                if (player1->X_velocity > speedCap) {
                    player1->X_velocity = speedCap;
                }
                if (player1->X_velocity < -speedCap) {
                    player1->X_velocity = -speedCap;
                }
                if (player1->Y_velocity > speedCap) {
                    player1->Y_velocity = speedCap;
                }
                if (player1->Y_velocity < -speedCap) {
                    player1->Y_velocity = -speedCap;
                }
            }
            player1->Move(player1->X_velocity * dT, player1->Y_velocity * dT);
        }
        //A- Re-render the player
        display.Update();
        //A- Update for DT
        player1->lastUpdate = SDL_GetTicks();

        std::cout << player1->oldPos.x << std::endl;

        //A- End of loop, increase frame counter & get end time.
        totalFrames++;
        Uint32 endTick = SDL_GetTicks();

        //A- If frame finished earlier than FPS cap (which it will)
        int frameTicks = endTick - startTick;
        if (frameTicks < TICKS_PER_FRAME){
            //A- Delay next frame according to FPS cap
            SDL_Delay(TICKS_PER_FRAME - frameTicks);
        }
    }
    cleanup();
    return 0;
}

// stuff I made, and stopped using, but don't wanna throw away yet so I can reference it
// namespace Junkyard {
//     void DrawPlayer(Player player, SDL_Surface* winSurface) {
//         SDL_Rect rect = SDL_Rect {player.position.x, player.position.y, player.width, player.height};
//         SDL_FillRect( winSurface, &rect, SDL_MapRGB( winSurface->format, 255, 90, 120 ));
//     }

//     void InitSurface(Player player1, SDL_Surface* winSurface) {
//         const int stride = 5;
//         for (int i = 0; i < MAP_WIDTH; i+=stride) {
//             for (int j = 0; j < MAP_HEIGHT; j+=stride) {
//                 SDL_Rect rect {i, j, stride, stride};
//                 SDL_FillRect(winSurface, &rect, i*MAP_HEIGHT + j);
//             }
//         }
//         DrawPlayer(player1, winSurface);
//     }
// };