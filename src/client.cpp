
#include <cstdio>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <thread>
#include <array>
#include "util.hpp"

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_net.h"
#include "sockets/networking.hpp"

#include "World.hpp"

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
            //Log::emit("Unexpected mvkey value outside of array range: %d\n", mvkey);
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
    ClientDriver() {}
    void Initialize();
    bool ProcessEvent(SDL_Event);
    void ExecuteFrame();

    // networking functions
    void ProcessServerUpdate();
    void InitializeEntities();

    void Cleanup();

    private:
    MovementCode currentMovementInfo;
    bool justMoved;
    MovementCode lastMovementInfo;
    Client clientInfo;
    Map map;
    Player* me;
    Display* display;
    std::vector<MapEntity*> entitiesToDraw;
    Log logger;
};

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

// returns false if we are to exit the user from the game.
bool ClientDriver::ProcessEvent(SDL_Event ev) {

    logger.StartPhase("ProcessEvent");

    const char* description = MapEventIdToName(ev.type);

    logger.Emit("event type = %s\n", description);

    if (InputIsQuitGame(ev)) {
        logger.Emit("quit game (returning 'true')\n");
        return true;
    }

    if (InputIsUserMovement(ev)) {

        auto [sdlKey, isKeyRelease] = EventGetMovementInfo(ev);
        
        if (!isKeyRelease) {   
            logger.Emit("event is a keyPress\n");
            logger.Emit("old movement status %X\n", currentMovementInfo.asBits());

            justMoved = true;

            switch (sdlKey) {
            case SDLK_w:
                currentMovementInfo.set(MovementCode::MovementKey::W);
                break;
            case SDLK_a:
                currentMovementInfo.set(MovementCode::MovementKey::A);
                break;
            case SDLK_s:
                currentMovementInfo.set(MovementCode::MovementKey::S);
                break;
            case SDLK_d:
                currentMovementInfo.set(MovementCode::MovementKey::D);
                break;
            default:
                logger.Error("Error: key press not one of w,a,s,d. sdlKey = %d\n", sdlKey);
                break;
            }
        }
        else {
            logger.Emit("event is a keyRelease\n");
            logger.Emit("old movement status %X\n", currentMovementInfo.asBits());
            
            switch (sdlKey) {
            case SDLK_w:
                currentMovementInfo.clear(MovementCode::MovementKey::W);
                break;
            case SDLK_a:
                currentMovementInfo.clear(MovementCode::MovementKey::A);
                break;
            case SDLK_s:
                currentMovementInfo.clear(MovementCode::MovementKey::S);
                break;
            case SDLK_d:
                currentMovementInfo.clear(MovementCode::MovementKey::D);
                break;
            default:
                logger.Error("Error: key release not one of w,a,s,d. sdlKey = %d\n", sdlKey);
                break;
            }
        }

        logger.Emit("new movement status %X\n", currentMovementInfo.asBits());
    }

    return false;
}

void ClientDriver::ExecuteFrame() {
    logger.StartPhase("ExecuteFrame");
    logger.Emit("begin\n");
    if (justMoved) {
        justMoved = false;

        logger.Emit("user moved this frame\n");

        ImMoving moving {0, 0};
        if (currentMovementInfo.IsMovingUp()) {
            logger.Emit("moving up\n");
            moving.yOff = -10;
        }
        else if (currentMovementInfo.IsMovingDown()) {
            logger.Emit("moving down\n");
            moving.yOff = 10;
        }

        if (currentMovementInfo.IsMovingRight()) {
            logger.Emit("moving right\n");
            moving.xOff = 10;
        }
        else if (currentMovementInfo.IsMovingLeft()) {
            logger.Emit("moving left\n");
            moving.xOff = -10;
        }

        Packet newPacket(Packet::Flag_t::bMoving);
        newPacket.Encode(moving);
        clientInfo.SendPacket(clientInfo.serverSoc, newPacket);

        // next update our own map
        me->Move(moving.xOff, moving.yOff);

        // finally, update the screen
        map.drawMeBuf.push_back(me);
        if (!map.drawMeBuf.empty()) {
            display->DrawFrame(map.drawMeBuf);
            map.drawMeBuf.clear();
        }
    }
    logger.EndPhase();
}

void ClientDriver::ProcessServerUpdate() {
    const int WAIT_TIME = 0;

    logger.StartPhase("ProcessServerUpdate");

    int clients_ready = SDLNet_CheckSockets(clientInfo.socket_set, WAIT_TIME);
    if (clients_ready == -1) {
        Log::error("Error returned by SDLNet_CheckSockets\n");
    }
    else if (clients_ready == 0) {
        return;
    }
    else {
        if(!SDLNet_SocketReady(clientInfo.serverSoc)) {
            logger.Emit("SDLnET\n)");
            return;
        }
        // We have something from the server
        std::vector<Packet> consumedPackets;
        if(!clientInfo.ConsumePackets(clientInfo.serverSoc, consumedPackets)) {
            logger.Emit("ConsumePackets failed during ProcessServerUpdate\n");
            return;
        }
        
        for (auto p : consumedPackets) {
            if (p.flags.test(Packet::Flag_t::bMoving)) {
                // something has moved in the world
                EntityMoveUpdate data = p.ReadAsType<EntityMoveUpdate>();
                MapEntity* entityToMove = map.GetEntity(data.id);
                entityToMove->Move(data.xOff, data.yOff);
                map.drawMeBuf.push_back(entityToMove);
            }
        }
    }
    logger.EndPhase();
}

// TODO: Integrate this into ClientDriver2
// ask server for the world initialization data
void ClientDriver::InitializeEntities() {
    logger.StartPhase("InitializeEntities");
    const int WAIT_TIME = 0;
    bool recvComplete = false;
    while (!recvComplete) {
        int clients_ready = SDLNet_CheckSockets(clientInfo.socket_set, WAIT_TIME);
        logger.Emit("\tclients_ready=%d\n", clients_ready);
        if (clients_ready < 0) {
            Log::error("Error: SDLNet_CheckSockets failed\n");
        }
        else if (clients_ready == 0) {
            logger.Emit("\twaiting for socket...\n");
        }
        else {
            if(!SDLNet_SocketReady(clientInfo.serverSoc)) {
                Log::error("Error: CheckSockets returned %d, but the SocketReady(server) was false\n)");
                continue;
            }

            std::vector<Packet> consumedPackets;
            clientInfo.ConsumePackets(clientInfo.serverSoc, consumedPackets);
            
            for (auto p : consumedPackets) {
                if (p.flags.test(Packet::Flag_t::bEndOfPacketGroup)) {
                    logger.Emit("\trecevied end-of-list packet\n");
                    recvComplete = true;
                    break;
                }
                else if (p.flags.test(Packet::Flag_t::bNewEntity)) {
                    // we need to build the entity type again
                    
                    logger.Emit("\treceived new-entity packet\n");

                    // get the type id from data
                    if (p.polyTypeID == TypeDetails<Player>::index) {
                        logger.Emit("\tnew-entity packet is a Player\n");
                        Player transmitted = p.ReadAsType<Player>();
                        Player* player = map.SpawnEntity<>(transmitted);
                        map.drawMeBuf.push_back(player);
                    }
                    else if (p.polyTypeID == TypeDetails<Wall>::index) {
                        logger.Emit("\tnew-entity packet is a Wall\n");
                        Wall transmitted = p.ReadAsType<Wall>();
                        Wall* wall = map.SpawnEntity<>(transmitted);
                        map.drawMeBuf.push_back(wall);
                    }
                    else if (p.polyTypeID == TypeDetails<MapEntity>::index) {
                        logger.Emit("\tnew-entity packet is a MapEntity\n");
                        MapEntity transmitted = p.ReadAsType<Player>();
                        map.SpawnEntity<>(transmitted);
                        MapEntity* entity = map.SpawnEntity<>(transmitted);
                        map.drawMeBuf.push_back(entity);
                    }
                    else {
                        Log::error("Error: unexpected new-entity polyTypeID=%d\n", p.polyTypeID);
                    }
                    // RegisterNewEntity allocates memory and adds it to the list of entities;
                }
            }
        }
    }

    // last entity sent should be ourselves
    // ^this appears to still be true
    me = dynamic_cast<Player*>(map.allEntities.back());
    
    logger.EndPhase();
}

void cleanup(void);

int main() {

    ClientDriver driver = ClientDriver();

    driver.Initialize();

    // build up the entities sent over from the server to start with
    driver.InitializeEntities();

    const int WAIT_TIME = 0;
    SDL_Event ev;
    bool running = true;

    while (running) {
        // next check if the server has anything for us
        driver.ProcessServerUpdate();

        // first check for any input from the client device
        if (SDL_PollEvent(&ev) != 0) {
            running = !driver.ProcessEvent(ev);
        }

        driver.ExecuteFrame();
    }

    driver.Cleanup();
}

// TODO: should i group the SDL stuff?
void ClientDriver::Initialize() {

    // initialize user screen
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    // initialize SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Log::error("ER: SDL_Init: %sn", SDL_GetError());
    }

    SDL_Window* window = SDL_CreateWindow("my window", 100, 100, MAP_WIDTH, MAP_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        Log::error("Failed to create window! Error: %s\n", SDL_GetError());
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
       Log::error("Failed to create renderer! Error: %s\n", SDL_GetError());
    }

    this->map = Map();

    this->display = new Display(&(this->map));

    this->display->window = window;

    // SDL_UpdateWindowSurface(window);

    this->display->renderer = renderer;

    clientInfo = Client("localhost");
    if (!clientInfo.Connect()) {
       Log::error("Failed to connect to server\n");
    }
}

// TODO: should this be a destructor?
void ClientDriver::Cleanup() {

    delete display;

    // cleanup user screen
    SDL_DestroyRenderer(display->renderer);
    //SDL_DestroyWindow(display->window);
    SDL_Quit();

    // cleanup SDL subsystems
    SDL_Quit();
}