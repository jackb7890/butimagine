#include "sockets.h"
#include <assert.h>
#include <string>

// FlagsType

bool FlagsType::IsQuit() {
    return flags.quit != 0;
}

bool FlagsType::IsMove() {
    return flags.move != 0;
}

void FlagsType::SetQuit() {
    flags.quit = 1;
}

void FlagsType::SetMove() {
    flags.move = 1;
}

void FlagsType::Init(uint16_t flagdata) {
    bits = flagdata;
}

FlagsType::FlagsType() {
    bits = 0x0000;
}

// Data

Data::Data() : data(nullptr), isValid(false), size(0) {}

Data::Data(uint8_t* rawTempData, int size) {
    // first 16-bits of one TCP message are going to stand for some status flags
    dataFlags = 0;

    // assert this
    if (data != nullptr) {
        // assert
    }
    data = (uint8_t*) malloc((size)*sizeof(uint8_t));
    memcpy(data, rawTempData, size);
    this->size = size;
    this->isValid = true;
}

bool Data::GetRawData(uint8_t* out) {
    if (!isValid) {
        return false;
    }
    if (!out) {
        return false;
    }

    *reinterpret_cast<uint16_t*>(out) = dataFlags.bits;
    memcpy(out + FlagsType::GetFlagsSize(), data, size);
    return true;
}

Data Data::CreateHollowData() {
    Data data;
    data.data = nullptr;
    data.dataFlags = 0;
    data.size = 0;
    data.isValid = true;
    return data;
}

Data Data::CreateTestData() {
    return Data(reinterpret_cast<uint8_t*>("jack"), 5);
}

void Data::AppendByte(uint8_t byte) {
    // ASSERT data != nullptr
    *(data + size) = byte;
    size++;
}

void Data::AppendMovementData(int8_t x, int8_t y) {
    if (size == 0) {
        // starting new list of datas
        data = (uint8_t *)malloc(2);
    }
    AppendByte(x);
    AppendByte(y);
}

Data::~Data() {
    if (data) {
        free(data);
    }
}

// NetworkHelperBase

NetworkHelperBase::NetworkHelperBase() : 
    isValid(false), serverSoc(nullptr), socket_set(nullptr) {
}

NetworkHelperBase::~NetworkHelperBase() {
    if (isValid) {
        if(SDLNet_TCP_DelSocket(socket_set, serverSoc) == -1) {
            printf("ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
            exit(-1);
        } SDLNet_TCP_Close(serverSoc);
        
        SDLNet_FreeSocketSet(socket_set);
        SDLNet_Quit();
    }
}

int NetworkHelperBase::SendData(TCPsocket& socket, Data& data) {
    uint8_t temp_data[MAX_PACKETS];
    if (!data.GetRawData(&temp_data[0])) {
        printf("Error in GetRawData\n");
        return -1;
    }

    int num_sent = SDLNet_TCP_Send(socket, temp_data, data.size);
    if(num_sent < data.size) {
        printf("ER: SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        return -1;
    }

    return num_sent; 
}

Data NetworkHelperBase::RecvData(TCPsocket& socket) {
    uint8_t temp_data[MAX_PACKETS];
    int num_recv = SDLNet_TCP_Recv(socket, temp_data, MAX_PACKETS);

    if(num_recv <= 0) {
        printf("num received: %d\n", num_recv);
        printf("Tried Receiving data on client socket, but there TCP_Recv read no data\n Error: ");
        const char* err = SDLNet_GetError();
        if(strlen(err) == 0) {
            printf("no error to emit\n");
        } else {
            printf("SDLNet_TCP_Recv: %sn\n", err);
        }

        return Data(); // empty constructor returns a non-valid Data object
    } else {
        printf("got Datas received: %d\n", num_recv);
        return Data(&temp_data[0], num_recv);
    }
}

void NetworkHelperBase::CloseSocket(TCPsocket* socket) {
    if (!socket) {
        // we should probably assert if we are trying to close a null socket
        assert(false);
        return;
    }

    if(SDLNet_TCP_DelSocket(socket_set, *socket) == -1) {
        printf("ER: SDLNet_TCP_DelSocket: %s\n", SDLNet_GetError());
        exit(-1);
    }

    SDLNet_TCP_Close(*socket);
    socket = nullptr;
}

void NetworkHelperBase::Validate() {
    assert(!isValid);
    isValid = true;
}

// NetworkHelper
// todo: make this a singleton

NetworkHelperServer::NetworkHelperServer() : next_ind(0) {
    memset(&clientSockets[0], 0, sizeof(TCPsocket[MAX_SOCKETS]));
}

NetworkHelperServer::~NetworkHelperServer() {
    // Remember, base class destructor is called automatically, after this one
    if (isValid) {
        for (int i=0; i<MAX_SOCKETS; ++i) {
            if(clientSockets[i] == nullptr) continue;
            CloseSocket(&clientSockets[i]);
        }
    }
}

bool NetworkHelperServer::Init() {
    if(SDLNet_Init() == -1) {
        printf("ER: SDLNet_Init: %s\n", SDLNet_GetError());
        return false;
    }

    if(SDLNet_ResolveHost(&serverIp, nullptr, SERVER_PORT) == -1) {
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

    Validate();
    return true;
}

bool NetworkHelperServer::TryAddClient() {
    if (next_ind >= MAX_SOCKETS) {
        // could choose to override sockets here, probably should
        return false;
    }

    clientSockets[next_ind] = SDLNet_TCP_Accept(serverSoc);

    if (clientSockets[next_ind] == nullptr) {
        return false;
    }

    if (SDLNet_TCP_AddSocket(socket_set, clientSockets[next_ind]) == -1) {
        printf("ER: SDLNet_TCP_AddSocket: %sn", SDLNet_GetError());
        exit(-1);
    }

    printf("DB: new connection (next_ind = %d)\n", next_ind);
    next_ind++;
    return true;
}

// NetworkHelperClient

NetworkHelperClient::NetworkHelperClient() : serverHostname("") {}

NetworkHelperClient::NetworkHelperClient(std::string _serverHostname) :
    serverHostname(_serverHostname) {
}

bool NetworkHelperClient::Init() {
    if (serverHostname.empty()) {
        return false;
    }

    if(SDLNet_Init() == -1) {
        printf("ER: SDLNet_Init: %s\n", SDLNet_GetError());
        return false;
    }

    if(SDLNet_ResolveHost(&serverIp, serverHostname.c_str(), SERVER_PORT) == -1) {
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

    Validate();
    return true;
}