#ifndef __GHOST_STRUCT_H
#define __GHOST_STRUCT_H

#include "Vector.h"

typedef struct {
	Vector position;
	Vector direction;
	Vector startupPosition;
	Vector startupDirection;
} Ghost;

#endif