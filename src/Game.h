#ifndef __GAME_H
#define __GAME_H

#include "GameStruct.h"

void Game_Init(Game* game);
FieldState Game_GetFieldState(Game* game, Vector position);
void Game_DoStep(Game* game);
void Game_Start(Game* game);

#endif