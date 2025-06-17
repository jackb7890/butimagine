
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

    bool IsMovingUp() {
        return wasd[0] && !wasd[2];
    }

    bool IsMovingDown() {
        return wasd[2] && !wasd[0];
    }

    bool IsMovingRight() {
        return wasd[3] && !wasd[1];
    }

    bool IsMovingLeft() {
        return wasd[1] && !wasd[3];
    }

    bool IsMoving() {
        return IsMovingUp() || IsMovingDown() || IsMovingLeft() || IsMovingRight();
    }
};

class ClientDriver {
    public:
    MovementCode currentMovementInfo;
    MovementCode lastMovementInfo;
    Client clientInfo;
    Map map;
    Display display;
    bool updateDisplay;
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
            Log::error("Err: Calling PrcessUserMovement for unexpected key unpress");
            break;
        }
    }
    else {
        Log::error("Err: Calling PrcessUserMovement for unexpected input");
    }

    if (!driver.currentMovementInfo.IsMoving()) {
        return;
    }

    driver.updateDisplay = true;

    // first send this information to the server
    ImMoving moving;
    if (driver.currentMovementInfo.IsMovingUp()) {
        moving.yOff = 1;
    }
    else if (driver.currentMovementInfo.IsMovingDown()) {
        moving.yOff = -1;
    }

    if (driver.currentMovementInfo.IsMovingRight()) {
        moving.xOff = 1;
    }
    else if (driver.currentMovementInfo.IsMovingLeft()) {
        moving.xOff = -1;
    }

    Packet newPacket(Packet::Flag_t::bMoving);
    newPacket.Encode(moving);
    driver.clientInfo.SendPacket(driver.clientInfo.serverSoc, newPacket);

    // next update our own map
    
}

bool InputIsQuitGame(const SDL_Event& ev) {
    return ev.type == SDL_QUIT;
}

// returns false to kill program.
// true otherwise
bool ProcessUserInput(SDL_Event ev) {
    if (InputIsUserMovement(ev)) {
        ProcessUserMovement(ev);
    }
    
    if (InputIsQuitGame(ev)) {
        return false;
    }
    return true;
}

void ProcessServerUpdate() {
    const int WAIT_TIME = 300;
    int clients_ready = SDLNet_CheckSockets(driver.clientInfo.socket_set, WAIT_TIME);
    if (clients_ready == -1) {
        Log::error("Error returned by SDLNet_CheckSockets\n");
    }
    else if (clients_ready == 0) {
        return;
    }
    else {
        if(!SDLNet_SocketReady(driver.clientInfo.serverSoc)) {
            Log::emit("SDLnET\n)");
            return;
        }
        // We have something from the server
        Packet p = driver.clientInfo.ConsumePacket(driver.clientInfo.serverSoc);
        
        if (p.flags.test(Packet::Flag_t::bMoving)) {
            // something has moved in the world
            EntityMoveUpdate data = p.ReadAsType<EntityMoveUpdate>();
            MapEntity* entityToMove = driver.map.GetEntity(data.id);
            entityToMove->Move(data.xOff, data.yOff);
            
            driver.updateDisplay = true;
        }
        
    }
}

void GetAllEntities() {
    driver.updateDisplay = true;
    // After init and before game loop below, we should start ask server for the world initialization data
    const int WAIT_TIME = 300;
    bool receivedStartingData = false;
    while (!receivedStartingData) {
        int clients_ready = SDLNet_CheckSockets(driver.clientInfo.socket_set, WAIT_TIME);
        if (clients_ready == -1) {
            Log::error("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready > 0) {
        
            if(!SDLNet_SocketReady(driver.clientInfo.serverSoc)) {
                Log::emit("SDLnET\n)");
                continue;
            }

            Packet p = driver.clientInfo.ConsumePacket(driver.clientInfo.serverSoc);
            
            if (p.flags.test(Packet::Flag_t::bEndOfPacketGroup)) {
                receivedStartingData = true;
                break;
            }

            else if (p.flags.test(Packet::Flag_t::bNewEntity)) {
                // we need to build the entity type again
                
                // get the type id from data
                if (p.polyTypeID == TypeDetails<Player>::index) {
                    Player transmitted = p.ReadAsType<Player>();
                    driver.map.SpawnEntity<>(transmitted);
                }
                else if (p.polyTypeID == TypeDetails<Wall>::index) {
                    Wall transmitted = p.ReadAsType<Wall>();
                    driver.map.SpawnEntity<>(transmitted);
                }
                else if (p.polyTypeID == TypeDetails<MapEntity>::index) {
                    MapEntity transmitted = p.ReadAsType<Player>();
                    driver.map.SpawnEntity<>(transmitted);
                }
                else {
                    Log::error("received NewEntity packet but it wasn't any of the expeted types");
                }
                // RegisterNewEntity allocates memory and adds it to the list of entities;
            }
            
        }
    }
}

void setup_screen() {
    SDL_Window* win = SDL_CreateWindow( "my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN );
    if ( !win ) {
        Log::error("Failed to create a window! Error: %s\n", SDL_GetError());
    }
    driver.ntwk.display = Display(win, &driver.ntwk.map);
}

void update_screen() {
    if (driver.updateDisplay) {
        display.Update();
    }
    driver.updateDisplay = false;
}

int main() {
    init();

    driver.clientInfo = Client("localhost");
    if (!driver.clientInfo.Connect()) {
        Log::error("Failed to connect to server\n");
    }

    // build up the entities sent over from the server to start with
    GetAllEntities();

    const int WAIT_TIME = 300;
    SDL_Event ev;
    bool running = true;

    while (running) {
        // first check for any input from the client device
        if (SDL_PollEvent(&ev) != 0) {
            running = ProcessUserInput(ev);      
        }
        
        // next check if the server has anything for us
        int clients_ready = SDLNet_CheckSockets(driver.clientInfo.socket_set, WAIT_TIME);

        if (clients_ready == -1) {
            Log::error("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready > 0) {
            ProcessServerUpdate();
        }

        // finally, update the screen
        update_screen();
    }

    cleanup();
}