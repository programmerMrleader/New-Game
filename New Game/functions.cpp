#include <iostream>
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
#include "settings.h"
#include <SDL_image.h>
#include <ctime>
#include <cstdlib>
#include "FastNoiseLite.h"
#include <algorithm>

void PrintMemoryUsage() {
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		std::cout
			<< "\rCurrent Memory Usage : " << ((pmc.WorkingSetSize) / (1024.0 * 1024.0)) << " MiB | "
			<< "Peak Memory Usage : " << ((pmc.PeakWorkingSetSize) / (1024.0 * 1024.0)) << " MiB  "
			<< std::flush;
	}
	else {
		std::cerr << "Failed to retrieve memory info.\n";
	}
}
bool Collision(SDL_Rect a, SDL_Rect b) {

	// Side boundaries of Rect A
	int leftA = a.x;
	int rightA = a.x + a.w;
	int topA = a.y;
	int bottomA = a.y + a.h;

	// Side boundaries of Rect B
	int leftB = b.x;
	int rightB = b.x + b.w;
	int topB = b.y;
	int bottomB = b.y + b.h;

	// If any side from A is outside of B, no collision is possible
	if (bottomA <= topB)   return false;
	if (topA >= bottomB)   return false;
	if (rightA <= leftB)   return false;
	if (leftA >= rightB)   return false;

	// Otherwise, they overlap!
	return true;
}
Bullet::Bullet(int x, int y, SDL_RendererFlip direction) : Player(x, y) {
	rect.w = 10;
	rect.h = 10;

	rect.x = x - rect.w / 2;
	rect.y = y;

	speedX = direction == SDL_FLIP_HORIZONTAL ? -12.0f : 12.0f;
	speedY = 0;
	flip = direction;

	isAlive = true;

	frame = 0;
	animationTimer = 0.0f;
}
SDL_Rect Bullet::GetRect() const { return rect; }
void Bullet::Update(float deltaTime) {
	rect.x += speedX;
	rect.y += speedY;
	animationTimer += deltaTime;
	if (animationTimer >= 0.3f) {
		animationTimer = 0.0f;
		frame++;
	}
	if (frame >= 7) frame = 0;
	if (
		rect.y + rect.h < 0 ||
		rect.y > WORLD_PIXEL_HEIGHT ||
		rect.x + rect.w < 0 ||
		rect.x > WORLD_PIXEL_WIDTH
		) {
		isAlive = false;
	}
}
bool Bullet::IsAlive() const { return isAlive; }
void Bullet::Kill() { isAlive = false; }

void Bullet::Draw(SDL_Renderer* renderer, SDL_Texture* texture, Camera camera) {
	short int frameWidth = 32;
	short int frameHigh = 24; // 34x24 pixels
	SDL_Rect srcRect = {
		frame * frameWidth,
		0,
		frameWidth,
		frameHigh
	};
	SDL_Rect destRect = {
		rect.x - camera.x,
		rect.y - camera.y,
		32,
		24
	};
	SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, flip);
}
//initinalize the enemy 

void Enemy::Update(float deltaTime) {

	rect.x += speedX;

	if (rect.x + rect.w > WORLD_PIXEL_WIDTH || rect.x < 0) {
		speedX *= -1;
		rect.x = std::clamp(rect.x, 0, WORLD_PIXEL_WIDTH - rect.w);

	}
	if (speedX < 0) flip = SDL_FLIP_HORIZONTAL;
	if (speedX > 0) flip = SDL_FLIP_NONE;

	if (rect.y + rect.h < WORLD_PIXEL_HEIGHT) {
		rect.y += GRAVITY;
		if (rect.y + rect.h > WORLD_PIXEL_HEIGHT) {
			rect.y = WORLD_PIXEL_HEIGHT - rect.h;
		}
	}
	animationTimer += deltaTime;
	if (animationTimer >= 3.0f) {
		animationTimer = 0.0f;
		frame++;
	}
	if (frame >= 6) frame = 0;
}
void Enemy::Draw(SDL_Renderer* renderer, SDL_Texture* texture, Camera camera) {
	short int frameWidth = 48;
	short int frameHigh = 48;
	SDL_Rect srcRect = {
		frame * frameWidth,
		0,
		frameWidth,
		frameHigh
	};
	SDL_Rect destRect = {
		rect.x - camera.x,
		rect.y - camera.y,
		48,
		48,
	};
	SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, flip);
}

void Player::Move(const uint8_t* key) {

	bool isMoving = false;
	if (key[SDL_SCANCODE_D] && rect.x + rect.w < WORLD_PIXEL_WIDTH) {
		rect.x = std::min(rect.x + static_cast<int>(speedX), WORLD_PIXEL_WIDTH - rect.w);
		animationMode = Animation::RUN;
		flip = SDL_FLIP_NONE;
		std::cout << "Not Flipping \n";
		isMoving = true;
	}
	if (key[SDL_SCANCODE_A] && rect.x > 0) {
		rect.x = std::max(rect.x - static_cast<int>(speedX), 0);
		animationMode = Animation::RUN;
		flip = SDL_FLIP_HORIZONTAL;
		std::cout << "Flipping \n";
		isMoving = true;
	}
	if (key[SDL_SCANCODE_W] && rect.y > 0) {
		rect.y = std::max(rect.y - static_cast<int> (speedY), 0);
		animationMode = Animation::FACE_UPWARD;
		flip = SDL_FLIP_VERTICAL;
		isMoving = true;
	}
	if (key[SDL_SCANCODE_S] && rect.y + rect.h < WORLD_PIXEL_HEIGHT) {
		rect.y = std::min(rect.y + static_cast<int>(speedY), WORLD_PIXEL_HEIGHT - rect.h);
		animationMode = Animation::FACE_DOWNWARD;
		isMoving = true;
	}
	// gravity
	/*
	bool isJumping = false;
	if (key[SDL_SCANCODE_SPACE] && jumpcount <= 10 && rect.y > 0) {
		rect.y -= speedY;
		jumpcount += 1;
		animationMode = Animation::JUMP;
		isJumping = true;
		// used to debug std::cout << jumpcount << "\n";
	}
	if (rect.y + rect.h < WORLD_PIXEL_HEIGHT) {
		rect.y += GRAVITY;
		if (rect.y + rect.h > WORLD_PIXEL_HEIGHT) {
			rect.y = WORLD_PIXEL_HEIGHT - rect.h;
		}
	}
	if (rect.y + rect.h >= WORLD_PIXEL_HEIGHT) jumpcount = 0;
	
	if (isJumping || rect.y + rect.h < WORLD_PIXEL_HEIGHT) {
		newMode = Animation::JUMP;
	}
	*/
	Animation newMode = Animation::IDLE;
	
	if (isMoving) {
		newMode = Animation::RUN;
	}

	// Reset frame counter if state changed to prevent animation glitching
	if (newMode != animationMode) {
		animationMode = newMode;
		frame = 0;
		animationTimer = 0.0f;
	}
}

void Player::Update(float deltaTime) {
	animationTimer += deltaTime;
	if (animationTimer >= 3.0f) {
		animationTimer = 0.0f;
		frame++;
	}
	if (frame >= 8) frame = 0;
}
void Player::Draw(SDL_Renderer* renderer, SDL_Texture* RunTexture, SDL_Texture* IdleTexture, SDL_Texture* JumpTexture, Camera camera) {
	uint8_t frameWidth = 48;
	uint8_t frameHigh = 48;
	SDL_Rect srcRect = {
		frame * frameWidth,
		0,
		frameWidth,
		frameHigh,
	};
	SDL_Rect destRect = {
		rect.x - camera.x,
		rect.y - camera.y,
		frameWidth,
		frameHigh
	};
	switch (animationMode) {
	case Animation::RUN:
		SDL_RenderCopyEx(renderer, RunTexture, &srcRect, &destRect, 0.0, NULL, flip);
		break;
	case Animation::JUMP:
		SDL_RenderCopyEx(renderer, RunTexture, &srcRect, &destRect, 0.0, NULL, flip);
		break;
	default:
		SDL_RenderCopyEx(renderer, IdleTexture, &srcRect, &destRect, 0.0, NULL, flip);
		break;
	}
}
Tiles TileMap::generateMap(int x, int y) {
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetSeed(static_cast<int>(seed));
	noise.SetFrequency(0.05f);
	TilesType type;
	float elevation = noise.GetNoise(static_cast<float>(x), static_cast<float>(y));
	if (elevation < -0.15f) type = TilesType::Water;
	else if (elevation < 0.25f) type = TilesType::Grass;
	else type = TilesType::Dirt;
	bool walkable = (type != TilesType::Water);
	return Tiles{ type,walkable };

}
void TileMap::generateRegion(int width, int heigh, uint32_t seed) {
	this->seed = seed;
	chunks.clear();

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigh; y++) {
			chunks[{x, y}] = generateMap(x, y);
		}
	}
}

void TileMap::render(SDL_Renderer* renderer, SDL_Texture* Water, SDL_Texture* Dirt, SDL_Texture* Grass, Camera camera) {
	const uint8_t TILE_SIZE = 32;
	int startX = std::max(camera.x / TILE_SIZE, 0);
	int startY = std::max(camera.y / TILE_SIZE, 0);
	int endX = std::min((camera.x + camera.width + TILE_SIZE - 1) / TILE_SIZE, WORLD_TILES_X);
	int endY = std::min((camera.y + camera.heigh + TILE_SIZE - 1) / TILE_SIZE, WORLD_TILES_Y);
	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {

			// Lookup or generate missing tile on demand when scrolling
			auto it = chunks.find({ x, y });
			if (it == chunks.end()) {
				Tiles newTile = generateMap(x, y);
				it = chunks.insert({ { x, y }, newTile }).first;
			}

			const Tiles& tile = it->second;

			// 3. Convert world coords to screen coords using camera offset
			SDL_Rect destRect = {
				(x * TILE_SIZE) - camera.x,
				(y * TILE_SIZE) - camera.y,
				TILE_SIZE,
				TILE_SIZE
			};

			switch (tile.type) {
			case TilesType::Air:
				break;
			case TilesType::Dirt:
				SDL_RenderCopy(renderer, Dirt, NULL, &destRect);
				break;
			case TilesType::Grass:
				SDL_RenderCopy(renderer, Grass, NULL, &destRect);
				break;
			case TilesType::Water:
				SDL_RenderCopy(renderer, Water, NULL, &destRect);
				break;
			}
		}
	}
}

void TileMap::CleanUp(SDL_Texture* Water, SDL_Texture* Dirt, SDL_Texture* Grass) {
	SDL_DestroyTexture(Water);
	SDL_DestroyTexture(Dirt);
	SDL_DestroyTexture(Grass);
}