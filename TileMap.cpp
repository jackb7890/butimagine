#include "TileMap.hpp"
#include "TextureManager.hpp"
#include "World.hpp"

Tile::Tile(HitBox _hb, SDL_Texture* _tex, int _col) {
	this->SetCollisionType(_col);
	this->SetTexture(_tex);
	this->hitbox = _hb;
}

Tile::~Tile() {
	SDL_DestroyTexture(texture);
}

void Tile::SetTexture(const char* filepath, SDL_Renderer* ren) {
	texture = TextureManager::LoadTexture(filepath, ren);
}

void Tile::SetTexture(SDL_Texture* tex) {
	texture = tex;
}




TileMap::TileMap() {
	
}

//int TileMap::ReadMapFile() {

//}

void TileMap::GenerateTileMap(int arr[TILESWIDTH][TILESHEIGHT]) {

}

void TileMap::DisplayMap() {

}