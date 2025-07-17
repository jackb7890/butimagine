#pragma once

#define SDL_MAIN_HANDLED 1

#include <iostream>
#include <fstream>
#include <list>

#include "SDL.h"
#include "SDL_image.h"

#include "TextureManager.hpp"
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

    public:
    HitBox hitbox;
    RGBColor color;
    SDL_Texture* texture;
    bool hasCollision;
    Map* map = nullptr;
    int ID = NULL;

    double X_velocity, Y_velocity;
    bool hasMovedOffScreen = false;
    GridPos oldPos;

    inline MapEntity(HitBox _hb, RGBColor _c, bool _hasCol = true) :
        hitbox(_hb), color(_c), hasCollision(_hasCol) {
    }
    inline MapEntity(HitBox _hb, RGBColor _c, Map* _m, bool _hasCol) :
        hitbox(_hb), color(_c), map(_m), hasCollision(_hasCol) {
    }

    MapEntity(HitBox _hb, SDL_Texture* tex, bool _hasCol = true);
    MapEntity(HitBox _hb, SDL_Texture* tex, Map* _m, bool _hasCol = true);

    bool Valid();

    inline GridPos GetCurrentPos() const {
        return hitbox.origin;
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

    inline GridPos GetOldPos() const {
        return oldPos;
    }

    void Move(int xD, int yD);
};

class Wall : public MapEntity {
    private:
    bool isVert;
    const static int thickness = 2;

    public:
    // default color 112,112,112 is gray
    Wall(GridPos _pos, int _length, bool _isV, RGBColor _c);
};

class Player : public MapEntity {

public:
    int health = 100;
    int runEnergy = 100;
    Uint32 born = 0;
    Uint32 lastUpdate = 0;

    inline Player(HitBox _hb, RGBColor _c, Map* _m) :
        MapEntity(_hb, _c, _m, true) {
    }
};

//A- Map is a collection of game entities.
struct Map {
    std::list<MapEntity*> allEntities;
    Arr2d<MapEntity*> grid;
    //Arr2d<MapEntity> background;

    Map();

    inline int GetNumberOfEntities() {
        return allEntities.size();
    }

    //A- Permanently deletes all MapEntities.
    inline void DeleteAllEntities() {
        allEntities.clear();
    }

    //Removes most recently added MapEntity.
    //Pop does not return the last element, just deletes it.
    inline void PopEntity() {
        allEntities.pop_back();
    }

    void AddToGrid(MapEntity& entity);

    //Adds an entity to the map.
    //This will also update the map variable stored within the entity, making the entity valid.
    void AddEntity(MapEntity* entity);
    void AddEntity(Player* player);
    void AddEntity(Wall* wall);

    bool CheckForCollision(const HitBox& movingPiece, int ID);

};

struct Display {
    SDL_Renderer* renderer;
    SDL_Window* window = nullptr;
    Map* map = nullptr;

    Display(SDL_Window* _w, SDL_Renderer* _r, Map* map);

    ~Display();

    void Update();

    void Render();
};
