#include "TextureManager.hpp"
#include <iostream>

//Loading textures (images) takes like 3 different functions everytime so we're just making 1 function that does it all.
SDL_Texture* TextureManager::LoadTexture(const char* filePath, SDL_Renderer* ren) {
	SDL_Surface* surface = IMG_Load(filePath);
	if (surface == NULL) {
		std::cout << "Error loading image" << std::endl;
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	return tex;
}