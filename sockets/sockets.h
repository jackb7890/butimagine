#include "SDL_net.h"
#include <cstdio>
#include <string>

struct FlagsType {
    static const int Handshake = 0x8000;
    union {
        unsigned short bits : 16;
        struct _flags {
            unsigned short isHandshake : 1;

            unsigned short wKeyPress : 1;
            unsigned short wKeyRelease : 1;
            unsigned short aKeyPress : 1;
            unsigned short aKeyRelease : 1;
            unsigned short sKeyPress : 1;
            unsigned short sKeyRelease : 1;
            unsigned short dKeyPress : 1;
            unsigned short dKeyRelease : 1;

            unsigned short unused : 7;
        } flags;
    };

    FlagsType(unsigned short _bits) : bits(_bits) {}

    inline static constexpr int GetFlagsSize() {
        return sizeof(flags);
    }
    
    inline bool IsQuitCode();

    inline int WKeyPress();

    inline int WKeyRelease();

    inline void Init(uint16_t flagdata);
    
    FlagsType();
};

struct Data {

    bool isValid;
    uint8_t* data;
    FlagsType dataFlags;
    int size; // size of data, not including the flag data
    Data();

    Data(uint8_t* rawTempData, int _size);

    bool GetRawData(uint8_t* out);
    static Data CreateDummyData();

    ~Data();
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

    int SendData(TCPsocket& socket, Data data);
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
    bool TryAddClient();

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