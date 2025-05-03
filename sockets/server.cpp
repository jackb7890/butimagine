#include "SDL2/SDL_net.h"
#include <cstdio>
#include <string.h>
// tutorial maybe https://stephenmeier.net/2015/12/23/sdl-2-0-tutorial-04-networking/
struct Packet {
    union {
        unsigned short bits : 16;
        struct _flags {
            unsigned short wKeyPress : 1;
            unsigned short wKeyRelease : 1;
            unsigned short aKeyPress : 1;
            unsigned short aKeyRelease : 1;
            unsigned short sKeyPress : 1;
            unsigned short sKeyRelease : 1;
            unsigned short dKeyPress : 1;
            unsigned short dKeyRelease : 1;
            unsigned short unused : 8;
        } flags;
    };
    
    inline bool IsQuitCode() {
        return bits == 0; 
    }

    inline int WKeyPress() {
        return flags.wKeyPress;
    }

    inline int WKeyRelease() {
        return flags.wKeyRelease;
    }
};

struct NetworkHelper {
    // todo: make this a singleton

    static const int MAX_PACKETS = 0xFF; // 255
    static const int MAX_SOCKETS = 0x10; // 16

    static const int SERVER_PORT = 8099;

    int next_ind;
    TCPsocket serverSoc;
    SDLNet_SocketSet socket_set;
    TCPsocket clientSoc1;
    bool soc1Setup;
    IPaddress ip;

    NetworkHelper() : next_ind(0), soc1Setup(false) {
        init();

        if(SDLNet_ResolveHost(&ip, NULL, SERVER_PORT) == -1) {
            fprintf(stderr, "ER: SDLNet_ResolveHost: %sn", SDLNet_GetError());
            exit(-1);
        }
        
        serverSoc = SDLNet_TCP_Open(&ip);
        if(serverSoc == NULL) {
            fprintf(stderr, "ER: SDLNet_TCP_Open: %sn", SDLNet_GetError());
            exit(-1);
        }

        socket_set = SDLNet_AllocSocketSet(MAX_SOCKETS+1);
        if(socket_set == NULL) {
            fprintf(stderr, "ER: SDLNet_AllocSocketSet: %sn", SDLNet_GetError());
            exit(-1);
        }
        
        if(SDLNet_TCP_AddSocket(socket_set, serverSoc) == -1) {
            fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %sn", SDLNet_GetError());
            exit(-1);
        }
    }

    ~NetworkHelper() {
        if(SDLNet_TCP_DelSocket(socket_set, serverSoc) == -1) {
            fprintf(stderr, "ER: SDLNet_TCP_DelSocket: %sn", SDLNet_GetError());
            exit(-1);
        } SDLNet_TCP_Close(serverSoc);

        CloseSocket(clientSoc1);
        
        // int i;
        // for(i=0; i<MAX_SOCKETS; ++i) {
        //     if(sockets[i] == NULL) continue;
        //     CloseSocket(i);
        // }
        
        SDLNet_FreeSocketSet(socket_set);
        SDLNet_Quit();
        SDL_Quit();
    }

    bool TryAddClient() {
        if (soc1Setup) {
            return false;
        }

        clientSoc1 = SDLNet_TCP_Accept(serverSoc);

        if (clientSoc1 == nullptr) {
            return false;
        }

        if (SDLNet_TCP_AddSocket(socket_set, clientSoc1) == -1) {
            fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %sn", SDLNet_GetError());
            exit(-1);
        }
    
        printf("DB: new connection (next_ind = %d)n", next_ind);

        soc1Setup = true;
        return true;
    }

    void SendData(TCPsocket socket, uint8_t* data, uint16_t length, uint16_t flag) {
        uint8_t temp_data[MAX_PACKETS];
    
        int offset = 0;
        memcpy(temp_data+offset, &flag, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        memcpy(temp_data+offset, data, length);
        offset += length;
    
        int num_sent = SDLNet_TCP_Send(sockets[index], temp_data, offset);
        if(num_sent < offset) {
            fprintf(stderr, "ER: SDLNet_TCP_Send: %sn", SDLNet_GetError());
            CloseSocket(index);
        }
    }

    uint8_t* RecvData(int index, uint16_t* length, uint16_t* flag) {
        uint8_t temp_data[MAX_PACKET];
        int num_recv = SDLNet_TCP_Recv(sockets[index], temp_data, MAX_PACKET);
    
        if(num_recv <= 0) {
            CloseSocket(index);
            const char* err = SDLNet_GetError();
            if(strlen(err) == 0) {
                printf("DB: client disconnectedn");
            } else {
                fprintf(stderr, "ER: SDLNet_TCP_Recv: %sn", err);
            }
    
            return NULL;
        } else {
            int offset = 0;
            *flag = *(uint16_t*) &temp_data[offset];
            offset += sizeof(uint16_t);
    
            *length = (num_recv-offset);
    
            uint8_t* data = (uint8_t*) malloc((num_recv-offset)*sizeof(uint8_t));
            memcpy(data, &temp_data[offset], (num_recv-offset));
    
            return data;
        }
    }

    private:
    void init() {
        if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
            fprintf(stderr, "ER: SDL_Init: %sn", SDL_GetError());
            exit(-1);
        }
 
        if(SDLNet_Init() == -1) {
            fprintf(stderr, "ER: SDLNet_Init: %sn", SDLNet_GetError());
            exit(-1);
        }
    }

    void CloseSocket(TCPsocket socket) {
        if(SDLNet_TCP_DelSocket(socket_set, socket) == -1) {
            fprintf(stderr, "ER: SDLNet_TCP_DelSocket: %sn", SDLNet_GetError());
            exit(-1);
        }
    
        SDLNet_TCP_Close(socket);
    }
};

int main() {

    const int WAIT_TIME = 5000; // ms

    // create server socket
    NetworkHelper ntwk;
    bool runLoop = true;

    while (runLoop) {
        int clients_ready = SDLNet_CheckSockets(ntwk.socket_set, 1000 /* 1000ms of checking time */);

        if (clients_ready <= 0) {
            continue;
        }

        // clients are ready, using just one for now

        if(SDLNet_SocketReady(ntwk.serverSoc)) {
            // client connection ocurred
            ntwk.TryAddClient();
        }

        if (SDLNet_SocketReady(ntwk.clientSoc1)) {
            uint8_t* data;
            uint16_t flag;
            uint16_t length;
                        
            data = ntwk.RecvData(0, &length, &flag);
            if(data == NULL) {
                continue;
            }
        
            switch(flag) {
                case FLAG_WOOD_UPDATE: {
                    uint16_t offset = 0;
                    uint8_t send_data[MAX_PACKET];
        
                    memcpy(send_data+offset, &clients[ind].amt_wood, sizeof(uint8_t));
                    offset += sizeof(uint8_t);
        
                    SendData(ind, send_data, offset, FLAG_WOOD_UPDATE);
                } break;
        
                case FLAG_QUIT: {
                    running = 0;
                    printf("DB: shutdown by client id: %dn", ind);
                } break;
            }
        
            free(data);
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
    }

    return 0;
}