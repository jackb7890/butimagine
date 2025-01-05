#include <array>
#include <cstdio>
#include <windows.h>

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include "World.hpp"


// what to do first

// i need a map


// struct Map;

// struct GridPos {
//     int x, y;

//     GridPos() : x(0), y(0) {}
//     GridPos(int _x, int _y) : x(_x), y(_y) {}
// };

// struct Player {
//     private:
//     // Currently the way it works is player has 2 positions.
//     // the second position is to store the players updated position
//     // That way we can keep track of the old position and remove it from the screen
//     // There will be problems if you move the player multiple times without drawing to the screen
//     // Because then the Display::Update(Player) func will "erase" pixels thinking the player
//     // was there, however, the pixels were never drawn to the display
//     bool hasMovedOffScreen = false;

//     public:
//     GridPos position;
//     GridPos oldPos;
//     int width = 10;
//     int height = 10;
//     int health = 100;
//     int runEnergy = 100;
//     int playerID = 333; //using this as their color
    
//     Map* map;

//     Player(int _x, int _y, Map* _map) : position(_x, _y), map(_map) {
//         map->Add(*this);
//     }

//     void MoveHoriz(int xD) {
//         hasMovedOffScreen = true;
//         oldPos.x = position.x;
//         map->Clear(*this);
//         position.x += xD;
//         if (position.x > MAP_WIDTH) {
//             position.x = position.x % MAP_WIDTH;
//         }
//         map->Add(*this);
//     }

//     void MoveVert(int yD) {
//         hasMovedOffScreen = true;
//         oldPos.y = position.y;
//         map->Clear(*this);
//         position.y += yD;
//         if (position.y > MAP_HEIGHT) {
//             position.y = position.y % MAP_HEIGHT;
//         }
//         map->Add(*this);
//     }

//     friend struct Display;
// };

// struct Map {
//     typedef std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> arr2d;
//     arr2d grid;

//     // npcs

//     // walls

//     Map () {
//         std::array<int, MAP_HEIGHT> inner;
//         inner.fill(0);
//         grid.fill(inner);
//     }

//     // Drawing each pixel based on each entry of grid for the map
//     // will be slow compared to if we can do some SDL_FillRects, but
//     // idk how to we'd do that
//     void SetStartMap() {
//         // random stuff to start it out
//         const int stride = 5;
//         for (int i = 0; i < MAP_WIDTH; i+=stride) {
//             for (int j = 0; j < MAP_HEIGHT; j+=stride) {
//                 grid[i][j] = i*MAP_HEIGHT + j;
//             }
//         }
//     }

//     // Clears the map at area covered by player
//     void Clear(Player player) {
//         for (int i = player.position.x; i < player.width; i++) {
//             for (int j = player.position.y; j < player.height; j++) {
//                 grid[i % MAP_WIDTH][j % MAP_HEIGHT] = 0;
//             }
//         }
//     }

//     // Adds a player to the map
//     void Add(Player player) {
//         for (int i = player.position.x; i < player.width; i++) {
//             for (int j = player.position.y; j < player.height; j++) {
//                 grid[i % MAP_WIDTH][j % MAP_HEIGHT] = player.playerID;
//             }
//         }
//     }
// };

// struct Display {
//     SDL_Window* window = nullptr;
//     SDL_Surface* surface = nullptr;

//     Display(SDL_Window* _w) : window(_w) {
//         surface = SDL_GetWindowSurface(window);
//     }

//     ~Display() {
//         SDL_DestroyWindow(window);
//     }

//     void Update(Map map, bool updateScreen = true) {
//         for (int i = 0; i < MAP_WIDTH; i++) {
//             for (int j = 0; j < MAP_HEIGHT; j++) {
//                 SDL_Rect point {i, j, 1, 1};
//                 SDL_FillRect(surface, &point, map.grid[i][j]);
//             }
//         }
//         if (updateScreen) {
//             SDL_UpdateWindowSurface(window);
//         }
//     }

//     void Erase(Player player, bool updateScreen = true) {
//         SDL_Rect blankRect = {player.position.x, player.position.y, player.width, player.height};
//         SDL_FillRect(surface, &blankRect, 0);
//         if (updateScreen) {
//             SDL_UpdateWindowSurface(window);
//         }
//     }

//     void Update(Player player, bool updateScreen = true) {
//         if (player.hasMovedOffScreen) {
//             Erase(player, false /* false so we don't update in Erase and update again here*/);
//         }
//         SDL_Rect playerRect = SDL_Rect {player.position.x, player.position.y, player.width, player.height};
//         SDL_FillRect(surface, &playerRect, player.playerID);
//         player.hasMovedOffScreen = false;  // we just drew it, so it hasn't moved from what's on the screen for now
//         if (updateScreen) {
//             SDL_UpdateWindowSurface(window);
//         }
//     }
// };

void init() {
    SDL_Init(SDL_INIT_EVERYTHING);
}

void cleanup() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    init();

    Map map();
    map.SetStartMap();

    // not sure how I feel about the map updating through the player class but fuck it right
    Player player1(MAP_WIDTH/2, MAP_HEIGHT/2, &map);

    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN );
    if ( !win ) {
        printf("Failed to create a window! Error: %s\n", SDL_GetError());
    }

    Display display(win);

    display.Update(map); // first draw of the map the screen (should include player initial pos)
    Sleep(1500);

    player1.MoveHoriz(20);
    display.Update(player1);
    Sleep(1500);

    player1.MoveVert(40);
    display.Update(player1);

    cleanup();
    return 0;
}


// stuff I made, and stopped using, but don't wanna throw away yet so I can reference it
namespace Junkyard {
    void DrawPlayer(Player player, SDL_Surface* winSurface, ) {
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