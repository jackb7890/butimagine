#include "SDL_net.h"
#include <cstdio>
#include <string>
#include <vector>
#include <memory>

class FlagsType {
    public:
    static const int QuitBit = 0x0001;
    static const int MoveBit = 0x0002;
    static const int EntityBit = 0x0004;
    
    static const int Default = 0x0000;
    union {
        unsigned short bits : 16;
        struct _flags {
            unsigned short quit : 1;
            unsigned short move : 1;
            unsigned short isEntityUpdate : 1;
            unsigned short unused : 13;
        } flags;
    };

    FlagsType(unsigned short _bits) : bits(_bits) {}

    inline static constexpr int GetFlagsSize() {
        return sizeof(flags);
    }
    
    bool IsQuit();
    bool IsMove();
    bool IsEntityUpdate();
    void SetQuit();
    void SetMove();
    void SetIsEntityUpdate();
    
    FlagsType();
};

class Data {
public:
    std::unique_ptr<void> ptr;
    size_t size;
    FlagsType dataFlags;
    bool alreadyEncoded;

    Data() : ptr(nullptr), size(0), dataFlags(FlagsType::Default), alreadyEncoded(false) {};

    template <typename T>
    static Data SerializeObject(T obj, FlagsType flags = FlagsType::Default) {
        Data retData;
        retData.ptr = std::unique_ptr<void>::make_unique(sizeof(T))
        memcpy(retData.ptr, &obj, sizeof(T));
        alreadyEncoded = true;
        return retData;
    }

    template <typename T>
    void Encode(T obj, FlagsType flags = FlagsType::Default) {
        if (alreadyEncoded) {
            Log::emit("uh oh reencoding data\n");
            return;
        }

        dataFlags = flags;
        ptr = std::unique_ptr<void>::make_unique(sizeof(T))
        memcpy(ptr, &obj, sizeof(T));
    }

    // old Data stuff
    Data(uint8_t* rawTempData, int _size);

    void GetRawData(uint8_t* out, int maxsize = 256);
    static Data CreateTestData();
    static Data CreateHollowData();

    // Movement data functions
    void AppendMovementData(int8_t x, int8_t y);
    void AppendKeyDown(SDL_Keycode key);

    ~Data();

    private:
    void AppendByte(uint8_t byte);
};

// Not meant to make objects of this type
// Abstract parent class for client and server classes
class NetworkHelperBase {
    public:
    static const int MAX_PACKETS = 0xFF; // 255
    static const int SERVER_PORT = 8099;

    bool isValid = false;

    SDLNet_SocketSet socket_set;
    IPaddress serverIp;
    TCPsocket serverSoc;

    int SendData(TCPsocket& socket, Data& data);
    Data RecvData(TCPsocket& socket);
    void CloseSocket(TCPsocket* socket);
    
    protected:
    NetworkHelperBase();
    ~NetworkHelperBase();

    // runs SDLNET_{Init, ResolveHost, TCP_Open}
    virtual bool Init() = 0;
    void Validate();
};

struct NetworkHelperServer : public NetworkHelperBase {
    // todo: make this a singleton
    static const int MAX_SOCKETS = 0x10; // max number of client sockets to hold
    int next_ind;

    TCPsocket clientSockets[MAX_SOCKETS];

    NetworkHelperServer();

    ~NetworkHelperServer();

    bool Init();
    int TryAddClient();

    private:
};

// NetworkHelperClient
struct NetworkHelperClient : public NetworkHelperBase {
    private:
    TCPsocket clientSocket;
    std::string serverHostname;
    public:
    NetworkHelperClient();
    NetworkHelperClient(std::string _serverHostname);
    // runs SDLNet_{AllocSocketSet(1), TCP_AddSocket(server)}, and Validate()
    bool Init();
};