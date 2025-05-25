
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
#include "networking.hpp"

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
    std::array<int, 4> wasd;
    
    MovementCode() {
        wasd = {0, 0, 0, 0};
    }

    void set(MovementKey mvkey) {
        if (mvkey >= wasd.size()) {
            Log::emit("Unexpected mvkey value outside of array range: %d\n", mvkey);
        }
        wasd[mvkey] = 1;
    }

    void clear(MovementKey mvkey) {
        wasd[mvkey] = 0;
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

    uint8_t compress() {
        return (wasd[0] & 1) | ((wasd[1] & 1) << 1) 
            | ((wasd[2] & 1) << 2) | ((wasd[3] & 1) << 3);
    }
};

class ClientDriver {
    public:
    MovementCode currentMovementInfo;
    MovementCode lastMovementInfo;
    int playerSpeed;
    Client clientInfo;
};

ClientDriver driver;

using namespace std;

bool InputIsUserMovement(const SDL_Event& ev) {
    if (ev.type != SDL_KEYDOWN || ev.type != SDL_KEYUP) {
        return false;
    }

    switch (ev.key.keysym.sym) {
        case SDLK_w:
        case SDLK_a:
        case SDLK_s:
        case SDLK_d:
        return true;
        
        default:
        return false;
    }
    return false;
}

void ProcessUserMovement(SDL_Event ev) {
    if (ev.type == SDL_KEYDOWN) {
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
            Log::error("Err: Calling PrcessUserMovement for unexpected key press");
            break;
        }
    }
    else if (ev.type == SDL_KEYUP) {
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
            Log::error("Err: Calling PrcessUserMovement for unexpected key unpress");
            break;
        }
    }
    else {
        Log::error("Err: Calling PrcessUserMovement for unexpected input");
    }

    Packet newPacket;
    newPacket.Encode(driver.currentMovementInfo.compress(), Packet::Flag_t::bMoving);
    driver.ntwk.SendPacket(ntwk.serverSoc, newPacket);
    // redo support new packet form for this movement data
}

bool InputIsQuitGame(const SDL_Event& ev) {
    ev.type == SDL_QUIT;
}

// returns false to kill program.
// true otherwise
bool ProcessUserInput(SDL_Event ev) {
    if (InputIsUserMovement(ev)) {
        ProcessUserMovement(ev);
    }
    else if(InputIsQuitGame(ev)) {
        return false;
    }
    return true;
}

void setup_screen() {
    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN );
    if ( !win ) {
        Log::emit("Failed to create a window! Error: %s\n", SDL_GetError());
    }
    Map map;

    Display display(win, &map);
}

void ConsumeGameStartupData() {
    
}

int main() {
    init();

    driver.clientInfo = Client("localhost");
    if (!driver.clientInfo.Connect()) {
        Log::error("Failed to connect to server\n");
    }

    // After init and before game loop below, we should start ask server for the world initialization data
    const int WAIT_TIME = 300;
    bool receivedStartingData = false;
    while (!receivedStartingData) {
        int clients_ready = SDLNet_CheckSockets(driver.clientInfo.socket_set, WAIT_TIME);
        if (clients_ready == -1) {
            Log::error("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready > 0) {
            ConsumeGameStartupData();
            receivedStartingData = true;
        }
        // continue waiting on server
    }

    display.Update(lowerFront);
    display.Update(back);
    display.Update(top);
    display.Update(bottom);

    SDL_Event ev;
    bool running = true;
    while (running) {
        if (SDL_PollEvent(&ev) != 0) {
            running = ProcessUserInput(ev);

            
        }
    }

    cleanup();
}