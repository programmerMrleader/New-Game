#include <iostream>
#include <algorithm>
#include <iostream>
#include <vector>
#include "settings.h"
#include <string>
#include <random>
int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO); // initialize the video
	TTF_Init(); // initalize the font
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
	std::random_device rd;

	game_t game;

	SDL_Color TextWhite = { 255,255,255 };
	Camera camera = { 0,0,WIDTH,HIGH };
	game.window = SDL_CreateWindow("New Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HIGH, 0);
	game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	game.font = TTF_OpenFont("Rostex-Regular.ttf", 18);
	if (!game.font) {
		std::cerr << SDL_GetError();
	}
	game.EntryTextSurface = TTF_RenderText_Solid_Wrapped(game.font, "Hello Welcome From My Game :D \n\n Press Enter to Start :D \n", TextWhite, 0);
	game.ExitTextSurface = TTF_RenderText_Solid_Wrapped(game.font, "Thanks you for playing the game :D \n\n See you Next time :D", TextWhite, 0);

	SDL_Surface* surface = IMG_Load("eye_red.png");
	SDL_Texture* BulletTexture = SDL_CreateTextureFromSurface(game.renderer, surface);
	SDL_Surface* IdleSurface = IMG_Load("Idle.png");
	SDL_Texture* IdleTexture = SDL_CreateTextureFromSurface(game.renderer, IdleSurface);
	SDL_Surface* JumpSurface = IMG_Load("Jump.png");
	SDL_Texture* JumpTexture = SDL_CreateTextureFromSurface(game.renderer, JumpSurface);
	SDL_Surface* RunSurface = IMG_Load("Run.png");
	SDL_Texture* RunTexture = SDL_CreateTextureFromSurface(game.renderer, RunSurface);

	// inventory stats and cell
	SDL_Surface* inventory_stats_surface = IMG_Load("inventory_stats.png");
	SDL_Texture* inventory_stats = SDL_CreateTextureFromSurface(game.renderer, inventory_stats_surface);

	SDL_Surface* inventory_cell_surface = IMG_Load("inventory_cell.png");
	SDL_Texture* inventory_cell = SDL_CreateTextureFromSurface(game.renderer, inventory_cell_surface);
	int inventoryCellWidth = 32;
	int inventoryCellHeight = 32;
	SDL_QueryTexture(inventory_cell, nullptr, nullptr, &inventoryCellWidth, &inventoryCellHeight);
	int inventoryStatsWidth = 273;
	int inventoryStatsHeight = 137;
	SDL_QueryTexture(inventory_stats, nullptr, nullptr, &inventoryStatsWidth, &inventoryStatsHeight);

	SDL_Surface* armor_surface = IMG_Load("items/armor.png");
	SDL_Surface* health_item_surface = IMG_Load("items/health.png");

	SDL_Texture* armor = SDL_CreateTextureFromSurface(game.renderer, armor_surface);
	SDL_Texture* health_item = SDL_CreateTextureFromSurface(game.renderer, health_item_surface);

	SDL_Surface* dirtSurface = IMG_Load("dirt.png");
	SDL_Texture* dirtTexture = SDL_CreateTextureFromSurface(game.renderer, dirtSurface);
	SDL_Surface* waterSurface = IMG_Load("water.png");
	SDL_Texture* waterTexture = SDL_CreateTextureFromSurface(game.renderer, waterSurface);
	SDL_Surface* grassSurface = IMG_Load("grass.png");
	SDL_Texture* grassTexture = SDL_CreateTextureFromSurface(game.renderer, grassSurface);
	SDL_Surface* enemy_surface = IMG_Load("run_enemy.png");

	SDL_Texture* enemy_texture = SDL_CreateTextureFromSurface(game.renderer, enemy_surface);
	SDL_Rect TextRect = { 50, HIGH / 2 , game.EntryTextSurface->w , game.EntryTextSurface->h };
	SDL_Rect health_bar = { WIDTH - 150, 30, 100,50 };
	Player player(100, 100);
	std::vector<Bullet> bullet;
	std::vector<Enemy> enemy;
	std::vector<ItemPickup> pickups = {
        {{220, 100, 24, 24}, "Health", health_item},
		{{500, 100, 24, 24}, "Armor", armor}
	};

	enemy.emplace_back(300, 300);

	game.EntryTextTexture = SDL_CreateTextureFromSurface(game.renderer, game.EntryTextSurface);
	game.ExitTextTexture = SDL_CreateTextureFromSurface(game.renderer, game.ExitTextSurface);

	// freeing the surface
	SDL_FreeSurface(game.EntryTextSurface);
	SDL_FreeSurface(game.ExitTextSurface);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(IdleSurface);
	SDL_FreeSurface(RunSurface);
	SDL_FreeSurface(JumpSurface);
	SDL_FreeSurface(waterSurface);
	SDL_FreeSurface(dirtSurface);
	SDL_FreeSurface(grassSurface);
	SDL_FreeSurface(inventory_cell_surface);
	SDL_FreeSurface(inventory_stats_surface);
	SDL_FreeSurface(armor_surface);
	SDL_FreeSurface(health_item_surface);

	bool run = true;
	char GameMode = START;
	short int health = 100;
	bool inventoryOpen = false;
	bool inventoryStatsOpen = false;
	int selectedInventoryCell = 0;
	const short int LIMIT_FPS = 60;
	const int FRAME_DELAY = 1000 / 60;
	Uint32 frameStart;
	int frameTime;

	DynamicText scoreText(game.font, TextWhite);
	DynamicText inventoryText(game.font, TextWhite);
	DynamicText inventoryStatsText(game.font, TextWhite);
	Inventory inventory;
	TileMap Tile;
	Tile.generateRegion(100, 100, 100);
	while (run) {
		frameStart = SDL_GetTicks();
		while (SDL_PollEvent(&game.event)) {
			if (game.event.type == SDL_QUIT) run = false;
			switch (game.event.type) {
			case SDL_KEYDOWN:
				switch (game.event.key.keysym.sym) {
				case SDLK_ESCAPE: GameMode = EXIT; break;
				case SDLK_RETURN: GameMode = MAIN; break;

				case SDLK_w: {
					if (GameMode != MAIN) break;
					const SDL_Rect playerRect = player.GetRect();
					const SDL_RendererFlip direction = player.GetFlip();
					const int bulletX = direction == SDL_FLIP_HORIZONTAL
						? playerRect.x
						: playerRect.x + playerRect.w;
					bullet.emplace_back(
						bulletX,
						playerRect.y + playerRect.h / 2,
						direction
					);
					break;
				}
				case SDLK_i:
					if (GameMode == MAIN && game.event.key.repeat == 0) {
						inventoryOpen = !inventoryOpen;
						inventoryStatsOpen = false;
					}
					break;
				case SDLK_e:
					if (GameMode == MAIN && inventoryOpen && game.event.key.repeat == 0) {
						inventoryStatsOpen = !inventoryStatsOpen;
					}
					break;
				case SDLK_LEFT:
					if (inventoryOpen && selectedInventoryCell % INVENTORY_COLUMNS > 0) {
						--selectedInventoryCell;
					}
					break;
				case SDLK_RIGHT:
					if (inventoryOpen && selectedInventoryCell % INVENTORY_COLUMNS < INVENTORY_COLUMNS - 1) {
						++selectedInventoryCell;
					}
					break;
				case SDLK_UP:
					if (inventoryOpen && selectedInventoryCell >= INVENTORY_COLUMNS) {
						selectedInventoryCell -= INVENTORY_COLUMNS;
					}
					break;
				case SDLK_DOWN:
					if (inventoryOpen && selectedInventoryCell + INVENTORY_COLUMNS < INVENTORY_SLOT_COUNT) {
						selectedInventoryCell += INVENTORY_COLUMNS;
					}
					break;
				}

			}
		}
		if (GameMode == START) {
			SDL_RenderCopy(game.renderer, game.EntryTextTexture, NULL, &TextRect);
		}
		if (GameMode == MAIN) {
			// clean every type shit (can't clean) :')
			// more lowk good keycode
			const uint8_t* key = SDL_GetKeyboardState(NULL);

			player.Move(key);
			player.Update(3.0f);
			const SDL_Rect playerRect = player.GetRect();
			camera.x = playerRect.x + playerRect.w / 2 - camera.width / 2;
			camera.y = playerRect.y + playerRect.h / 2 - camera.heigh / 2;
			camera.x = std::clamp(camera.x, 0, WORLD_PIXEL_WIDTH - camera.width);
			camera.y = std::clamp(camera.y, 0, WORLD_PIXEL_HEIGHT - camera.heigh);

			const SDL_Rect playerBounds = player.GetRect();
			for (auto it = pickups.begin(); it != pickups.end();) {
				if (Collision(playerBounds, it->rect)) {
                  if (inventory.AddItem(it->name, it->texture)) {
						it = pickups.erase(it);
					}
					else {
						++it;
					}
				}
				else {
					++it;
				}
			}

			for (auto& b : bullet) b.Update(2.0f);
			for (auto& e : enemy) e.Update(0.3f);
			// deleting bullet with no memory overhead
			bullet.erase(std::remove_if(bullet.begin(), bullet.end(), [](const Bullet& b) { return !b.IsAlive(); }), bullet.end());
			enemy.erase(std::remove_if(enemy.begin(), enemy.end(), [](const Enemy& e) {return !e.IsAlive(); }), enemy.end());
			// collision checking
			for (auto& b : bullet) {
				for (const auto& e : enemy) {
					if (b.IsAlive() && e.IsAlive() && Collision(b.GetRect(), e.GetRect())) {
						health -= 1;
						health_bar.w -= 1;
						scoreText.UpdateText(game.renderer, "Score :" + std::to_string(health));
						b.Kill();
						break;
					}
				}
			}
			bullet.erase(std::remove_if(bullet.begin(), bullet.end(), [](const Bullet& b) { return !b.IsAlive(); }), bullet.end());
			if (health <= 0) GameMode = EXIT;

			SDL_RenderClear(game.renderer);
			SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 0);
			Tile.render(game.renderer, waterTexture, dirtTexture, grassTexture, camera);
			for (const auto& pickup : pickups) {
				SDL_Rect screenRect = {
					pickup.rect.x - camera.x,
					pickup.rect.y - camera.y,
					pickup.rect.w,
					pickup.rect.h
				};
              SDL_RenderCopy(game.renderer, pickup.texture, nullptr, &screenRect);
			}
			SDL_SetRenderDrawColor(game.renderer, 0, 0, 255, 0);
			for (auto& b : bullet) {
				if (b.IsAlive()) {
					b.Draw(game.renderer, BulletTexture, camera);
				}
			}
			for (auto& e : enemy) {
				e.Draw(game.renderer, enemy_texture, camera);

			}

			player.Draw(game.renderer, RunTexture, IdleTexture, JumpTexture, camera);
			char buffer[BUFFER];
			sprintf_s(buffer, "Health %d", health);
			scoreText.UpdateText(game.renderer, buffer);
			scoreText.Render(game.renderer, 100, 100);
			inventoryText.UpdateText(game.renderer, inventoryOpen ? "Arrows: select | E: stats | I: close" : "I: Inventory");
			inventoryText.Render(game.renderer, 20, 20);
			if (inventoryOpen) {
				SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 190);
				SDL_Rect overlay = { 0, 0, WIDTH, HIGH };
				SDL_RenderFillRect(game.renderer, &overlay);

				const int gridWidth = inventoryCellWidth * INVENTORY_COLUMNS;
				const int gridHeight = inventoryCellHeight * INVENTORY_ROWS;
				const int gridX = (WIDTH - (gridWidth + 16 + inventoryStatsWidth)) / 2;
				const int gridY = (HIGH - gridHeight) / 2;
				for (int row = 0; row < INVENTORY_ROWS; ++row) {
					for (int column = 0; column < INVENTORY_COLUMNS; ++column) {
						SDL_Rect cellRect = {
							gridX + column * inventoryCellWidth,
							gridY + row * inventoryCellHeight,
							inventoryCellWidth,
							inventoryCellHeight
						};
						SDL_RenderCopy(game.renderer, inventory_cell, nullptr, &cellRect);
                       const InventorySlot& slot = inventory.GetSlot(row * INVENTORY_COLUMNS + column);
						if (slot.count > 0 && slot.texture) {
							SDL_Rect itemRect = {
								cellRect.x + 4,
								cellRect.y + 4,
								cellRect.w - 8,
								cellRect.h - 8
							};
							SDL_RenderCopy(game.renderer, slot.texture, nullptr, &itemRect);
						}
					}
				}
				SDL_Rect selectedCellRect = {
					gridX + (selectedInventoryCell % INVENTORY_COLUMNS) * inventoryCellWidth,
					gridY + (selectedInventoryCell / INVENTORY_COLUMNS) * inventoryCellHeight,
					inventoryCellWidth,
					inventoryCellHeight
				};
				SDL_SetRenderDrawColor(game.renderer, 255, 220, 40, 255);
				SDL_RenderDrawRect(game.renderer, &selectedCellRect);

				if (inventoryStatsOpen) {
					SDL_Rect statsRect = {
						gridX + gridWidth + 16,
						gridY,
						inventoryStatsWidth,
						inventoryStatsHeight
					};
					SDL_RenderCopy(game.renderer, inventory_stats, nullptr, &statsRect);
					const InventorySlot& selectedSlot = inventory.GetSlot(selectedInventoryCell);
					const std::string stats = selectedSlot.count > 0
						? selectedSlot.name + " x" + std::to_string(selectedSlot.count)
						: "Empty slot";
					inventoryStatsText.UpdateText(game.renderer, stats);
					inventoryStatsText.Render(game.renderer, static_cast<uint8_t>(statsRect.x + 12), static_cast<uint8_t>(statsRect.y + 16));
				}
			}
			SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255);
			SDL_RenderFillRect(game.renderer, &health_bar);

		}
		if (GameMode == EXIT) {
			// clearing the background

			SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 0);
			SDL_RenderClear(game.renderer);

			SDL_RenderCopy(game.renderer, game.ExitTextTexture, NULL, &TextRect);
			SDL_RenderPresent(game.renderer);

			SDL_Delay(5000);
			run = false;
		}

		SDL_RenderPresent(game.renderer);
		PrintMemoryUsage();
		// capping the frame at 60 fps for further processing
		frameTime = SDL_GetTicks() - frameStart;
		if (FRAME_DELAY > frameTime) {
			SDL_Delay(FRAME_DELAY - frameTime);
		}
	}
	SDL_DestroyTexture(game.EntryTextTexture);
	SDL_DestroyTexture(BulletTexture);
	SDL_DestroyTexture(enemy_texture);
	SDL_DestroyTexture(inventory_cell);
	SDL_DestroyTexture(inventory_stats);
	SDL_DestroyTexture(armor);
	SDL_DestroyTexture(health_item);

	SDL_DestroyTexture(IdleTexture);
	SDL_DestroyTexture(RunTexture);
	SDL_DestroyTexture(JumpTexture);

	SDL_DestroyRenderer(game.renderer);
	TTF_CloseFont(game.font);
	SDL_Quit();
	return 0;
}
