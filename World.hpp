#pragma once

#define SDL_MAIN_HANDLED 1
#include "SDL.h"

#include "util.hpp"

#define MAP_WIDTH 640
#define MAP_HEIGHT 480

struct Map;
struct Display;

struct GridPos {
    int x, y;

    inline GridPos() : x(0), y(0) {}
    inline GridPos(int _x, int _y) : x(_x), y(_y) {}
};

struct RGBColor {
    int r;
    int g;
    int b;

    RGBColor(int _r, int _g, int _b) :
        r(_r), g(_g), b(_b) {}
};

struct Wall {
    bool isVert;
    GridPos origin;
    int width = 2;
    int length;
    RGBColor color;

    Map& map;

    // default color 112,112,112 is gray
    Wall(int _x, int _y, int _l, bool _isV, Map& _map) : 
        origin(_x, _y), length(_l), isVert(_isV), 
        color(RGBColor(112, 112, 112)), map(_map) {}

    Wall(int _x, int _y, int _l, RGBColor _c, bool _isV, Map& _map) : 
        origin(_x, _y), length(_l), isVert(_isV), 
        color(_c), map(_map) {}
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
    int playerID = INT_MAX/2; //using this as their color
    
    Map& map;

    Player::Player(int _x, int _y, Map& _map);

    void Player::MoveHoriz(int xD);
    void Player::MoveVert(int yD);

    friend struct Display;
};

struct Map {
    Arr2d<int> grid;

    // npcs

    // walls

    Map ();

    // Drawing each pixel based on each entry of grid for the map
    // will be slow compared to if we can do some SDL_FillRects, but
    // idk how to we'd do that
    void SetStartMap();

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
};

struct Display {
    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;

    Display(SDL_Window* _w);

    ~Display();

    void Update(Map map, bool updateScreen = true);

    void Erase(Player player, bool updateScreen = true);

    void Update(Player player, bool updateScreen = true);

    void Update(Wall wall, bool updateScreen = true);
};
