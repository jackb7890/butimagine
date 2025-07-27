#pragma once

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include "util.hpp"
#include "sockets/networking.hpp"

#include <unordered_map>

#include <type_traits>
#include <list>
#include <string>

//#define MAP_WIDTH 1280
//#define MAP_HEIGHT 720

#define MAP_WIDTH 720
#define MAP_HEIGHT 360

struct Map;
struct Display;

struct MapEntityList {
    std::list<MapEntity*> list;

    void Add(MapEntity* en) {
        // assert tail.next is null
        for (auto el : list) {
            if (!el) {
                continue;
            }
            if (el == en) {
                return;
            }
        }
        list.push_back(en);
    }

    void Remove(MapEntity* en) {
        list.remove(en);
    }
};

class MapEntity {
    // base class for things that go on the map
    // for collosion detection sake, assume everything is rectangular for now
    // if mfers want triangles do it yourself

    public:
    bool valid;
    HitBox hitbox;
    RGBColor color;
    bool hasCollision;
    size_t ID;

    // Currently the way it works is player has 2 positions.
    // the second position is to store the players updated position
    // That way we can keep track of the old position and remove it from the screen
    // There will be problems if you move the player multiple times without drawing to the screen
    // Because then the Display::Update(Player) func will "erase" pixels thinking the player
    // was there, however, the pixels were never drawn to the display
    bool hasMovedOffScreen = false;
    GridPos oldPos;

    Map* map;

    public:
    MapEntity() {}
    MapEntity(Map* _map, size_t _ID) : map(_map), ID(_ID) {}
    MapEntity(HitBox _hb, RGBColor _c, Map* _map, bool _hasCol = true);

    std::string ToString();

    inline void SetMap(Map* _map) {
        map = _map;
    }

    inline void SetID(int _ID) {
        ID = _ID;
    }

    inline GridPos GetCurrentPos() const {
        return hitbox.origin;
    }

    inline GridPos GetOldPos() const {
        return oldPos;
    }

    inline int GetWidth() const {
        return hitbox.dim.width;
    }

    inline int GetDepth() const {
        return hitbox.dim.depth;
    }

    inline void SetWidth(int w) {
        hitbox.dim.width = w;
    }

    inline void SetDepth(int d) {
        hitbox.dim.depth = d;
    }

    inline void SetPos(int x, int y) {
        hitbox.origin.x = x;
        hitbox.origin.y = y;
    }

    inline void SetPos(GridPos pos) {
        hitbox.origin = pos;
    }

    inline SDL_Rect GetSDLRect() const {
        return SDL_Rect {GetCurrentPos().x, GetCurrentPos().y, GetWidth(), GetDepth()};
    }

    void MoveHoriz(int xD);
    void MoveVert(int yD);
    void Move(int xD, int yD);
    void UpdateGrid(HitBox oldArea, HitBox newArea);

    virtual int GetTypeIndex() {
        return TypeDetails<MapEntity>::index;
    }
};

class Wall : public MapEntity {
    private:
    bool isVert;
    const static int thickness = 2;

    public:
    // default color 112,112,112 is gray
    Wall() {}
    Wall(GridPos _pos, int _length, bool _isV, RGBColor _c, Map* _map);

    virtual int GetTypeIndex() {
        return TypeDetails<Wall>::index;
    }
};

class Player : public MapEntity {
    public:
    
    int health = 100;
    int runEnergy = 100;
    Uint32 born = 0;
    Uint32 lastUpdate = 0;

    int multiplayerID;
    bool online;

    inline Player() {}
    inline Player(HitBox _hb, RGBColor _c, Map* _map) :
        MapEntity(_hb, _c, _map) {}

    inline Player(Map* _map, size_t ID) : 
        MapEntity(_map, ID) {}

    virtual int GetTypeIndex() {
        return TypeDetails<Player>::index;
    }

    // pretty sure we can remove this but not checking rn
    friend struct Display;
};

template <typename T>
concept MapEntityT = std::is_base_of<MapEntity, T>::value;

struct Map {

    static constexpr unsigned int width = MAP_WIDTH;
    static constexpr unsigned int height = MAP_HEIGHT;

    int numberOfEntities = 0;
    Arr2d<MapEntityList> grid;

    std::vector<Player*> players;
    std::vector<MapEntity*> allEntities;
    std::vector<MapEntity*> drawMeBuf;

    // npcs

    // walls

    Map ();
    ~Map ();

    template <MapEntityT T>
    T* SpawnEntity() {
        T* newEntity = AllocateNewEntity<T>();
        newEntity->SetMap(this);
        newEntity->SetID(numberOfEntities++);
        return newEntity;
    }

    template <MapEntityT T>
    T* SpawnEntity(const T& copy) {
        T* newEntity = AllocateNewEntity<T>(copy);
        newEntity->SetMap(this);
        newEntity->SetID(numberOfEntities++);
        return newEntity;
    }

    template <MapEntityT T>
    T* AllocateNewEntity() {
        T* newEntity = new T();
        allEntities.push_back(newEntity);
        return newEntity;
    }

    template <MapEntityT T>
    T* AllocateNewEntity(const T& copy) {
        T* newEntity = new T(copy);
        allEntities.push_back(newEntity);
        return newEntity;
    }

    // Drawing each pixel based on each entry of grid for the map
    // will be slow compared to if we can do some SDL_FillRects, but
    // idk how to we'd do that
    void InitializeWorld();

    void InitializeWorld2();

    // Clears the map at area covered by player
    void Clear(Player player);

    MapEntity* GetEntity(size_t id);

    // These add functions add data that get's drawn differently
    // than drawing via display.Update(wall) or for player.
    // For example, if we add a wall to the map, it will store wall.color.r
    // in the map, and when we display.Update(map) it will use wall.color.r to color that pixel
    // But if we do display.Update(wall) it will use the full rgb color correctly.

    // I think to fix this we should start storing entire objects in the grid,
    // and to do that, we will need to make them the same type. So that means we gotta
    // add inheritence aka a parent class for wall and player called like MapEntry or something
    // Then we have a grid full of MapEntry objects, some of which are players, some of which are walls.


    void Add(MapEntity* entity);
    void Clear(MapEntity* entity);

    bool CheckForCollision(const HitBox& movingPiece, size_t ID);
};

// TODO move this to util
struct Display {
    SDL_Window* window = nullptr;
    //A- Rendeder added to display
    SDL_Renderer* renderer = nullptr;
    SDL_Surface* surface = nullptr;
    Map* map = nullptr;

    Display() {};
    Display(SDL_Window* _w, SDL_Renderer* _r, Map* map);
    Display(SDL_Window* _w, Map* map);
    Display(Map* map);

    ~Display();

    void DrawBackground();
    void DrawEntity(MapEntity entity);
    void DrawFrame(std::vector<MapEntity*> entities);
};