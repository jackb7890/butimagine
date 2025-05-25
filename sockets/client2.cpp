
#include <cstdio>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <thread>
#include <array>
#include "../util.hpp"

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_net.h"
#include "sockets.h"

#include "../World2.hpp"

//-----------------------------------------------------------------------------
#define MAX_PACKET 0xFF
 
//-----------------------------------------------------------------------------
#define FLAG_QUIT 0x0000

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@@@ @@@@@@@ @@@@@@@@@@@@@@@@
// @@@@@@@@@ @@@@@@@ @@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@\@@@@@@@@@@@/@@@@@@@@@@@@@@
// @@@@@@@@@         @@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

const char * serverIP = "";
const size_t serverPort = 8099;

void init() {
    if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
        printf("ER: SDL_Init: %sn", SDL_GetError());
        assert(false);
    }
}

void cleanup() {
    SDL_Quit();
}

struct MovementCode {
    enum MovementKey {W, A, S, D};
    std::array<bool, 4> wasd;
    
    MovementCode() {
        wasd = {false, false, false, false};
    }

    void set(MovementKey mvkey) {
        if (mvkey >= wasd.size()) {
            Log::emit("Unexpected mvkey value outside of array range: %d\n", mvkey);
        }
        wasd[mvkey] = true;
    }

    void clear(MovementKey mvkey) {
        wasd[mvkey] = false;
    }
    
    bool get(MovementKey mvkey) {
        return wasd[mvkey];
    }

    bool operator==(const MovementCode& rhs) const {
        for (int i = 0; i < 4; i++) {
            if (wasd[i] != rhs.wasd[i]) return false;
        }
        return true;
    }

    bool operator!=(const MovementCode& rhs) const {
        return !(*this == rhs);
    }
};

class ClientDriver {
    public:
    MovementCode currentMovementInfo;
    MovementCode lastMovementInfo;
    int playerSpeed;
};

ClientDriver driver;

using namespace std;

Data ProcessUserInput(SDL_Event ev) {
    Data response;
    switch (ev.type) {
        case SDL_QUIT: {
            response.dataFlags.SetQuit();
        }
        break;
        //A- Instead of directly increasing/decreasing velocity, pressing a key sets a flag to true, while releasing it sets the flag to false
        //A- We had an issue before where holding a key down didn't register until it's been held for a full second
        //A- With this method, A key is considred to always be held down until a key up input is read
        //A- Changing directions and holding multiple directions now works as intended
        case SDL_KEYDOWN:
        {
            SDL_Keycode key = ev.key.keysym.sym;
            switch (key) {
            case SDLK_w:
                driver.currentMovementInfo.set(MovementCode::MovementKey::W);
                break;
            case SDLK_a:
                driver.currentMovementInfo.set(MovementCode::MovementKey::A);
                break;
            case SDLK_s:
                driver.currentMovementInfo.set(MovementCode::MovementKey::S);
                break;
            case SDLK_d:
                driver.currentMovementInfo.set(MovementCode::MovementKey::D);
                break;
            default:
                break;
            }
        }
        break;
        case SDL_KEYUP:
        {
            SDL_Keycode key = ev.key.keysym.sym;
            switch (key) {
            case SDLK_w:
                driver.currentMovementInfo.clear(MovementCode::MovementKey::W);
                break;
            case SDLK_a:
                driver.currentMovementInfo.clear(MovementCode::MovementKey::A);
                break;
            case SDLK_s:
                driver.currentMovementInfo.clear(MovementCode::MovementKey::S);
                break;
            case SDLK_d:
                driver.currentMovementInfo.clear(MovementCode::MovementKey::D);
                break;
            default:
                break;
            }
        }
        break;
        default: {
        }
        break;
    }
    
    if (driver.currentMovementInfo != driver.lastMovementInfo) {
        response.AppendMovementData(driver.playerSpeed, driver.playerSpeed);
    }

    return response;
}

void setup_screen() {
    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN );
    if ( !win ) {
        Log::emit("Failed to create a window! Error: %s\n", SDL_GetError());
    }
    Map map;

    Display display(win, &map);
}

int main() {
    init();

    NetworkHelperClient ntwk("localhost");
    if (!ntwk.Init()) {
        printf("Failed client init\n");
        return -1;
    }

    // After init and before game loop below, we should start ask server for the world initialization data


    display.Update(lowerFront);
    display.Update(back);
    display.Update(top);
    display.Update(bottom);

    SDL_Event ev;
    while (true) {
        if (SDL_PollEvent(&ev) != 0) {
            Data response = ProcessUserInput(ev);

            if (response.dataFlags.IsQuit()) {
                cleanup();
                return 0;
            }
            else if (response.dataFlags.IsMove()) {
                ntwk.SendData(ntwk.serverSoc, response);
            }
        }
    }
}