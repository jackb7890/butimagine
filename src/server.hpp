class ServerDriver {
    public:
    Map map;
    Server ntwk;
    std::array<Player*, Server::MAX_SOCKETS> clientEntities; // move this to map.players
    Log logger;
    int timeout;

    std::vector<std::string> ProcessArgs(int argc, char* argv[]);
    void SendAllEntities(TCPsocket socket);
    void SendPacketToAllButOne(Packet p, int socketIndx);
    void ProcessMoveData(std::vector<Player>& clientPlayers, Packet& packet, int client);

    void Initialize();
    void Cleanup();
};