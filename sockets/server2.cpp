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

#include "../World2.hpp"

#include "SDL_net.h"
#include "sockets.h"

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
    NetworkHelperServer ntwk;
    std::vector<ClientPlayer> players;
};

ServerDriver driver;

void ProcessMoveData(std::vector<Player>& clientPlayers, Data& data, int client) {
    // assert size is big enough
    if (data.data.size() > 0) {
        int8_t x, y;
        x = data.data[0];
        y = (data.data.size() > 1) ? data.data[1] : 0;
        
        clientPlayers[client].Move(x, y);

    }
}

void ProcessData(Data& data, int client) {
    // if (data.IsMove()) {
    //     ProcessMoveData(data, client);
    // }
}

bool Server_InitWorld() {
    driver.map.InitializeWorld();

    // not sure how I feel about the map updating through the player class but fuck it right
    HitBox player1HitBox = {MAP_WIDTH/2, MAP_HEIGHT/2, 10, 10};
    RGBColor player1Color = {120, 200, 200};
    Player player1(player1HitBox, player1Color, &driver.map);
    driver.map.Add(player1);
}

class ClientPlayer : public Player {
    public:
    int socketIndx;
    bool online;

    ClientPlayer (Map* _map) : Player(_map) {}

    void SendFreshConnectionClientData();
};

void ClientPlayer::SendFreshConnectionClientData() {
    auto socket = driver.ntwk.clientSockets[socketIndx];

    // Send them all the entities on the world

    driver.ntwk.SendData(socket, );
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

    const int MAX_PLAYERS = NetworkHelperServer::MAX_SOCKETS-1;
    std::vector<Player> clientPlayers;

    int timeoutTime = 50'000; // in ms, so 50 seconds
    if (args.size() != 0) {
        timeoutTime = stoi(args[0]);
    }
    Log::emit("timeout set to %dms\n", timeoutTime);

    init();

    Server_InitWorld();

    // Init network connection
    if (!(driver.ntwk.Init())) {
        Log::emit("Failed server init\n");
        __debugbreak();
        return -1;
    }
    
    bool runLoop = true;
    SDL_Event ev;

    Uint32 totalFrames = 0;
    Uint32 FPSTicks = SDL_GetTicks();
    //A- Target fps = fps cap. Change if you want to change the fps
    const Uint32 TARGET_FPS = 60;
    const Uint32 TICKS_PER_FRAME = 1000 / TARGET_FPS;

    //A- Velocity like position should be tracked by the player struct but i'm not doing that rn
    int Xelocity = 0;
    int Yelocity = 0;
    int speed = 20;
    const float GRAVITY = 12.0f;

    //A- An array of bools to track the 4 directions a player can move
    enum Velocity {XPOS, YPOS, XNEG, YNEG};
    std::array<bool, 4> vels;
    vels.fill(false);
    const int WAIT_TIME = 300;
    while (runLoop) {

        if (timeoutTime <= 0) {
            runLoop = false;
        }
        timeoutTime--;

        int clients_ready = SDLNet_CheckSockets(driver.ntwk.socket_set, WAIT_TIME);

        if (clients_ready == -1) {
            Log::emit("Error returned by SDLNet_CheckSockets\n");
        }
        else if (clients_ready == 0) {
            Log::emit("No clients ready at this time\n");
            // no clients ready, process things in server work queue
        }
        else if (clients_ready > 0) {
            // clients are ready
            Log::emit("One or more sockets ready\n");
            if(SDLNet_SocketReady(driver.ntwk.serverSoc)) {
                // client connection ocurred
                Log::emit("Server socket is ready\n)");
                int indx = driver.ntwk.TryAddClient();
                if (indx > 0) {
                    ClientPlayer joinedPlayer = ClientPlayer(&driver.map);
                    joinedPlayer.socketIndx = indx;
                    joinedPlayer.online = true;
                    driver.players.push_back(joinedPlayer);

                    // Send starter data to player client
                }

            }

            for (int socketIndx = 0; socketIndx < driver.ntwk.MAX_SOCKETS; socketIndx++) {
                if (!SDLNet_SocketReady(driver.ntwk.clientSockets[socketIndx])) {
                    continue;
                }

                Log::emit("Client socket is ready\n)");

                Data data = driver.ntwk.RecvData(driver.ntwk.clientSockets[socketIndx]);

                // if (data.IsMove()) {

                // }
                // RespondToClients()
            
                // switch(data.dataFlags.bits) {
                    // case FLAG_WOOD_UPDATE: {
                    //     Data dataToSend; // TODO: fill this in with real data
                    //     driver.ntwk.SendData(socketIndx, dataToSend);
                    // } break;
            
                    // case FLAG_QUIT: {
                    //     runLoop = false;
                    //     printf("DB: shutdown by client id: %dn", socketIndx);
                    // } break;
                //}        
            }
        }



        // ---------------------------------------
        Data& tickData = Data::CreateHollowData();
        //A- Timing starts at beginnig of core loop
        //A- SDL_GetTicks() is the global timer in ms
        Uint32 startTick = SDL_GetTicks();
        while (SDL_PollEvent(&ev) != 0) {
            switch (ev.type) {
            case SDL_QUIT:
                runLoop = HandleQuitEv(tickData);
                tickData.dataFlags.SetQuit();
                break;
            //A- Instead of directly increasing/decreasing velocity, pressing a key sets a flag to true, while releasing it sets the flag to false
            //A- We had an issue before where holding a key down didn't register until it's been held for a full second
            //A- With this method, A key is considred to always be held down until a key up input is read
            //A- Changing directions and holding multiple directions now works as intended
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = ev.key.keysym.sym;
                switch (key) {
                case SDLK_w:
                    vels[YNEG] = true;
                    break;
                case SDLK_a:
                    vels[XNEG] = true;
                    break;
                case SDLK_s:
                    vels[YPOS] = true;
                    break;
                case SDLK_d:
                    vels[XPOS] = true;
                    break;
                default:
                    break;
                }
            }
            break;
            case SDL_KEYUP:
            {
                SDL_Keycode key = HandleKeyDnEv(ev.key);
                switch (key) {
                case SDLK_w:
                    vels[YNEG] = false;
                    break;
                case SDLK_a:
                    vels[XNEG] = false;
                    break;
                case SDLK_s:
                    vels[YPOS] = false;
                    break;
                case SDLK_d:
                    vels[XPOS] = false;
                    break;
                default:
                    break;
                }
            }
            break;
            default:
                break;
            }
        }
        //A- Calc the average FPS, useful for debugging the framerate cap
        //A- Very first frame will be way higher than it should be, which is what the if corrects.
        float avgFPS = totalFrames / (((Uint32)SDL_GetTicks() - FPSTicks) / 1000.f);
        if (avgFPS > 2000000)
        {
            avgFPS = 0;
        }

        //A- Changes velocity accoring to what buttons are pressed
        if (vels[XPOS] && !vels[XNEG]) {
            Xelocity += speed;
        }
        if (vels[XNEG] && !vels[XPOS]) {
            Xelocity -= speed;
        }
        if (vels[YPOS] && !vels[YNEG]) {
            Yelocity += speed;
        }
        if (vels[YNEG] && !vels[YPOS]) {
            Yelocity -= speed;
        }

        // deceleration combinations
        // slow down when theres no buttons being pressed

        if (!vels[XPOS] && !vels[XNEG]) {
            if (Xelocity >= 0 && Xelocity < GRAVITY) {
                Xelocity = 0;
            }
            else {
                Xelocity -= Xelocity > 0 ? GRAVITY : -GRAVITY;
            }
        }
        if (!vels[YPOS] && !vels[YNEG]) {
            if (Yelocity >= 0 && Yelocity < GRAVITY) {
                Yelocity = 0;
            }
            else {
                Yelocity -= Yelocity > 0 ? GRAVITY : -GRAVITY;
            }
        }
        //A- This block supposedly keeps physics independent of the current FPS
        //A- Very important as otherwise faster FPS = faster movement (bad)
        //A- Doesn't seem to work, or doesn't work as much as intended, still figuring out why
        //A- Currently player still moves faster at a faster FPS, but maybe not by as much(?)
        Uint32 time = SDL_GetTicks();
        float dT = (time - player1.lastUpdate) / 1000.0f;

        //A- Move the player if velocity isn't 0
        if (Xelocity != 0 || Yelocity != 0) {
            tickData.dataFlags.SetMove();
            tickData.AppendMovementData(Xelocity, Yelocity);
            player1.MoveVert(Yelocity * dT);
            player1.MoveHoriz(Xelocity * dT);
        }
        //A- Re-render the player
        display.Update(player1);
        //A- Update for DT
        player1.lastUpdate = SDL_GetTicks();

        //A- End of loop, increase frame counter & get end time.
        totalFrames++;
        Uint32 endTick = SDL_GetTicks();

        //A- If frame finished earlier than FPS cap (which it will)
        int frameTicks = endTick - startTick;
        if (frameTicks < TICKS_PER_FRAME){
            //A- Delay next frame according to FPS cap
            SDL_Delay(TICKS_PER_FRAME - frameTicks);
        }

        driver.ntwk.SendData(driver.ntwk.serverSoc, tickData);
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