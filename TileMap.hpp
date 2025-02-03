#pragma once
#include <iostream>
#include <fstream>

#include "SDL.h"
#include "SDL_image.h"
#include "World.hpp"

//A- Tile resolution in pixels
#define TILE_RES 32;

const int TILESWIDTH = MAP_WIDTH / TILE_RES;
const int TILESHEIGHT = MAP_HEIGHT / TILE_RES;
//A- I don't like having another renderer here but I can't fix that rn

class Tile : public MapObject{
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

	void SetTexture(const char* filepath, SDL_Renderer* ren);
	void SetTexture(SDL_Texture* tex);

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