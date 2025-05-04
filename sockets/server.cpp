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

/*
Simplest client server interaction

1. Client sends keystroke
 - one of wasd, indicating movement

2. Server is listening or keeping a queue of incoming client packets

3. Server moves the player entity associated with the client according to wasd
 - What other data structures besides the geomap grid needs updating for client movement?
 - There should be a clear point in the code when the full game state for a frame has been updated

4.5 This could lead to interactions, like collosision with a wall, or teleport to a new area.
 - How do we go about processing thee?
 4.5.1 - update server side data structures
 4.5.2 - determine what clients need to get updates from this change, and what those updates need to be
 4.5.3 - For movement, return confirmed movement update (player X moved N tiles north)
            - if this client is on any other clients screen, send those clients updates of the movement change
            - any interaction should be taken care of server side, not when client b gets client a's movement update
            - this brings up the question of client state and server state
                - we don't want to have to send everything across ports, so only send important stuff
                - for less important stuff, this might fall out of sync with the server.
                    - example: cosmetic butterfly entity has some nontrivial movement based on the state of the game
                        . we will probably compute butterfly entity's movement and how it updates it's state (position, alive, etc)
                        all on the client side, rather than send that info over. So that means client A and client B are computing the butterfly separately
                        on their local device, and if the algorithm is nontrivial, they could get different answers for their new butterfly. Now there's an inconsistency
                        in the multiplayer game state since theoritically every client should be witnessing the same state (living in the same universe).
                        When these cases arise, since we are limiting this to not-important data we will simply ignore the issue until it inevitibly causes a problem.
                        We delay until then, saving latency ( we could also do something like verification in the server's free time between client requests to iron out these in lulls)
                        When we absolutely have to face the contradiciton between the two client's game states, we can just choose whatever the butterfly ended up as on the server's game state.

- I don't know how else to do this game but to have the world as a state and moving from one state to another is the smallest type of change that can happen
    - so that means I need to take extra care to make sure all the versions of the game running between clients+server are synchronized. aka everyone is on game state N at the same time, and
    - has not committed any changes past that (its possible we could optimize the clients to do look ahead computations of future game states that it can already compute and trash these results if necessary)

4.6 Server finishes sending all updates to clients

4.7 Server moves to the next game tick

*/

int main() {
    // General init (base SDL_init stuff so far)
    init();
    {
    // create server socket
    NetworkHelperServer ntwk;
    if (!ntwk.Init()) {
        return -1;
    }
    
    int d_sleepsLeft = 5;
    const int WAIT_TIME = 5000; // ms
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