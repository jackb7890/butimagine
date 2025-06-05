#include "TextureManager.hpp"

//Loading textures (images) takes like 3 different functions everytime so we're just making 1 function that does it all.
SDL_Texture* TextureManager::LoadTexture(const char* filePath, SDL_Renderer* ren) {
	SDL_Surface* surface = IMG_Load(filePath);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	return tex;
}