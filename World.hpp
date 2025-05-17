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

class MapObject {
    // base class for things that go on the map
    // for collosion detection sake, assume everything is rectangular for now

    //A- I was originally going to remove "Map" variable from MapObject.
    //A- But it turns out we do need it because that's how we check for collision.
    //A- "MapEntity" from before has been removed. Instead we're using a flag for immobile.
    //A- Objects are assumed immobile. Immobile objects can NEVER move.

    public:
    bool valid;
    HitBox hitbox;
    RGBColor color;
    bool hasCollision;
    Map* map;
    int ID = 0;
    bool immobile;

    //A- It would be silly but is there a way to only declare these variables if immobile is false?
    //A- idk if that could or should be done in the hpp.
    //A- Otherwise we'd have to check if an object is immobile for every movement function.
    //A- It'd be easier for these to just not exist if an object is immobile.
    double X_velocity, Y_velocity;
    bool hasMovedOffScreen = false;
    GridPos oldPos;

    inline MapObject() : valid(false) {}

    inline MapObject(HitBox _hb, RGBColor _c, bool _hasCol = true, bool _im = true) :
        hitbox(_hb), color(_c), hasCollision(_hasCol), immobile(_im) {
    }

    MapObject(HitBox _hb, RGBColor _c, Map* _m, bool _hasCol = true, bool _im = true);
    MapObject(HitBox _hb, SDL_Texture* tex, bool _hasCol = true, bool _im = true);
    MapObject(HitBox _hb, SDL_Texture* tex, Map* _m, bool _hasCol = true, bool _im = true);


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

class Wall : public MapObject {
    private:
    bool isVert;
    const static int thickness = 2;

    public:
    // default color 112,112,112 is gray
    Wall(GridPos _pos, int _length, bool _isV, RGBColor _c);
};

class Player : public MapObject {

public:
    int health = 100;
    int runEnergy = 100;
    Uint32 born = 0;
    Uint32 lastUpdate = 0;

    inline Player(HitBox _hb, RGBColor _c, Map* _m) :
        MapObject(_hb, _c, _m, true, false) {
    }
};

struct Map {
    int numberOfEntities = 0;
    Arr2d<MapObject> grid;
    Arr2d<MapObject> background;

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

    void Add(MapObject entity);
    void Clear(MapObject entity);

    bool CheckForCollision(const HitBox& movingPiece, int ID);
};

struct Display {
    //A- Tutorial said it was a good idea to have a single static renderer
    SDL_Renderer* renderer;
    SDL_Window* window = nullptr;
    Map* map = nullptr;

    Display(SDL_Window* _w, SDL_Renderer* _r, Map* map);

    ~Display();

    void Update();

    void Erase(Player player, bool renderChange = true);

    void Update(Player player);

    void Update(Wall wall);

    void Update(MapObject object);

    void Render();
};

class Tile : public MapObject {
public:
    Uint8 collisionType;
    SDL_Texture* texture;

    Tile(HitBox _hb, SDL_Texture* _tex, int _col);
    ~Tile();

    inline int GetCollisionType() {
        return collisionType;
    }

    inline GridPos GetTilePosition() {
        int x = hitbox.origin.x % TILE_RES;
        int y = hitbox.origin.y % TILE_RES;
        return GridPos(x, y);
    }

    inline void SetCollisionType(int col = 15) {
        collisionType = col % 16;
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

    //get tile object is on
    //overload to get player, walls, etc.
    void GetObjectTile();

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


//notes

//We currently render GameObjects, which are SDL_Rects with some properties
//There's currently no way to generate a game object from a texture
//We need to generate a tile as a game object
//TileMap should be an array of Tile game objects
//Only way to render something to the screen is Display.update
//Display.update heavily relies on the background game object
//Background is an array of 1x1 game objects, aka pixels
//Every pixel in background is a gameobject
//We need to change the way things are rendered so it's at least background > objects > gui
//Currently it's pixel by pixel, if there's no gameobject at that coordinate it uses the background object at that coord
//Display is the renderer, window, and map
//Map is a 2d grid(x,y coords) and a background

//So what we need to do
//Have each tile be a game object, that lets us do collision & texture easily
//Have Tilemap create & track tiles based on a map file (or algoritm)
//Change our rendering to allow for a tilemap background
//Adjust for rendering bugs
//simplify display.update(?)