#ifndef __GANE_STRUCT_H
#define __GANE_STRUCT_H

#include "Vector.h"
#include "FieldState.h"
#include "PacmanStruct.h"
#include "GhostStruct.h"

#define MAP_WIDTH 19
#define MAP_HEIGHT 11
#define GHOSTS_COUNT 3

typedef struct {
	Pacman pacman;
	Ghost ghosts[GHOSTS_COUNT];
	FieldState map[MAP_HEIGHT][MAP_WIDTH];

	Vector defaultPacmanPosition;
	Vector defaultPacmanDirection;
	int lives;

	int isRunning;
	int isLost;
	int isWin;

	int bonusRemainingTicks;
} Game;

#endif