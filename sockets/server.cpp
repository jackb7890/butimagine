#include <cstdio>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <thread>
// tutorial maybe https://stephenmeier.net/2015/12/23/sdl-2-0-tutorial-04-networking/

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_net.h"
#include "sockets.h"

void init() {
    if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
        printf("ER: SDL_Init: %sn", SDL_GetError());
        assert(false);
    }
}

void cleanup() {
    SDL_Quit();
}

#include <vector>
int main(int argc, char * argv[]) {
    std::vector<std::string> args;
    if (argc > 2) {
        for (int i = 0; i < argc - 2; i++) {
            if (!argv[i]) {
                Log::emit("error reading args to main\n");
            }
            args.push_back(argv[i]);
        }
    }


    int timeoutTime = 50'000; // in ms, so 50 seconds
    if (!args.empty()) {
        timeoutTime = stoi(args[0]);
    }
    Log::emit("timeout set to %dms\n", timeoutTime);

    // General init (base SDL_init stuff so far)
    init();
    {
    // create server socket
    NetworkHelperServer ntwk;
    if (!ntwk.Init()) {
        return -1;
    }
    
    int d_sleepsLeft = 100;
    const int WAIT_TIME = 1000; // ms
    bool runLoop = true;

    while (runLoop && d_sleepsLeft > 0) {
        d_sleepsLeft--;

        int clients_ready = SDLNet_CheckSockets(ntwk.socket_set, WAIT_TIME);

        if (clients_ready == -1) {
            printf("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready == 0) {
            printf("No clients ready at this time\n");
            // no clients ready, process things in server work queue
        }
        else if (clients_ready > 0) {
            // clients are ready
            printf("Client(s) are ready\n");
            if(SDLNet_SocketReady(ntwk.serverSoc)) {
                // client connection ocurred
                printf("Server socket is ready\n)");
                ntwk.TryAddClient();
            }

            for (int socketIndx = 0; socketIndx < ntwk.MAX_SOCKETS; socketIndx++) {
                if (!SDLNet_SocketReady(ntwk.clientSockets[socketIndx])) {
                    continue;
                }

                printf("Client socket is ready\n)");

                Data data = ntwk.RecvData(ntwk.clientSockets[socketIndx]);
                if(!data.isValid) {
                    continue;
                }
            
                // switch(data.dataFlags.bits) {
                    // case FLAG_WOOD_UPDATE: {
                    //     Data dataToSend; // TODO: fill this in with real data
                    //     ntwk.SendData(socketIndx, dataToSend);
                    // } break;
            
                    // case FLAG_QUIT: {
                    //     runLoop = false;
                    //     printf("DB: shutdown by client id: %dn", socketIndx);
                    // } break;
                //}        
            }
        }
    }
    }
    cleanup();
    return 0;
}


        // listen on the port for a client, loop until you find one.
        // I guess there is some way in the socket functions to tell its the client from our game
        // maybe we can hardcode a passcode in the client, and pass that on its port to this server socket

        // once you have a connection,
        // wait for it to send you data
        // probably just keyboard input for now

        // once you have the keyboard input,
        // we can use the event processing code thats in the eventful-walls branch
        // + Map, Player, and Wall code in the main branch to move a player on the map, and then return the map back to the client

        // then the client can display the map, and send another keyboard input to the server and repeat

        // server.cpp will have the map, player, and wall objects to keep track of the state of the game
        // it will also have the logic to parse the keyboard input from the client
        // then it will send back a the new state of the game for the player to display

        // ORRRRRRR

        // i just got different idea
        // we could have the map, player, walls, etc on the client side and the server side
        // so when we do like WASD on the client to move around, we could
            // 1) process the keyboard input on the client side
            // 2) update the map with the new player location
            // 3) send the keyboard input to the server as well
            // 4) and the server can use the keyboard input to make sure its version of the map
            //      is updated in the same way
            // 5) The server will only have to send this data to other clients
            //      so that they are aware that another client has moved
            // 
            // I like this because the size of the data being transferred is a lot smaller
            // in the first one, its the entire world map, every keystroke
            // in this one, its just the single keystroke