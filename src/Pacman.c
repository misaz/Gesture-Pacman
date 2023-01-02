#include "Pacman.h"
#include "Game.h"

void Pacman_Init(Game* game, Pacman* pacman) {
	pacman->position = game->defaultPacmanPosition;
	pacman->direction = game->defaultPacmanDirection;
}

void Pacman_DoStep(Game* game, Pacman* pacman) {
	Vector newPosition = Vector_Add(pacman->position, pacman->direction);

	if (Game_GetFieldState(game, newPosition) != FieldState_Wall) {
		pacman->position = newPosition;
	}
}
