#ifndef __PACMAN_STRUCT_H
#define __PACMAN_STRUCT_H

#include "Vector.h"

typedef struct {
	Vector position;
	Vector direction;
} Pacman;

#endif