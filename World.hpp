#include <array>
#include "SDL.h"

#define MAP_WIDTH 640
#define MAP_HEIGHT 480

struct Map;
struct Display;

struct GridPos {
    int x, y;

    inline GridPos() : x(0), y(0) {}
    inline GridPos(int _x, int _y) : x(_x), y(_y) {}
};

struct Player {
    private:
    // Currently the way it works is player has 2 positions.
    // the second position is to store the players updated position
    // That way we can keep track of the old position and remove it from the screen
    // There will be problems if you move the player multiple times without drawing to the screen
    // Because then the Display::Update(Player) func will "erase" pixels thinking the player
    // was there, however, the pixels were never drawn to the display
    bool hasMovedOffScreen = false;

    public:
    GridPos position;
    GridPos oldPos;
    int width = 10;
    int height = 10;
    int health = 100;
    int runEnergy = 100;
    int playerID = 333; //using this as their color
    
    Map* map;

    friend struct Display;
};

struct Map {
    typedef std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> arr2d;
    arr2d grid;

    // npcs

    // walls

    Map ();

    // Drawing each pixel based on each entry of grid for the map
    // will be slow compared to if we can do some SDL_FillRects, but
    // idk how to we'd do that
    void SetStartMap();

    // Clears the map at area covered by player
    void Clear(Player player);

    // Adds a player to the map
    void Add(Player player);
};

struct Display {
    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;

    Display(SDL_Window* _w);

    ~Display();

    void Update(Map map, bool updateScreen = true);

    void Erase(Player player, bool updateScreen = true);

    void Update(Player player, bool updateScreen = true);
};
