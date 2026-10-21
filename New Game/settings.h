#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <array>
#include <map>
#include <utility>
#include <string>
#include <random>

#define WIDTH 600 // 600px
#define HIGH 480 // 480px
#define GRAVITY 3.5 // gravity constant
#define BUFFER 64 // 64 bytes of words can be store

constexpr int TILE_SIZE = 32;
constexpr int WORLD_TILES_X = WIDTH *50;
constexpr int WORLD_TILES_Y = HIGH * 50;
constexpr int WORLD_PIXEL_WIDTH = WORLD_TILES_X * TILE_SIZE;
constexpr int WORLD_PIXEL_HEIGHT = WORLD_TILES_Y * TILE_SIZE;

// Tiles type
enum class TilesType {
	Water,
	Grass,
	Dirt,
	Air,
};

//levels
enum LEVELS {
	LEVEL_1 = 1,
	LEVEL_2,
	LEVEL_3,
	LEVEL_4,
};
// game mode
enum GameMode {
	START,
	MAIN,
	EXIT,
	DIE,
	NEXT_LEVEL
};
enum class Animation {
	IDLE,
	RUN,
	JUMP,
	FACE_UPWARD,
	FACE_DOWNWARD,
};

// structure 
struct Camera {
	int x, y;
	int width = WIDTH;
	int heigh = HIGH;
};
typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	// Entry text surface and texture
	SDL_Surface* EntryTextSurface;
	SDL_Texture* EntryTextTexture;
	// Exit text surfacce and texture
	SDL_Surface* ExitTextSurface;
	SDL_Texture* ExitTextTexture;

	SDL_Event event;
	TTF_Font* font;

} game_t;
// Tiles info
struct Tiles {
	TilesType type;
	bool isWalkable;
};
struct ItemPickup {
	SDL_Rect rect;
	std::string name;
    SDL_Texture* texture;
};
struct InventorySlot {
	std::string name;
	int count = 0;
  SDL_Texture* texture = nullptr;
};
constexpr int INVENTORY_COLUMNS = 5;
constexpr int INVENTORY_ROWS = 4;
constexpr int INVENTORY_SLOT_COUNT = INVENTORY_COLUMNS * INVENTORY_ROWS;
class Inventory {
private:
   std::array<InventorySlot, INVENTORY_SLOT_COUNT> slots{};
public:
    bool AddItem(const std::string& name, SDL_Texture* texture, int amount = 1) {
		if (name.empty() || amount <= 0) return false;
		for (auto& slot : slots) {
			if (slot.name == name) {
				slot.count += amount;
				return true;
			}
		}
		for (auto& slot : slots) {
			if (slot.count == 0) {
				slot.name = name;
				slot.count = amount;
                slot.texture = texture;
				return true;
			}
		}
		return false;
	}
  const InventorySlot& GetSlot(int index) const { return slots.at(index); }
};
// player class
class Player {
protected:
	SDL_Rect rect;
	float speedX, speedY;
	bool isAlive;
	Animation animationMode;
	float animationTimer = 0.0f;
	uint8_t frame = 0;
	uint8_t jumpcount = 0;
	SDL_RendererFlip flip = SDL_FLIP_NONE;
public:
	//construcutor
	Player(int x, int y) {
		rect.x = x;
		rect.y = y;
		rect.w = 42;
		rect.h = 42;
		speedX = 12;
		speedY = 12;
		animationMode = Animation::IDLE; // normal player standing
	}
	void Update(float deltaTime);
	void Draw(SDL_Renderer* renderer, SDL_Texture* RunTexture, SDL_Texture* IdleTexture, SDL_Texture* JumpTexture, Camera camera);
	void Move(const uint8_t* key);

	SDL_Rect GetRect() { return rect; }
	SDL_RendererFlip GetFlip() const { return flip; }
};

class Bullet : public Player {
protected:
	SDL_Rect rect;
	float speedX, speedY;
	bool isAlive;
	float animationTimer = 0.0f;
	short int frame = 0;
public:
	Bullet(int x, int y, SDL_RendererFlip direction = SDL_FLIP_NONE);
	void Draw(SDL_Renderer* renderer, SDL_Texture* texture, Camera camera);
	void Update(float deltaTime);
	bool IsAlive() const;
	void Kill();
	SDL_Rect GetRect() const;
};

class Enemy : public Bullet {
protected:
	SDL_RendererFlip flip;
public:
	void Update(float deltaTime);
	void Draw(SDL_Renderer* renderer, SDL_Texture* texture, Camera camera);
	//construction the class
	Enemy(int x, int y) : Bullet(x, y) {
		rect.x = x;
		rect.y = y;
		rect.w = 32;
		rect.h = 24;
		speedX = 12;
		speedY = 0;
	}
};

// dynamic Text for score and level :D
class DynamicText {
public:
	DynamicText(TTF_Font* font, SDL_Color color) : m_font(font), m_color(color), m_texture(nullptr), m_rect{ 0,0,0,0 } {}
	~DynamicText() {
		CleanUp();
	}
	void UpdateText(SDL_Renderer* renderer, const std::string& str) {
		if (str == m_cached_text && m_texture != nullptr) {
			return;
		}
		CleanUp();
		m_cached_text = str;
		if (str.empty()) return;
		SDL_Surface* surface = TTF_RenderUTF8_Blended(m_font, m_cached_text.c_str(), m_color);
		if (!surface) return;
		m_texture = SDL_CreateTextureFromSurface(renderer, surface);
		m_rect.w = surface->w;
		m_rect.h = surface->h;

		SDL_FreeSurface(surface);
	}
	void Render(SDL_Renderer* renderer, uint8_t x, uint8_t y) {
		if (!m_texture) return;
		m_rect.x = x;
		m_rect.y = y;
		SDL_RenderCopy(renderer, m_texture, nullptr, &m_rect);
	}
private:
	void CleanUp() {
		if (m_texture) {
			SDL_DestroyTexture(m_texture);
			m_texture = nullptr;
		}
	}
	TTF_Font* m_font;
	SDL_Color m_color;
	SDL_Texture* m_texture;
	SDL_Rect m_rect;
	std::string m_cached_text;
};
// Tile map class

class TileMap {
private:
	std::map<std::pair<int, int>, Tiles> chunks;
	uint32_t seed = 0;
public:

	Tiles generateMap(int x, int y);
	void generateRegion(int width, int heigh, uint32_t seed);
	void render(SDL_Renderer* renderer, SDL_Texture* Water, SDL_Texture* Dirt, SDL_Texture* Grass, Camera camera);
	void CleanUp(SDL_Texture* Water, SDL_Texture* Dirt, SDL_Texture* Grass);
};
//function declearation
void PrintMemoryUsage();
bool Collision(SDL_Rect rect_1, SDL_Rect rect_2);
