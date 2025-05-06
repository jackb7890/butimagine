
#include <cstdio>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <thread>

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_net.h"
#include "sockets.h"

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

using namespace std;

int main() {
    init();

    NetworkHelperClient ntwk("localhost");
    if (!ntwk.Init()) {
        printf("Failed client init\n");
        return -1;
    }

    int d_sleepsLeft = 10;
    while (true && d_sleepsLeft > 0) {
        d_sleepsLeft--;
        Data d = Data::CreateTestData();
        ntwk.SendData(ntwk.serverSoc, d);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}