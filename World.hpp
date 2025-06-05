#pragma once

#define SDL_MAIN_HANDLED 1

#include <iostream>
#include <fstream>
#include "SDL.h"
#include "SDL_image.h"

#include "TextureManager.hpp"
#include "util.hpp"

#define MAP_WIDTH 1280
#define MAP_HEIGHT 720

//Tile resolution in pixels
#define TILE_RES 32;

const int TILESWIDTH = MAP_WIDTH / TILE_RES;
const int TILESHEIGHT = MAP_HEIGHT / TILE_RES;

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

    bool immobile;
    double X_velocity, Y_velocity;
    bool hasMovedOffScreen = false;
    GridPos oldPos;

    inline MapEntity(HitBox _hb, RGBColor _c, bool _hasCol = true, bool _im = true) :
        hitbox(_hb), color(_c), hasCollision(_hasCol), immobile(_im) {
    }
    inline MapEntity(HitBox _hb, RGBColor _c, Map* _m, bool _hasCol = true, bool _im = true) :
        hitbox(_hb), color(_c), map(_m), hasCollision(_hasCol), immobile(_im) {
    }
    inline MapEntity(HitBox _hb, SDL_Texture* _tex, bool _hasCol = true, bool _im = true) :
        hitbox(_hb), texture(_tex), hasCollision(_hasCol), immobile(_im) {
    }
    inline MapEntity(HitBox _hb, SDL_Texture* _tex, Map* _m, bool _hasCol, bool _im) :
        hitbox(_hb), texture(_tex), map(_m), hasCollision(_hasCol), immobile(_im) {
    }

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

    void ForceMove(int xD, int yD);
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
        MapEntity(_hb, _c, _m, true, false) {
    }
};

//A- Map is a collection of game Entities.
struct Map {
    std::vector<MapEntity*> allEntities;
    //Arr2d<MapEntity> background;
    Arr2d<MapEntity*> grid;

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

    //Returns an Entity reference from an ID
    inline MapEntity& GetEntity(int ID) {
        MapEntity* entity = allEntities.at(ID);
        //The ".at()" function automatically performs error checking, like if we passed an ID that's out of bounds or negative.
        return *entity;
    }

    void AddToGrid(MapEntity& entity);

    //Adds an entity to the map.
    //This will also update the map variable stored within the entity, making the entity valid.
    void AddEntity(MapEntity* entity);
    void AddEntity(Player* player);
    void AddEntity(Wall* wall);

    bool CheckForCollision(const HitBox& movingPiece, int ID);

    //Unused
    void CreateBackground();

};

struct Display {
    SDL_Renderer* renderer;
    SDL_Window* window = nullptr;
    Map* map = nullptr;

    Display(SDL_Window* _w, SDL_Renderer* _r, Map* map);

    ~Display();

    //A- ChangeMap swaps map in display with a different one.
    void ChangeMap(Map* /* pointer? */);

    //A- Clear wipes the screen completly. Use before changing maps.
    void Clear();

    void Update();

    void Render();

    //Unused
    void Erase(Player player, bool renderChange = true);
    void Update(Player player);
    void Update(Wall wall);
    void Update(MapEntity entity);
};

//Tiles are only using 2 variables in MapEntity.
//I'll keep it as a child for rendering but in the future it should be it's own class.
class Tile : public MapEntity {
public:
    Uint8 collisionType;

    Tile(HitBox _hb, SDL_Texture* _tex, int _col);
    ~Tile();

    inline int GetCollisionType() {
        return collisionType;
    }

    inline void SetCollisionType(int col = 0) {
        collisionType = col;
        assert(collisionType < 16);
    }

    inline void SetTexture(const char* filepath, SDL_Renderer* ren) {
        texture = TextureManager::LoadTexture(filepath, ren);
    }

    inline void SetTexture(SDL_Texture* tex) {
        texture = tex;
    }
};

//Should read, load, & render the tile based map
//Helper functions to check tiles, collision(?), coords
class TileMap {
public:
    TileMap();
    ~TileMap();

    //read map file
    //Should return the array from the map file(?)
    int ReadMapFile();

    //generate tilemap based on map file
    //Probably have it use ReadMapFile, should have everything ready for rendering  
    void GenerateTileMap(int arr[TILESWIDTH][TILESHEIGHT]);

    //render textures to screen
    //used in place of Display struct in World.h
    void DisplayMap();

    //Debug display what tile player is on
    bool DebugHighlightCurrentTile();

    //get tile entity is on
    //overload to get player, walls, etc.
    void GetEntityTile();

    //collision logic
    //There are 16 possible collision combinations for a tile (4^2)
    //Somehow I think binary is the easiest way to assign & remember collision (internally)
    //Having 1 variable determine collision makes assigning easy
    //Having it be binary based makes calculations easy
    // 0 0 0 0, in order of Left, Top, Bottom, Right
    /*	0 = 0000 = No collision
    *	1 = 1000 = Left
    *	2 = 0100 = Top
    *	3 = 1100 = Left, Top
    *	4 = 0010 = Bottom
    *	5 = 1010 = Left, Bottom
    *	6 =	0110 = Top, Bottom
    *	7 = 1110 = Left, Top, Bottom
    *	8 = 0001 = Right
    *	9 = 1001 = Left, Right
    *	10= 0101 = Top, Right
    *	11= 1101 = Left, Top, Right
    *	12= 0011 = Bottom, Right
    *	13= 1011 = Left, Bottom , Right
    *	14= 0111 = Top, Bottom, Right
    *	15= 1111 = All
    */
    //For calcualting;
    //	Left	= Odd number
    //	Top		= %4 is >= 2
    //	Bottom	= %8 is >= 4
    //	Right	= >= 8

    //As a final example to this shear lack of any actual code
    //If we look at a collision value of 11
    // 1. 11 is odd, so we know there's collision on the left
    // 2. 11 %4 = 3, so we know there's collision on the top
    // 3. 11 %8 = 3, so we know there's no bottom collision
    // 4. 11 is >= 8, so we know there's collision on the right.

private:
    SDL_Rect* src, dest;
    SDL_Texture* grass;
    int map[TILESWIDTH][TILESHEIGHT];
};
