#include <cstdio>
#include <array>
#include <windows.h>
#include <string>
#include <iostream>

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_image.h"
#include "time.h"

#include "TileMap.hpp"
#include "TextureManager.hpp"

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

    map.CreateBackground();

    HitBox player1HitBox = {MAP_WIDTH/2, MAP_HEIGHT/2, 10, 10};
    RGBColor player1Color = {120, 200, 200};
    Player player1(player1HitBox, player1Color, &map);
    HitBox tilehb = { 100, 100, 32, 32 };
    SDL_Texture* tiletex = TextureManager::LoadTexture("assets/tiles/grass.png", renderer);
    Tile tile1(tilehb, tiletex, 15);
    tile1.map = &map;
    tile1.valid = 1;
    std::cout << tile1.valid << std::endl;
    map.Add(player1);
    map.Add(tile1);



    // short walls are 25 long
    // long walls on bot/top are 50 long
    // long back wall is 75 long

    RGBColor wallColor = {170, 170, 170};

    Wall lowerFront = Wall({205, 255}, 25, true, wallColor, &map);
    map.Add(lowerFront);
    Wall bottom = Wall({155, 280}, 50, false, wallColor, &map);
    map.Add(bottom);
    Wall back = Wall({155, 205}, 75, true, wallColor, &map);
    map.Add(back);
    Wall top = Wall({155, 205}, 50, false, wallColor, &map);
    map.Add(top);
    Wall upperFront = Wall({205, 205}, 25, true, wallColor, &map);
    map.Add(upperFront);

    display.Update(); // this updates the map stored within display
    
    bool runLoop = true;
    SDL_Event ev;

    Uint32 totalFrames = 0;
    Uint32 FPSTicks = SDL_GetTicks();
    //A- Target fps = fps cap. Change if you want to change the fps
    const Uint32 TARGET_FPS = 60;
    const Uint32 TICKS_PER_FRAME = 1000 / TARGET_FPS;

    player1.X_velocity = 0.0;
    player1.Y_velocity = 0.0;
    int speed = 10;
    const float GRAVITY = 8.0f;

    //A- Speed cap, if you want to use it.
    bool capSpeed = TRUE;
    int speedCap = 300;

    //A- An array of bools to track the 4 directions a player can move
    enum Velocity {XPOS, YPOS, XNEG, YNEG};
    std::array<bool, 4> vels;
    vels.fill(false);

    while (runLoop) {
        //A- Timing starts at beginnig of core loop
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
            player1.X_velocity += speed;
        }
        if (vels[XNEG] && !vels[XPOS]) {
            player1.X_velocity -= speed;
        }
        if (vels[YPOS] && !vels[YNEG]) {
            player1.Y_velocity += speed;
        }
        if (vels[YNEG] && !vels[YPOS]) {
            player1.Y_velocity -= speed;
        }

        // deceleration combinations
        // slow down when theres no buttons being pressed

        if (!vels[XPOS] && !vels[XNEG]) {
            if (player1.X_velocity >= 0 && player1.X_velocity < GRAVITY) {
                player1.X_velocity = 0.0;
            }
            else {
                player1.X_velocity -= player1.X_velocity > 0 ? GRAVITY : -GRAVITY;
            }
        }
        if (!vels[YPOS] && !vels[YNEG]) {
            if (player1.Y_velocity >= 0 && player1.Y_velocity < GRAVITY) {
                player1.Y_velocity = 0.0;
            }
            else {
                player1.Y_velocity -= player1.Y_velocity > 0 ? GRAVITY : -GRAVITY;
            }
        }
        //A- This block supposedly keeps physics independent of the current FPS
        //A- Very important as otherwise faster FPS = faster movement (bad)
        //A- Doesn't seem to work, or doesn't work as much as intended, still figuring out why
        //A- Currently player still moves faster at a faster FPS, but maybe not by as much(?)
        Uint32 time = SDL_GetTicks();
        float dT = (time - player1.lastUpdate) / 1000.0f;

        //A- Move the player if velocity isn't 0
        if (player1.X_velocity != 0.0 || player1.Y_velocity != 0.0) {
            if (capSpeed) {
                if (player1.X_velocity > 0 && player1.X_velocity > speedCap) {
                    player1.X_velocity = speedCap;
                }
                if (player1.X_velocity < 0 && player1.X_velocity < -speedCap) {
                    player1.X_velocity = -speedCap;
                }
                if (player1.Y_velocity > 0 && player1.Y_velocity > speedCap) {
                    player1.Y_velocity = speedCap;
                }
                if (player1.Y_velocity < 0 && player1.Y_velocity < -speedCap) {
                    player1.Y_velocity = -speedCap;
                }
            }
            //A- I saw you added move() instead of move horz/vert so I changed this
            //A- Side effect - you can't slide against walls, hitting any wall will stop all movement
            player1.Move(player1.X_velocity * dT, player1.Y_velocity * dT);
        }
        //A- Re-render the player
        display.Update(player1);
        //A- Update for DT
        player1.lastUpdate = SDL_GetTicks();

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