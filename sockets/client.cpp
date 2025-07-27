
#include <cstdio>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <thread>
#include <array>
#include "../util.hpp"

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_net.h"
#include "networking.hpp"

#include "../World.hpp"

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

struct MovementCode {
    enum MovementKey {W, A, S, D};
    std::array<int, 4> wasd;
    
    MovementCode() {
        wasd = {0, 0, 0, 0};
    }

    int asBits() {
        return wasd[D] | (wasd[S] << 1) | (wasd[A] << 2) | (wasd[W] << 3);
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

struct ClientDriver {
    public:
    MovementCode currentMovementInfo;
    bool justMoved;
    MovementCode lastMovementInfo;
    Client clientInfo;
    Map map;
    Player* me;
    Display* display;
    std::vector<MapEntity*> entitiesToDraw;

    ClientDriver() : justMoved(false) {};
};

ClientDriver driver;

void init() {

    // initialize user screen
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    // initialize SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Log::error("ER: SDL_Init: %sn", SDL_GetError());
    }

    driver.display = new Display(&driver.map);
    //driver.display = new Display(window, &driver.map);

    //A- Window should be the same as it was before the rendering swap
    SDL_Window* window = SDL_CreateWindow("my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        Log::error("Failed to create window! Error: %s\n", SDL_GetError());
    }

    driver.display->window = window;

    // SDL_UpdateWindowSurface(window);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
       Log::error("Failed to create renderer! Error: %s\n", SDL_GetError());
    }

    driver.display->renderer = renderer;
    // finished initializing user screen
}

void cleanup() {

    delete driver.display;

    // cleanup user screen
    SDL_DestroyRenderer(driver.display->renderer);
    //SDL_DestroyWindow(driver.display->window);
    SDL_Quit();

    // cleanup SDL subsystems
    SDL_Quit();
}

using namespace std;

bool InputIsUserMovement(const SDL_Event& ev) {
    if (ev.type != SDL_KEYDOWN && ev.type != SDL_KEYUP) {
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

std::pair<int, bool> EventGetMovementInfo(SDL_Event ev) {
    return std::pair(ev.key.keysym.sym, ev.type == SDL_KEYUP);
}

bool InputIsQuitGame(const SDL_Event& ev) {
    return ev.type == SDL_QUIT;
}

// returns false to kill program.
// true otherwise
bool ProcessEvent(SDL_Event ev) {

    Log logger("ProcessEvent: ");
    logger.Emit("begin\n");
    logger.Emit("event type = %d", ev.type);

    if (InputIsQuitGame(ev)) {
        logger.Emit("quit game (return 'true')\n");
        return true;
    }

    if (InputIsUserMovement(ev)) {

        driver.justMoved = true;

        auto [sdlKey, isKeyRelease] = EventGetMovementInfo(ev);
        if (!isKeyRelease) {
            logger.Emit("event is a keyPress\n");
            logger.Emit("old movement status %X\n", driver.currentMovementInfo.asBits());
            switch (sdlKey) {
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
                logger.Error("Error: key press not one of w,a,s,d. sdlKey = %d\n", sdlKey);
                break;
            }
        }
        else {
            logger.Emit("event is a keyRelease\n");
            logger.Emit("old movement status %X\n", driver.currentMovementInfo.asBits());
            switch (sdlKey) {
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
                logger.Error("Error: key release not one of w,a,s,d. sdlKey = %d\n", sdlKey);
                break;
            }
        }

        logger.Emit("new movement status %X", driver.currentMovementInfo.asBits());
    }

    return false;
}

void RunAllClientJobs() {
    Log logger("RunAllClientJobs: ");
    logger.Emit("begin\n");
    if (driver.justMoved) {
        driver.justMoved = false;

        logger.Emit("driver moved this iteration\n");

        ImMoving moving {0, 0};
        if (driver.currentMovementInfo.IsMovingUp()) {
            logger.Emit("moving up\n");
            moving.yOff = 1;
        }
        else if (driver.currentMovementInfo.IsMovingDown()) {
            logger.Emit("moving down\n");
            moving.yOff = -1;
        }

        if (driver.currentMovementInfo.IsMovingRight()) {
            logger.Emit("moving right\n");
            moving.xOff = 1;
        }
        else if (driver.currentMovementInfo.IsMovingLeft()) {
            logger.Emit("moving left\n");
            moving.xOff = -1;
        }

        // assert we are indeed moving
        if (!moving.xOff && !moving.yOff) {
            logger.Error("Error: All IsMoving* calls returned 'false'\n");
        }

        Packet newPacket(Packet::Flag_t::bMoving);
        newPacket.Encode(moving);
        driver.clientInfo.SendPacket(driver.clientInfo.serverSoc, newPacket);

        // next update our own map
        driver.me->Move(moving.xOff, moving.yOff);
    }
}

void ProcessServerUpdate() {
    const int WAIT_TIME = 300;

    Log::emit("Running ProcessServerUpdate...\n");

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
        std::vector<Packet> consumedPackets;
        if(!driver.clientInfo.ConsumePackets(driver.clientInfo.serverSoc, consumedPackets)) {
            Log::emit("ConsumePackets failed during ProcessServerUpdate\n");
            return;
        }
        
        for (auto p : consumedPackets) {
            if (p.flags.test(Packet::Flag_t::bMoving)) {
                // something has moved in the world
                EntityMoveUpdate data = p.ReadAsType<EntityMoveUpdate>();
                MapEntity* entityToMove = driver.map.GetEntity(data.id);
                entityToMove->Move(data.xOff, data.yOff);
                driver.map.drawMeBuf.push_back(entityToMove);
            }
        }
    }
}
// ask server for the world initialization data
void GetAllEntities() {
    Log::emit("Start of GetAllEntities (initial server transmission)\n");
    const int WAIT_TIME = 300;
    bool recvComplete = false;
    while (!recvComplete) {
        int clients_ready = SDLNet_CheckSockets(driver.clientInfo.socket_set, WAIT_TIME);
        Log::emit("GetAllEntites: clients_ready=%d\n", clients_ready);
        if (clients_ready < 0) {
            Log::error("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready == 0) {
            Log::emit("GetAllEntities waiting for socket...\n");
        }
        else {
            if(!SDLNet_SocketReady(driver.clientInfo.serverSoc)) {
                Log::error("Error: CheckSockets shows > 0 ready, but SocketReady is false\n)");
                continue;
            }

            std::vector<Packet> consumedPackets;
            driver.clientInfo.ConsumePackets(driver.clientInfo.serverSoc, consumedPackets);
            
            for (auto p : consumedPackets) {
                if (p.flags.test(Packet::Flag_t::bEndOfPacketGroup)) {
                    Log::emit("GetAllEntities: recevied end-of-list packet\n");
                    recvComplete = true;
                    break;
                }
                else if (p.flags.test(Packet::Flag_t::bNewEntity)) {
                    // we need to build the entity type again
                    
                    Log::emit("GetAllEntities: received new-entity packet\n");

                    // get the type id from data
                    if (p.polyTypeID == TypeDetails<Player>::index) {
                        Log::emit("new-entity packet is a Player\n");
                        Player transmitted = p.ReadAsType<Player>();
                        Player* player = driver.map.SpawnEntity<>(transmitted);
                        driver.map.drawMeBuf.push_back(player);
                    }
                    else if (p.polyTypeID == TypeDetails<Wall>::index) {
                        Log::emit("new-entity packet is a Wall\n");
                        Wall transmitted = p.ReadAsType<Wall>();
                        Wall* wall = driver.map.SpawnEntity<>(transmitted);
                        driver.map.drawMeBuf.push_back(wall);
                    }
                    else if (p.polyTypeID == TypeDetails<MapEntity>::index) {
                        Log::emit("new-entity packet is a MapEntity\n");
                        MapEntity transmitted = p.ReadAsType<Player>();
                        driver.map.SpawnEntity<>(transmitted);
                        MapEntity* entity = driver.map.SpawnEntity<>(transmitted);
                        driver.map.drawMeBuf.push_back(entity);
                    }
                    else {
                        Log::error("GetAllEntites: Error unexpected new-entity polyTypeID=%d\n", p.polyTypeID);
                    }
                    // RegisterNewEntity allocates memory and adds it to the list of entities;
                }
            }
        }
    }

    // last entity sent should be ourselves
    // ^this appears to still be true
    driver.me = dynamic_cast<Player*>(driver.map.allEntities.back());
    
    Log::emit("Finished GetAllEntites - success!\n");
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
            running = !ProcessEvent(ev);
            RunAllClientJobs();
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
        if (!driver.map.drawMeBuf.empty()) {
            SDL_RenderClear(driver.display->renderer);
            driver.display->DrawFrame(driver.map.drawMeBuf);
            driver.map.drawMeBuf.clear();
        }
        
    }

    cleanup();
}