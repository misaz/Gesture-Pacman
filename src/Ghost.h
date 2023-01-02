#ifndef __GHOST_H
#define __GHOST_H

#include "Vector.h"
#include "GameStruct.h"
#include "GhostStruct.h"

void Ghost_Init(Ghost* ghost, Vector position, Vector direction);
int Ghost_DoStep(Game* game, Ghost* ghost);
void Ghost_SetDefaultPosition(Ghost* ghost);

#endif