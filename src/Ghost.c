#include "Ghost.h"
#include "Game.h"
#include "UART.h"

#include "mxc.h"

static int isRngInitialized = 0;

void Ghost_Init(Ghost* ghost, Vector position, Vector direction) {
	int status;

	ghost->direction = direction;
	ghost->position = position;
	ghost->startupDirection = direction;
	ghost->startupPosition = position;

	// init RNG
	if (!isRngInitialized) {
		status = MXC_TRNG_Init();
		if (status) {
			__BKPT(0);
		}

		isRngInitialized = 1;
	}
}

int Ghost_DoStep(Game* game, Ghost* ghost) {
	Vector abnodedField = Vector_Add(ghost->position, Vector_Inv(ghost->direction));

	Vector fieldRightLocation = Vector_Add(ghost->position, vectorRight);
	Vector fieldLeftLocation = Vector_Add(ghost->position, vectorLeft);
	Vector fieldUpLocation = Vector_Add(ghost->position, vectorUp);
	Vector fieldDownLocation = Vector_Add(ghost->position, vectorDown);

	int canMoveRight = Game_GetFieldState(game, fieldRightLocation) != FieldState_Wall && Vector_Cmp(fieldRightLocation, abnodedField) == 0;
	int canMoveLeft = Game_GetFieldState(game, fieldLeftLocation) != FieldState_Wall && Vector_Cmp(fieldLeftLocation, abnodedField) == 0;
	int canMoveUp = Game_GetFieldState(game, fieldUpLocation) != FieldState_Wall && Vector_Cmp(fieldUpLocation, abnodedField) == 0;
	int canMoveDown = Game_GetFieldState(game, fieldDownLocation) != FieldState_Wall && Vector_Cmp(fieldDownLocation, abnodedField) == 0;

	Vector allowedLocations[4];
	Vector allowedDirections[4];
	int allowedDirectionCount = 0;

	if (canMoveRight) {
		allowedDirections[allowedDirectionCount] = vectorRight;
		allowedLocations[allowedDirectionCount++] = fieldRightLocation;
	}
	if (canMoveLeft) {
		allowedDirections[allowedDirectionCount] = vectorLeft;
		allowedLocations[allowedDirectionCount++] = fieldLeftLocation;
	}
	if (canMoveUp) {
		allowedDirections[allowedDirectionCount] = vectorUp;
		allowedLocations[allowedDirectionCount++] = fieldUpLocation;
	}
	if (canMoveDown) {
		allowedDirections[allowedDirectionCount] = vectorDown;
		allowedLocations[allowedDirectionCount++] = fieldDownLocation;
	}

	if (allowedDirectionCount > 0) {
		uint32_t randomNumber = (uint32_t)MXC_TRNG_RandomInt();

		randomNumber %= allowedDirectionCount;

		ghost->position = allowedLocations[randomNumber];
		ghost->direction = allowedDirections[randomNumber];
	} else {
		ghost->position = abnodedField;
		ghost->direction = Vector_Inv(ghost->direction);
	}

	return 0;
}

void Ghost_SetDefaultPosition(Ghost* ghost) {
	ghost->position = ghost->startupPosition;
	ghost->direction = ghost->startupDirection;
}
