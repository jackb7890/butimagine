#pragma once
// Everything necessary to transfer data between PCs

// SDL includes
#include "SDL_net.h"
// stdlib includes
#include <memory>
#include <string>
#include <typeinfo>
#include <cstring>

#include "../util.hpp"

// DataTypes Packet wraps
struct ImMoving {
    int8_t xOff;
    int8_t yOff;
};

struct EntityMoveUpdate {
    size_t id;
    int8_t xOff;
    int8_t yOff;
};

// A very broad type thats supports the
// send and receive functions
struct Packet {
    // private:
    short polyTypeID;

    public:
    struct alignas(short) Flag_t {
        typedef unsigned short bits_t;
        bits_t bits;
                
        static const bits_t bQuit = 0x0001;
        static const bits_t bMoving = 0x0002;
        static const bits_t bNewEntity = 0x0004;
        static const bits_t bEndOfPacketGroup = 0x0008;
        
        static const bits_t bNone = 0x0000;

        Flag_t(bits_t _bits) : bits(_bits) {}

        // not including meta data aka not full size of Flag_t.
        // just the amount of bits flags represents (and that will be transmitted)
        inline static constexpr int size() {
            return sizeof(bits);
        }

        // set the bits specified to 1
        inline void set(bits_t bit) {
            bits |= bit;
        }

        // clears the bits specfied to 0
        // E.g. if bit = b0010 0101 and bits was = b0111 1011
        // bits would become = b0101 1010
        inline void clear(bits_t bit) {
            bits &= ~bit;
        }

        inline bool test(bits_t testbits) {
            return (bits & testbits) != 0;
        }
    };

    std::shared_ptr<char[]> data;
    Flag_t flags;
    bool alreadyEncoded;
    size_t size;
    std::string description;

    Packet() : data(nullptr), size(0), flags(Flag_t::bNone), alreadyEncoded(false), description("") {};
    Packet(Flag_t::bits_t bits) : data(nullptr), size(0), flags(bits), alreadyEncoded(false) {};

    static Packet TCPToPacket(void* p_packet, size_t _size);
    const char* ToString();

    template <typename T>
    void Encode(T obj) {
        if (alreadyEncoded) {
            Log::error("uh oh reencoding data\n");
            return;
        }
        
        data = std::make_shared<char[sizeof(T)]>();
        memcpy(data.get(), &obj, sizeof(T));
        alreadyEncoded = true;
        size = sizeof(T) + flags.size() + sizeof(size);
    }

    // try to get rid of these
    template <typename T>
    void Encode(T obj, Flag_t _flags) {
        flags = _flags;
        Encode(obj);
    }

    template <typename T>
    void Encode(T obj, Flag_t::bits_t _flagbits) {
        flags = _flagbits;
        Encode(obj);
    }

    // Encode polymorphic obj (like MapEntity)
    // it marks a IsPolyType flag that tells the reader
    // that the first 32-bits of data is a unique type ID
    template <typename T>
    void EncodePoly(T obj, int typeID) {
        polyTypeID = typeID;
        Encode(obj);
    }

    template <typename T>
    void EncodePoly(T obj, int typeID, Flag_t _flags) {
        flags = _flags;
        EncodePoly(obj, typeID);
    }

    template <typename T>
    void EncodePoly(T obj, int typeID, Flag_t::bits_t _flagbits) {
        flags = _flagbits;
        EncodePoly(obj, typeID);
    }

    template <typename T>
    T ReadAsType(size_t byteoffset = 0) {
        T dataAsTypeT;
        std::memcpy(&dataAsTypeT, data.get() + byteoffset, sizeof(T));
        return dataAsTypeT;
    }

    // assumes its handed a ptr to already allocated memory
    // releases memory allocated by data
    void WriteBuffer(void* buffer);

    inline void SetFlags(unsigned short _flags) {
        flags.set(_flags);
    }

    inline void ClearFlags(unsigned short _flags) {
        flags.clear(_flags);
    }

    inline size_t Size() {
        return flags.size() + size;
    }
};

// Singleton class -- just kidding
// apparently those are bad, and i agree because they require
// using a mutex lock to prevent race conditons, and video game
// code will eventually (hopefully) become very multithreaded

// its not a singleton class, but a "controller" class :-)

// Central place for data and functions used to transmit data
// over ports
class Networking {
    public:
    static const int max_packet_size = 256; // (in bytes)
    static const int server_port_num = 8099;

    SDLNet_SocketSet socket_set;
    IPaddress serverIp;
    TCPsocket serverSoc;

    int SendPacket(TCPsocket& socket, Packet& Packet);
    Packet ConsumePacket(TCPsocket& socket);
    void CloseSocket(TCPsocket* socket);
    
    protected:

    bool connected;
    Networking();
    ~Networking();
};

struct Server : public Networking {
    static const int MAX_SOCKETS = 0x10; // max number of client sockets to hold
    int next_ind;

    TCPsocket clientSockets[MAX_SOCKETS];

    Server();

    ~Server();

    bool Setup();
    int TryAddClient();

    private:
};

// Client
struct Client : public Networking {
    private:
    TCPsocket clientSocket;
    std::string serverHostname;
    public:
    Client();
    Client(std::string _serverHostname);
    // runs SDLNet_{AllocSocketSet(1), TCP_AddSocket(server)}, and Validate()
    bool Connect();
};