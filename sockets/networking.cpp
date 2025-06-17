#include "Networking.hpp"
#include "../util.hpp"

Networking::Networking() : 
    connected(false), serverSoc(nullptr), socket_set(nullptr) {
}

Networking::~Networking() {
    if (connected) {
        if(SDLNet_TCP_DelSocket(socket_set, serverSoc) != -1) {
            SDLNet_TCP_Close(serverSoc);
            SDLNet_FreeSocketSet(socket_set);
            SDLNet_Quit();
        }
        else {
            Log::error("ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
        }
    }
}

const char* Packet::ToString() {
    // needs safety, rushed
    if (flags.test(Packet::Flag_t::bNewEntity)) {
        description = std::string("new entity packet: typeID: ") + std::to_string(polyTypeID);
    }
    else {
        description = std::string("dont care packet");
    }
    return description.c_str();
}

int Networking::SendPacket(TCPsocket& socket, Packet& packet) {
    if (packet.Size() > max_packet_size) {
        Log::error("Trying to send packet larger than buffer size allows."
            "\n\tpacket.size: %d, maximum: %d\n", packet.Size(), max_packet_size);
    }
    uint8_t temp_data[max_packet_size];
    packet.WriteBuffer(&temp_data[0]);

    int num_sent = SDLNet_TCP_Send(socket, temp_data, (int)packet.Size());
    if(num_sent < packet.Size()) {
        Log::error("ER: SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    }

    Log::emit("Sent packet: %s\n", packet.ToString());
    return num_sent; 
}

Packet Networking::ConsumePacket(TCPsocket& socket) {
    uint8_t temp_data[max_packet_size];
    int num_recv = SDLNet_TCP_Recv(socket, temp_data, max_packet_size);
    Log::emit("Consumed %d packets\n", num_recv);

    if(num_recv <= 0) {
        const char* errmsg = SDLNet_GetError();
        if(errmsg && strlen(errmsg) > 0) {
            Log::emit("SDLNet_TCP_Recv: %sn\n", errmsg);
        }
        Log::error("Error in ConsumePacket, num_recv=%d\n", num_recv);
        return Packet(); // empty constructor returns a non-valid Data object
    } 

    Packet p = Packet::TCPToPacket(&temp_data[0], num_recv);
    Log::emit("Consumed packet(%dbytes): %s\n", num_recv, p.ToString());
    return p;
}

void Networking::CloseSocket(TCPsocket* socket) {
    if (!socket) {
        // we should probably assert if we are trying to close a null socket
        assert(false);
        return;
    }

    if(SDLNet_TCP_DelSocket(socket_set, *socket) == -1) {
        Log::error("ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
    }

    SDLNet_TCP_Close(*socket);
    socket = nullptr;
}

// NetworkHelper
// todo: make this a singleton

Server::Server() : next_ind(0) {
    memset(&clientSockets[0], 0, sizeof(TCPsocket[MAX_SOCKETS]));
}

Server::~Server() {
    // Remember, base class destructor is called automatically, after this one
    for (int i=0; i<MAX_SOCKETS; ++i) {
        if(clientSockets[i] == nullptr) continue;
        CloseSocket(&clientSockets[i]);
    }
}

bool Server::Setup() {
    if(SDLNet_Init() == -1) {
        printf("ER: SDLNet_Init: %s\n", SDLNet_GetError());
        return false;
    }

    if(SDLNet_ResolveHost(&serverIp, nullptr, server_port_num) == -1) {
        printf("ER: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        assert(false);
        return false;
    }
    
    serverSoc = SDLNet_TCP_Open(&serverIp);
    if(serverSoc == NULL) {
        printf("ER: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        __debugbreak();
        assert(false);
        return false;
    }

    socket_set = SDLNet_AllocSocketSet(MAX_SOCKETS + 1); // server socket is always created
    if(!socket_set) {
        printf("ER: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        assert(false);
        return false;
    }

    if(SDLNet_TCP_AddSocket(socket_set, serverSoc) == -1) {
        printf("ER: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
        assert(false);
        return false;
    }

    connected = true;
    return true;
}

// returns index in clientSockets of newest client
// if it failed, returns -1
int Server::TryAddClient() {
    if (next_ind >= MAX_SOCKETS) {
        // could choose to override sockets here, probably should
        return -1;
    }

    clientSockets[next_ind] = SDLNet_TCP_Accept(serverSoc);

    if (clientSockets[next_ind] == nullptr) {
        return -1;
    }

    if (SDLNet_TCP_AddSocket(socket_set, clientSockets[next_ind]) == -1) {
        printf("ER: SDLNet_TCP_AddSocket: %sn", SDLNet_GetError());
        exit(-1);
    }

    printf("DB: new connection (next_ind = %d)\n", next_ind);
    return next_ind++; // btw next_ind++ returns the value before its incremented
}

// Client

Client::Client() : serverHostname("") {}

Client::Client(std::string _serverHostname) :
    serverHostname(_serverHostname) {
}

bool Client::Connect() {
    if (serverHostname.empty()) {
        return false;
    }

    if(SDLNet_Init() == -1) {
        printf("ER: SDLNet_Init: %s\n", SDLNet_GetError());
        return false;
    }

    if(SDLNet_ResolveHost(&serverIp, serverHostname.c_str(), server_port_num) == -1) {
        printf("ER: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        assert(false);
        return false;
    }
    
    serverSoc = SDLNet_TCP_Open(&serverIp);
    if(serverSoc == NULL) {
        printf("ER: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        __debugbreak();
        assert(false);
        return false;
    }

    socket_set = SDLNet_AllocSocketSet(1);
    if(!socket_set) {
        printf("ER: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        assert(false);
        return false;
    }

    if(SDLNet_TCP_AddSocket(socket_set, serverSoc) == -1) {
        printf("ER: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
        assert(false);
        return false;
    }

    connected = true;
    return true;
}

// Packet functions
// ----------------
void Packet::WriteBuffer(void* buffer) {
    memcpy(buffer, &flags.bits, sizeof(flags.bits));
    buffer = (char*) buffer + sizeof(flags.bits);
    memcpy(buffer, &polyTypeID, sizeof(polyTypeID));
    buffer = (char*) buffer + sizeof(polyTypeID);
    memcpy(buffer, data.get(), size);
}

Packet Packet::TCPToPacket(void* p_packet, size_t _size) {

    if (_size - sizeof(Flag_t::bits_t) < 0) {
        Log::error("Error in TCPToPacket\n");
    }
    Packet newPacket;
    newPacket.flags = *reinterpret_cast<Flag_t::bits_t*>(p_packet);
    p_packet = (char*) p_packet + sizeof(Flag_t::bits_t);

    newPacket.polyTypeID = *reinterpret_cast<short*>(p_packet);
    p_packet = (char*) p_packet + sizeof(short);

    newPacket.data = std::make_shared<char[]>(_size - sizeof(Flag_t::bits_t));
    memcpy(newPacket.data.get(), p_packet, _size - sizeof(Flag_t::bits_t));

    return newPacket;
}