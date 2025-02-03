#pragma once

#include "SDL.h"
#include "SDL_image.h"

class TextureManager {
public:
	static SDL_Texture* LoadTexture(const char* fileName, SDL_Renderer* ren);

	//A- In the future I want this to read any png & automatically create the tiles based on the image size & specified tile size
	//A- Lets us have all our tiles in one image, add tiles whenever, change the resolution, etc.
	//static SDL_Texture* LoadTileMapTextures(const char* fileName, SDL_Renderer* ren);
};