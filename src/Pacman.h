#ifndef __PACMAN_H
#define __PACMAN_H

#include "GameStruct.h"
#include "PacmanStruct.h"

void Pacman_Init(Game* game, Pacman* pacman);
void Pacman_DoStep(Game* game, Pacman* pacman);

#endif