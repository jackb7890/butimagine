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
    this->description = std::string("packet datasize: ") + std::to_string(size) + std::string(" ");
    if (flags.test(Flag_t::bNewEntity)) {
        description += std::string("new entity packet: typeID: ") + std::to_string(polyTypeID);
    }
    else if (flags.test(Flag_t::bQuit)) {
        description += std::string("quit packet");
    }
    else if (flags.test(Flag_t::bMoving)) {
        description += std::string("movement packet");
    }
    else if (flags.test(Flag_t::bEndOfPacketGroup)) {
        description += std::string("packet list terminator packet");
    }
    else {
        description += std::string("unknown packet type");
    }
    return description.c_str();
}

int Networking::SendPacket(TCPsocket& socket, Packet& packet) {
    if (packet.EncodeSize() > max_packet_size) {
        Log::error("Trying to send packet larger than buffer size allows."
            "\n\tpacket.size: %d, maximum: %d\n", packet.EncodeSize(), max_packet_size);
    }
    uint8_t temp_data[max_packet_size];
    packet.WriteBuffer(&temp_data[0]);

    int num_sent = SDLNet_TCP_Send(socket, temp_data, (int)packet.EncodeSize());
    if(num_sent < packet.EncodeSize()) {
        Log::error("ER: SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    }

    Log::emit("Sent packet: %s\n", packet.ToString());
    return num_sent; 
}

void Networking::ConsumePackets(TCPsocket& socket, std::vector<Packet>& packetsOut) {
    uint8_t temp_data[max_packet_size];
    int num_recv = SDLNet_TCP_Recv(socket, temp_data, max_packet_size);
    Log::emit("Consumed %d packets\n", num_recv);

    if(num_recv <= 0) {
        const char* errmsg = SDLNet_GetError();
        if(errmsg && strlen(errmsg) > 0) {
            Log::emit("SDLNet_TCP_Recv: %sn\n", errmsg);
        }
        Log::error("Error in ConsumePacket, num_recv=%d\n", num_recv);
        return;
    } 

    Packet::TCPToPackets(&temp_data[0], num_recv, packetsOut);

    if (packetsOut.empty()) {
        Log::emit("ConsumePackets failed: no packet read\n");
        return;
    }
    
    Log::emit("Consumed packet(%dbytes): %s\n", num_recv, packetsOut[0].ToString());
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
    Log::emit("Writing Packet to buffer...\n");

    memcpy(buffer, &size, sizeof(size));
    Log::emit("Encoding %d bytes for size(%d)\n", sizeof(size), size);
    buffer = (char*) buffer + sizeof(size); 

    memcpy(buffer, &flags.bits, sizeof(flags.bits));
    Log::emit("Encoding %d bytes for flags.bits(%d)\n", sizeof(flags.bits), size);
    buffer = (char*) buffer + sizeof(flags.bits);

    memcpy(buffer, &polyTypeID, sizeof(polyTypeID));
    Log::emit("Encoding %d bytes for polyTypeID(%d)\n", sizeof(polyTypeID), polyTypeID);
    buffer = (char*) buffer + sizeof(polyTypeID);

    memcpy(buffer, data.get(), size);
    Log::emit("Encoding %d bytes for data\n", size);

    size_t totalSize = sizeof(size) + sizeof(flags.bits) + sizeof(polyTypeID) + size;
    Log::emit("Total encoded size: %d\n", totalSize);
}

void Networking::PushUnfinishedPacket(Packet p) {
    if (buf1Count == buf1Size) {
        // buf1 (array) is full, use buf2
        buf2.push(p);
    }
    else {
        // save in quicker access buf1
        buf1[buf1Next] = p;
        buf1Count++;
        buf1Next = (buf1Next + 1 ) % buf1Size;
    }   
}

Packet Networking::PopUnfinishedPacket() {
    if (buf1Count == 0 && buf2.size() == 0) {
        // error?
        return Packet();
    }

    if (buf1Count != 0) {
        // buf1 (array) is full, use buf2
        buf1Next = (buf1Next - 1 ) % buf1Size;
        return buf1[buf1Next];
    }
    else {
        // get from buf2
        Packet retVal = buf2.front();
        buf2.pop();
        return retVal;
    }   
}

// advances the pointer to pointer p_packet
Packet Packet::TCPToPacket(void** p_packet, size_t _size, size_t* remaining = nullptr) {
    // assert size > 0
    if (remaining) {
        *remaining = 0;
    }
    // we need something to make sure we at least have size data to read
    if (_size < EXPECTED_BASE_PACKET_SIZE) {
        Log::emit("TcpToPacket failed: _size too small\n");
        return Packet();
    }

    Packet newPacket;
    newPacket.size = *reinterpret_cast<decltype(size)*>(*p_packet);

    if (newPacket.size > MAX_INCOMING_PACKET_SIZE) {
        Log::emit("TcpToPacket failed: Packet size too large\n"
        "\tnewPacket: %d _size: %d\n");
        return Packet();
    }

    if (_size - EXPECTED_BASE_PACKET_SIZE < newPacket.size) {
        Log::emit("TcpToPacket: packet unfinished\n");
        newPacket.unfinished = true;
    }

    void* p_packet_orig = *p_packet;

    *p_packet = (char*) *p_packet + sizeof(decltype(size));
    newPacket.flags = *reinterpret_cast<Flag_t::bits_t*>(*p_packet);
    *p_packet = (char*) *p_packet + sizeof(Flag_t::bits_t);

    newPacket.polyTypeID = *reinterpret_cast<short*>(*p_packet);
    *p_packet = (char*) *p_packet + sizeof(short);

    newPacket.data = std::make_shared<char[]>(newPacket.size);
    memcpy(newPacket.data.get(), *p_packet, newPacket.size);

    *p_packet = (char*) *p_packet + newPacket.size;

    if (remaining) {
        *remaining  = _size - (((char*)*p_packet) - (char*)p_packet_orig);
        // assert this is 0 or more
    }
    return newPacket;
}

void Packet::TCPToPackets(void* p_packet, size_t _size, std::vector<Packet>& packetsOut) {
    // assert _size > 0

    size_t remaining;
    Packet newPacket = Packet::TCPToPacket(&p_packet, _size, &remaining);
    if (!newPacket.IsInvalid()) {
        packetsOut.push_back(newPacket);
    }

    if (remaining > 0) {
        Packet::TCPToPackets(p_packet, remaining, packetsOut);
    }
}