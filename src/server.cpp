#include <cstdio>
#include <array>
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>

#define SDL_MAIN_HANDLED 1

#include "SDL.h"
#include "SDL_image.h"
#include "time.h"

#include "World.hpp"

#include "SDL_net.h"
#include "sockets/networking.hpp"

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
    std::array<Player*, Server::MAX_SOCKETS> clientEntities; // move this to map.players
    Log logger;

    std::vector<std::string> ProcessArgs(int argc, char* argv[]);
    void SendAllEntities(TCPsocket socket);
    void SendPacketToAllButOne(Packet p, int socketIndx);
    void ProcessMoveData(std::vector<Player>& clientPlayers, Packet& packet, int client);
};

// assumes error handling already done
void ServerDriver::ProcessMoveData(std::vector<Player>& clientPlayers, Packet& packet, int client) {
    logger.StartPhase("ProcessPlayerMove");

    int8_t x = packet.ReadAsType<int8_t>(0);
    int8_t y = packet.ReadAsType<int8_t>(1);
    
    logger.Emit("player: %d -> dx: %d, dy: %d\n", client, x, y);

    clientPlayers[client].Move(x, y);
}

void ProcessData(Packet& data, int client) {
    // if (data.IsMove()) {
    //     ProcessMoveData(data, client);
    // }
}

void ServerDriver::SendAllEntities(TCPsocket socket) {
    // Send them all the entities on the world
    logger.StartPhase("SendAllEntities");
    for (MapEntity* entity : map.allEntities) {
        Packet newPacket(Packet::Flag_t::bNewEntity);
        newPacket.EncodePoly(*entity, entity->GetTypeIndex());
        logger.Emit("Sending one entity... total encoded size %d bytes\n", newPacket.EncodeSize());
        ntwk.SendPacket(socket, newPacket);
    }

    Packet newPacket(Packet::Flag_t::bEndOfPacketGroup);

    logger.Emit("Sending end-of-list packet... total encoded size %d bytes\n", newPacket.EncodeSize());
    ntwk.SendPacket(socket, newPacket);

    logger.EndPhase();
}

void ServerDriver::SendPacketToAllButOne(Packet p, int socketIndx) {
    for (int i = 0; i < ntwk.MAX_SOCKETS; i++) {
        TCPsocket clientSocket = ntwk.clientSockets[i];
        if (!clientSocket) {
            continue;
        }
        if (i == socketIndx) {
            continue;
        }
        ntwk.SendPacket(clientSocket, p);
    }
}

std::vector<std::string> ServerDriver::ProcessArgs(int argc, char* argv[]) {
    std::vector<std::string> args;
    if (argc > 1) {
        for (int i = 1; i <= argc - 1; i++) {
            if (!argv[i]) {
                logger.Emit("error reading args to main\n");
            }
            args.push_back(argv[i]);
        }
    }
    return args;
}

int main(int argc, char* argv[]) {
    ServerDriver driver;
    // process args
    std::vector<std::string> args = driver.ProcessArgs(argc, argv);

    std::time_t startTime = std::time(nullptr);
    if (startTime == -1) {
        // error
    }

    int timeoutTime = 15; // in seconds
    if (args.size() != 0) {
        timeoutTime = stoi(args[0]);
    }
    driver.logger.EmitVerbose("timeout set to %dms\n", timeoutTime);

    const int MAX_PLAYERS = Server::MAX_SOCKETS-1;
    std::vector<Player> clientPlayers;

    init();

    // Init network connection
    if (!(driver.ntwk.Setup())) {
        Log::error("Failed server setup\n");
    }
    
    bool runLoop = true;
    const int WAIT_TIME = 0;
    while (runLoop) {
        std::time_t frameTime = std::time(nullptr);
        if (frameTime < startTime) {
            // overflow, come back to this, for now, just switch them
            Log::error("TODO handle overflow time (error for now)\n");
        }
        else if ((frameTime - startTime) >= timeoutTime) {
            runLoop = false;
        }

        int clients_ready = SDLNet_CheckSockets(driver.ntwk.socket_set, WAIT_TIME);

        if (clients_ready == -1) {
            Log::error("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready == 0) {
            // driver.logger.Emit("No clients ready at this time\n");
            // no clients ready, process things in server work queue
        }
        else if (clients_ready > 0) {
            // driver.logger.Emit("One or more sockets ready\n");
            int newPlayerInd = -1;
            if(SDLNet_SocketReady(driver.ntwk.serverSoc)) {
                driver.logger.Emit("new player has connected!\n");
                newPlayerInd = driver.ntwk.next_ind;
                int indx = driver.ntwk.TryAddClient();
                if (indx >= 0) {
                    Player* joinedPlayer = driver.map.SpawnEntity<Player>();
                    joinedPlayer->multiplayerID = indx;
                    joinedPlayer->online = true;
                    joinedPlayer->SetWidth(25);
                    joinedPlayer->SetDepth(40);
                    driver.clientEntities[indx] = joinedPlayer;
                    // Send starter data to player client
                    driver.SendAllEntities(driver.ntwk.clientSockets[indx]);
                }
            }
            
            for (int socketIndx = 0; socketIndx < driver.ntwk.MAX_SOCKETS; socketIndx++) {
                if (newPlayerInd == socketIndx) {
                    continue; // i think client might be saying its ready right after but has not packets
                }
                if (!SDLNet_SocketReady(driver.ntwk.clientSockets[socketIndx])) {
                    continue;
                }

                driver.logger.Emit("incoming packets from client %d\n", socketIndx);
                std::vector<Packet> consumedPackets;
                driver.ntwk.ConsumePackets(driver.ntwk.clientSockets[socketIndx], consumedPackets);

                if (consumedPackets.size() == 0) {
                    driver.logger.Emit("error in reading packets from client %d\n", socketIndx);
                }
                // process client packet
                for (auto packet : consumedPackets) {
                    // is it the client moving?
                    if (packet.flags.test(Packet::Flag_t::bMoving)) {
                        driver.logger.Emit("client %d sent moving packet\n", socketIndx);
                        // update our server map
                        ImMoving moving = packet.ReadAsType<ImMoving>();
                        Player* client = driver.clientEntities[socketIndx];
                        client->Move(moving.xOff, moving.yOff);

                        // send notifcation to the other clients about this update
                        Packet outgoingPacket(Packet::Flag_t::bMoving);
                        EntityMoveUpdate data = EntityMoveUpdate {client->ID, moving.xOff, moving.yOff};
                        outgoingPacket.Encode<EntityMoveUpdate>(data);
                        driver.SendPacketToAllButOne(outgoingPacket, socketIndx);
                    }
                }
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