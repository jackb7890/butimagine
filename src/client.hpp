
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
#include "sdl_helpers/events.hpp"
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
    std::array<char, 4> wasd;
    
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
    void ProcessMoveInput(SDL_Event ev);
    void ExecuteFrame();

    // networking functions
    void ProcessServerUpdate();
    void InitializeEntities();

    void Cleanup();

    private:
    MovementCode moveInfo;
    Client clientInfo;
    Map map;
    Player* me;
    Display* display;
    std::vector<MapEntity*> entitiesToDraw;
    Log logger;
};