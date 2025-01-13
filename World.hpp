#pragma once

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include "util.hpp"

#define MAP_WIDTH 1280
#define MAP_HEIGHT 720

struct Map;
struct Display;

struct GridPos {
    int x, y;

    inline GridPos() : x(0), y(0) {}
    inline GridPos(int _x, int _y) : x(_x), y(_y) {}
};

struct GridDimension {
    int width;
    int depth;

    inline GridDimension() : width(0), depth(0) {}
    inline GridDimension(int _w, int _d) : width(_w), depth(_d) {}
};

struct HitBox {
    GridPos origin;
    GridDimension dim;

    inline HitBox() :
        origin(0, 0), dim(1, 1) {}
    inline HitBox(GridPos _pos, GridDimension _dim) :
        origin(_pos), dim(_dim) {}
    inline HitBox(int _x, int _y, int _w, int _d) :
        origin(_x, _y), dim(_w, _d) {}
};

class RGBColor {
    public:
    int r;
    int g;
    int b;

    inline RGBColor() :
        r(255), g(255), b(255) {}
    inline RGBColor(int _r, int _g, int _b) :
        r(_r), g(_g), b(_b) {
        // eventually make the asserts debug only so they don't slow down the program
        assert(0 <= r && r <= 255);
        assert(0 <= g && g <= 255);
        assert(0 <= b && b <= 255);

    }

    unsigned ConvertToSDL(SDL_Surface* surface);
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
    int ID;

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
    MapEntity(HitBox _hb, RGBColor _c, Map* _map, bool _hasCol = true);
    inline MapEntity() : valid(false) {}

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
};

class Wall : public MapEntity {
    private:
    bool isVert;
    const static int thickness = 2;

    public:
    // default color 112,112,112 is gray
    Wall(GridPos _pos, int _length, bool _isV, RGBColor _c, Map* _map);
};

class Player : public MapEntity {
    public:
    
    int health = 100;
    int runEnergy = 100;
    Uint32 born = 0;
    Uint32 lastUpdate = 0;


    inline Player(HitBox _hb, RGBColor _c, Map* _map) :
        MapEntity(_hb, _c, _map) {}

    // pretty sure we can remove this but not checking rn
    friend struct Display;
};

struct Map {
    int numberOfEntities = 0;
    Arr2d<MapEntity> grid;
    Arr2d<MapEntity> background;

    // npcs

    // walls

    Map ();

    // Drawing each pixel based on each entry of grid for the map
    // will be slow compared to if we can do some SDL_FillRects, but
    // idk how to we'd do that
    void CreateBackground();

    // Clears the map at area covered by player
    void Clear(Player player);

    // These add functions add data that get's drawn differently
    // than drawing via display.Update(wall) or for player.
    // For example, if we add a wall to the map, it will store wall.color.r
    // in the map, and when we display.Update(map) it will use wall.color.r to color that pixel
    // But if we do display.Update(wall) it will use the full rgb color correctly.

    // I think to fix this we should start storing entire objects in the grid,
    // and to do that, we will need to make them the same type. So that means we gotta
    // add inheritence aka a parent class for wall and player called like MapEntry or something
    // Then we have a grid full of MapEntry objects, some of which are players, some of which are walls.

    // Adds a player to the map
    void Add(Player player);

    void Add(Wall wall);

    void Add(MapEntity entity);
    void Clear(MapEntity entity);

    bool CheckForCollision(const HitBox& movingPiece, int ID);
};

struct Display {
    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;
    Map* map = nullptr;

    Display(SDL_Window* _w, Map* map);

    ~Display();

    void Update(bool updateScreen = true);

    void Erase(Player player, bool updateScreen = true);

    void Update(Player player, bool updateScreen = true);

    void Update(Wall wall, bool updateScreen = true);

    void Update(MapEntity entity, bool updateScreen = true);
};
