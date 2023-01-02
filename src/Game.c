#include "Game.h"
#include "Pacman.h"
#include "Ghost.h"

#include <string.h>

static const FieldState defaultMap[MAP_HEIGHT][MAP_WIDTH] = {
	/*        0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 */
	/*        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 */
	/*  0 */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	/*  1 */ {1,3,3,3,3,1,3,3,3,3,3,3,3,1,3,3,3,3,1},
	/*  2 */ {1,3,1,1,3,1,3,1,1,1,1,1,3,1,3,1,1,3,1},
	/*  3 */ {1,3,1,3,3,3,3,3,3,3,3,3,3,3,3,3,1,3,1},
	/*  4 */ {1,3,1,3,1,1,3,1,1,0,1,1,3,1,1,3,1,3,1},
	/*  5 */ {1,3,3,3,3,3,3,1,0,0,0,1,3,3,3,3,3,3,1},
	/*  6 */ {1,3,1,3,1,1,3,1,1,1,1,1,3,1,1,3,1,3,1},
	/*  7 */ {1,3,1,2,3,3,3,3,3,3,3,3,3,3,3,2,1,3,1},
	/*  8 */ {1,3,1,1,3,1,3,1,1,1,1,1,3,1,3,1,1,3,1},
	/*  9 */ {1,3,3,3,3,1,3,3,3,3,3,3,3,1,3,3,3,3,1},
	/* 13 */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static const Vector defaultGhostLocations[] = {
	{.x = 8, .y = 5 },
	{.x = 9, .y = 5 },
	{.x = 10, .y = 5 }
};

static const Vector defaultGhostDirections[] = {
	{.x = 1, .y = 0 },
	{.x = 0, .y = -1 },
	{.x = -1, .y = 0 }
};

void Game_Init(Game * game) {
	game->defaultPacmanDirection = vectorRight;
	game->defaultPacmanPosition.x = 9;
	game->defaultPacmanPosition.y = 7;
	game->lives = 3;
	game->isRunning = 0;
	game->isLost = 0;
	game->isWin = 0;
	game->bonusRemainingTicks = 0;

	memcpy(game->map, defaultMap, sizeof(game->map));

	Pacman_Init(game, &game->pacman);

	for (int i = 0; i < GHOSTS_COUNT; i++) {
		Ghost_Init(game->ghosts + i, defaultGhostLocations[i], defaultGhostDirections[i]);
	}
}

FieldState Game_GetFieldState(Game* game, Vector position) {
	return game->map[position.y][position.x];
}

static void Game_HandlePacmanDied(Game* game) {
	game->lives--;

	if (game->lives == 0) {
		game->isLost = 1;
		game->isRunning = 0;
	}

	for (Ghost* i = game->ghosts, *end = game->ghosts + GHOSTS_COUNT; i < end; i++) {
		i->position = i->startupPosition;
		i->direction = i->startupDirection;
	}

	game->pacman.position = game->defaultPacmanPosition;
	game->pacman.direction = game->defaultPacmanDirection;
}

static void Game_CheckWin(Game* game) {
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (game->map[y][x] == FieldState_Point || game->map[y][x] == FieldState_Bonus) {
				return;
			}
		}
	}

	game->isWin = 1;
	game->isRunning = 0;
}

static void Game_ActivateBonus(Game* game) {
	game->bonusRemainingTicks = 25;
}


void Game_DoStep(Game* game) {
	if (!game->isRunning) {
		return;
	}

	Pacman_DoStep(game, &game->pacman);

	for (Ghost* i = game->ghosts, *end = game->ghosts + GHOSTS_COUNT; i < end; i++) {
		if (game->bonusRemainingTicks == 0 && Vector_Cmp(i->position, game->pacman.position)) {
			Game_HandlePacmanDied(game);
			return;
		}

		Ghost_DoStep(game, i);

		if (game->bonusRemainingTicks == 0 && Vector_Cmp(i->position, game->pacman.position)) {
			Game_HandlePacmanDied(game);
			return;
		}
	}

	if (game->map[game->pacman.position.y][game->pacman.position.x] == FieldState_Bonus) {
		game->map[game->pacman.position.y][game->pacman.position.x] = FieldState_Empty;
		Game_ActivateBonus(game);
	}

	if (game->map[game->pacman.position.y][game->pacman.position.x] == FieldState_Point) {
		game->map[game->pacman.position.y][game->pacman.position.x] = FieldState_Empty;
	}

	Game_CheckWin(game);

	if (game->bonusRemainingTicks) {
		game->bonusRemainingTicks--;
	}
}

void Game_Start(Game* game) {
	game->isRunning = 1;
}

