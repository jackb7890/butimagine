#include <cstdio>
#include <array>
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_image.h"
#include "time.h"

#include "../World.hpp"

#include "SDL_net.h"
#include "networking.hpp"

using namespace std;

#pragma warning(disable : 4244)

void init() {
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    SDL_Init(SDL_INIT_EVERYTHING);
    //A- Note the global timer starts at SDL initilization
}

void cleanup() {
    SDL_Quit();
}

class ServerDriver {
    public:
    Map map;
    Server ntwk;
};

ServerDriver driver;

// assumes error handling already done
void ProcessMoveData(std::vector<Player>& clientPlayers, Packet& packet, int client) {
    int8_t x = packet.ReadAsType<int8_t>(0);
    int8_t y = packet.ReadAsType<int8_t>(1);
    clientPlayers[client].Move(x, y);
}

void ProcessData(Packet& data, int client) {
    // if (data.IsMove()) {
    //     ProcessMoveData(data, client);
    // }
}

void SendAllEntities(TCPsocket socket) {
    // Send them all the entities on the world
    for (MapEntity* entity : driver.map.allEntities) {
        Packet newPacket(Packet::Flag_t::bNewEntity);
        newPacket.EncodePoly(entity);
        driver.ntwk.SendPacket(socket, newPacket);
    }
}

int main(int argc, char* argv[]) {
    // process args
    std::vector<std::string> args;
    if (argc > 1) {
        for (int i = 1; i <= argc - 1; i++) {
            if (!argv[i]) {
                Log::emit("error reading args to main\n");
            }
            args.push_back(argv[i]);
        }
    }

    const int MAX_PLAYERS = Server::MAX_SOCKETS-1;
    std::vector<Player> clientPlayers;

    int timeoutTime = 50'000; // in ms, so 50 seconds
    if (args.size() != 0) {
        timeoutTime = stoi(args[0]);
    }
    Log::emit("timeout set to %dms\n", timeoutTime);

    init();

    driver.map.InitializeWorld();

    // Init network connection
    if (!(driver.ntwk.Setup())) {
        Log::error("Failed server setup\n");
    }
    
    bool runLoop = true;
    const int WAIT_TIME = 300;
    while (runLoop) {

        if (timeoutTime <= 0) {
            runLoop = false;
        }
        timeoutTime--;

        int clients_ready = SDLNet_CheckSockets(driver.ntwk.socket_set, WAIT_TIME);

        if (clients_ready == -1) {
            Log::error("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready == 0) {
            Log::emit("No clients ready at this time\n");
            // no clients ready, process things in server work queue
        }
        else if (clients_ready > 0) {
            Log::emit("One or more sockets ready\n");
            if(SDLNet_SocketReady(driver.ntwk.serverSoc)) {
                Log::emit("Server socket is ready\n)");
                int indx = driver.ntwk.TryAddClient();
                if (indx > 0) {
                    Player& joinedPlayer = driver.map.CreateEntity<Player>();
                    joinedPlayer.multiplayerID = indx;
                    joinedPlayer.online = true;
                    // Send starter data to player client
                    SendAllEntities(driver.ntwk.clientSockets[joinedPlayer.multiplayerID]);
                }
            }
            
            for (int socketIndx = 0; socketIndx < driver.ntwk.MAX_SOCKETS; socketIndx++) {
                if (!SDLNet_SocketReady(driver.ntwk.clientSockets[socketIndx])) {
                    continue;
                }

                Log::emit("Client socket is ready\n)");

                Packet packet = driver.ntwk.ConsumePacket(driver.ntwk.clientSockets[socketIndx]);
            }
        }
    }
    cleanup();
    return 0;
}

// stuff I made, and stopped using, but don't wanna throw away yet so I can reference it
// namespace Junkyard {
//     void DrawPlayer(Player player, SDL_Surface* winSurface) {
//         SDL_Rect rect = SDL_Rect {player.position.x, player.position.y, player.width, player.height};
//         SDL_FillRect( winSurface, &rect, SDL_MapRGB( winSurface->format, 255, 90, 120 ));
//     }

//     void InitSurface(Player player1, SDL_Surface* winSurface) {
//         const int stride = 5;
//         for (int i = 0; i < MAP_WIDTH; i+=stride) {
//             for (int j = 0; j < MAP_HEIGHT; j+=stride) {
//                 SDL_Rect rect {i, j, stride, stride};
//                 SDL_FillRect(winSurface, &rect, i*MAP_HEIGHT + j);
//             }
//         }
//         DrawPlayer(player1, winSurface);
//     }
// };