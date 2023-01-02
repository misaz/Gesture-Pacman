#include "GameRenderer.h"
#include "BT817.h"
#include "Vector.h"

#include <math.h>

int dispW = 1024;
int dispH = 600;

int cellSize = 50;
int offsetX = 37;
int offsetY = 25;

static void GameRenderer_DrawGhost(Ghost* ghost, uint32_t color, uint8_t opacity) {
	int status;
	status = 0;

	status |= BT817_WriteDl(0x22000000); // SAVE_CONTEXT

	status |= BT817_WriteDl(0x11000008); // CLEAR_STENCIL(8)
	status |= BT817_WriteDl(0x26000002); // CLEAR(0,1,0)
	status |= BT817_WriteDl(0x1F000009); // BEGIN(RECTS)
	status |= BT817_WriteDl(0x0C000000 | (3 << 3) | 3); // STENCIL_OP(INCR, INCR)
	status |= BT817_WriteDl(0x10000000); // COLOR_A(0)
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize) * 16) << 15) | ((offsetY + ghost->position.y * cellSize) * 16));
	status |= BT817_WriteDl(0x40000000 | (((offsetX + (ghost->position.x + 1) * cellSize) * 16) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize * 2 / 3) * 16));

	status |= BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	status |= BT817_WriteDl(0x0D000000 | (cellSize / 2 - 3) * 16); // POINT_SIZE(16 * )
	status |= BT817_WriteDl(0x0A0509FF); // STENCIL_FUNC(EQUAL, 9, 255)
	status |= BT817_WriteDl(0x0C000000 | (1 << 3) | 0); // STENCIL_OP(ZERO, KEEP)
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize + cellSize / 2) * 16) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize / 2) * 16)); // VERTEX2F()

	status |= BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	status |= BT817_WriteDl(0x0C000000 | (0 << 3) | 0); // STENCIL_OP(ZERO, ZERO)
	//status |= BT817_WriteDl(0x0A0508FF); // STENCIL_FUNC(EQUAL, 8, 255)
	uint32_t smallPointSize = ((cellSize / 2 - 3) / 3) * 16;
	status |= BT817_WriteDl(0x0D000000 | smallPointSize); // POINT_SIZE()
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize + cellSize / 2) * 16) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize / 2 + 9) * 16)); // VERTEX2F()
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize + cellSize / 2 - 1) * 16 - smallPointSize * 2) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize / 2 + 9) * 16)); // VERTEX2F()
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize + cellSize / 2 + 1) * 16 + smallPointSize * 2) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize / 2 + 9) * 16)); // VERTEX2F()

	status |= BT817_WriteDl(0x1F000009); // BEGIN(RECTS)
	status |= BT817_WriteDl(0x0A0500FF); // STENCIL_FUNC(EQUAL, 0, 255)
	status |= BT817_WriteDl(0x04000000 | color); // COLOR_RGB(param)
	status |= BT817_WriteDl(0x10000000 | opacity); // COLOR_A(opacity)
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize) * 16) << 15) | ((offsetY + ghost->position.y * cellSize) * 16));
	status |= BT817_WriteDl(0x40000000 | (((offsetX + (ghost->position.x + 1) * cellSize) * 16) << 15) | ((offsetY + (ghost->position.y + 1) * cellSize) * 16));

	status |= BT817_WriteDl(0x0D000000 | 32); // POINT_SIZE()
	status |= BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	status |= BT817_WriteDl(0x04000000); // COLOR_RGB(black)
	status |= BT817_WriteDl(0x100000FF); // COLOR_A(0xFF)
	status |= BT817_WriteDl(0x0A0700FF); // STENCIL_FUNC(ALWAYS, 0, 255)
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize + cellSize / 3) * 16) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize / 3) * 16)); // VERTEX2F()
	status |= BT817_WriteDl(0x40000000 | (((offsetX + ghost->position.x * cellSize + cellSize * 2 / 3) * 16) << 15) | ((offsetY + ghost->position.y * cellSize + cellSize / 3) * 16)); // VERTEX2F()

	status |= BT817_WriteDl(0x23000000); // RESTORE_CONTEXT
}

static void GameRenderer_DrawPacman(Game* game, Pacman* pacman) {
	int status;
	status = 0;

	status |= BT817_WriteDl(0x22000000); // SAVE_CONTEXT

	status |= BT817_WriteDl(0x11000000); // CLEAR_STENCIL(0)
	status |= BT817_WriteDl(0x26000002); // CLEAR(0,1,0)
	status |= BT817_WriteDl(0x0C000000 | (3 << 3) | 3); // STENCIL_OP(INCR, INCR)
	status |= BT817_WriteDl(0x10000000); // COLOR_A(0)

	int deg;
	int eyeDegAdd;
	if (Vector_Cmp(pacman->direction, vectorRight)) {
		status |= BT817_WriteDl(0x1F000005); // BEGIN(EDGE_STRIP_R)
		deg = -30;
		eyeDegAdd = -30;
	} else if (Vector_Cmp(pacman->direction, vectorUp)) {
		status |= BT817_WriteDl(0x1F000007); // BEGIN(EDGE_STRIP_A)
		deg = -120;
		eyeDegAdd = -30;
	} else if (Vector_Cmp(pacman->direction, vectorLeft)) {
		status |= BT817_WriteDl(0x1F000006); // BEGIN(EDGE_STRIP_L)
		deg = -150;
		eyeDegAdd = 30;
	} else if (Vector_Cmp(pacman->direction, vectorDown)) {
		status |= BT817_WriteDl(0x1F000008); // BEGIN(EDGE_STRIP_B)
		deg = -300;
		eyeDegAdd = -30;
	}

	status |= BT817_WriteDl(0x40000000 | (((offsetX + pacman->position.x * cellSize + cellSize / 2) * 16) << 15) | ((offsetY + pacman->position.y * cellSize + cellSize / 2) * 16)); // VERTEX2F()
	status |= BT817_WriteDl(0x40000000 | (((offsetX + pacman->position.x * cellSize + cellSize / 2 + (int)(cos(deg * 3.14 / 180) * cellSize)) * 16) << 15) | ((offsetY + pacman->position.y * cellSize + cellSize / 2 + (int)(sin(deg * 3.14 / 180) * cellSize)) * 16)); // VERTEX2F()


	status |= BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	status |= BT817_WriteDl(0x0A0500FF); // STENCIL_FUNC(EQUAL, 0, 255)
	status |= BT817_WriteDl(0x0D000000 | (cellSize / 2 - 3) * 16); // POINT_SIZE(16 * ...)
	status |= BT817_WriteDl(0x100000FF); // COLOR_A(255)
	status |= BT817_WriteDl(0x04FFFF00); // COLOR_RGB(0xFF,0xFF,0)
	status |= BT817_WriteDl(0x40000000 | (((offsetX + pacman->position.x * cellSize + cellSize / 2) * 16) << 15) | ((offsetY + pacman->position.y * cellSize + cellSize / 2) * 16)); // VERTEX2F()

	status |= BT817_WriteDl(0x0A0700FF); // STENCIL_FUNC(ALWAYS, 0, 255)
	status |= BT817_WriteDl(0x04000000); // COLOR_RGB(0,0,0)
	status |= BT817_WriteDl(0x0D000000 | 32); // POINT_SIZE()
	deg += eyeDegAdd;
	status |= BT817_WriteDl(0x40000000 | (((offsetX + pacman->position.x * cellSize + cellSize / 2 + (int)(cos(deg * 3.14 / 180) * cellSize / 3)) * 16) << 15) | ((offsetY + pacman->position.y * cellSize + cellSize / 2 + (int)(sin(deg * 3.14 / 180) * cellSize / 3)) * 16)); // VERTEX2F()


	status |= BT817_WriteDl(0x23000000); // RESTORE_CONTEXT

	// draw pacman yellow dot
}

static void GameRenderer_DrawHeart(uint32_t x, uint32_t y, uint32_t color) {
	int status = 0;

	status |= BT817_WriteDl(0x22000000); // SAVE_CONTEXT

	status |= BT817_WriteDl(0x04000000 | color); // COLOR_RGB(0xFF,0,0)
	status |= BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	status |= BT817_WriteDl(0x0D000000 | 10 * 16); // POINT_SIZE()

	status |= BT817_WriteDl(0x40000000 | (((x + 10) * 16) << 16) | ((y + 20) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 18) * 16) << 16) | ((y + 20) * 16));

	status |= BT817_WriteDl(0x11000008); // CLEAR_STENCIL(8)
	status |= BT817_WriteDl(0x26000002); // CLEAR(0,1,0)

	status |= BT817_WriteDl(0x0C000000 | (3 << 3) | 3); // STENCIL_OP(INCR, INCR)
	status |= BT817_WriteDl(0x1F000008); // BEGIN(EDGE_STRIP_B)
	status |= BT817_WriteDl(0x10000000); // COLOR_A(0)

	status |= BT817_WriteDl(0x40000000 | (((x + 0) * 16) << 16) | ((y + 25) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 6) * 16) << 16) | ((y + 25) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 6) * 16) << 16) | ((y + 25) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 14) * 16) << 16) | ((y + 45) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 14) * 16) << 16) | ((y + 45) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 22) * 16) << 16) | ((y + 25) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 22) * 16) << 16) | ((y + 25) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 28) * 16) << 16) | ((y + 25) * 16));

	status |= BT817_WriteDl(0x0A0508FF); // STENCIL_FUNC(EQUAL, 8, 255)
	status |= BT817_WriteDl(0x100000FF); // COLOR_A(255)
	status |= BT817_WriteDl(0x1F000009); // BEGIN(RECTS)
	status |= BT817_WriteDl(0x40000000 | (((x + 6) * 16) << 16) | ((y + 20) * 16));
	status |= BT817_WriteDl(0x40000000 | (((x + 22) * 16) << 16) | ((y + 45) * 16));


	status |= BT817_WriteDl(0x23000000); // RESTORE_CONTEXT
}

void GameRenderer_RenderGame(Game* game) {
	int status;

	status = 0;

	if (game->isWin) {
		status |= BT817_WriteDl(0x0200FF00); // CLEAR_COLOR_RGB(0,0,0x44)
	} else if (game->isLost) {
		status |= BT817_WriteDl(0x02FF0000); // CLEAR_COLOR_RGB(0,0,0x44)
	} else {
		status |= BT817_WriteDl(0x02000044); // CLEAR_COLOR_RGB(0,0,0x44)
	}
	status |= BT817_WriteDl(0x26000007); // CLEAR(1,1,1)

	status |= BT817_WriteDl(0x1F000009); // BEGIN(RECTS)
	status |= BT817_WriteDl(0x04004400); // COLOR_RGB(0,0x44,0)

	status |= BT817_WriteDl(0x40000000 | ((offsetX * 16) << 15) | (offsetY * 16));
	status |= BT817_WriteDl(0x40000000 | (((offsetX + MAP_WIDTH * cellSize) * 16) << 15) | ((offsetY + MAP_HEIGHT * cellSize) * 16)); // VERTEX2F()

	status |= BT817_WriteDl(0x04440000); // COLOR_RGB(0x44,0,0)
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (game->map[y][x] == FieldState_Wall) {
				status |= BT817_WriteDl(0x40000000 | (((offsetX + x * cellSize) * 16) << 15) | ((offsetY + y * cellSize) * 16)); // VERTEX2F()
				status |= BT817_WriteDl(0x40000000 | (((offsetX + (x + 1) * cellSize) * 16) << 15) | ((offsetY + (y + 1) * cellSize) * 16)); // VERTEX2F()
			}
		}
	}

	status |= BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	status |= BT817_WriteDl(0x04AA00AA); // COLOR_RGB(0,0x44,0)
	status |= BT817_WriteDl(0x0D000000 | cellSize * 16 / 10); // POINT_SIZE()

	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (game->map[y][x] == FieldState_Point) {
				status |= BT817_WriteDl(0x40000000 | (((offsetX + x * cellSize + cellSize / 2) * 16) << 15) | ((offsetY + y * cellSize + cellSize / 2) * 16)); // VERTEX2F()
			}
		}
	}

	status |= BT817_WriteDl(0x04FF0000); // COLOR_RGB(0,0x44,0)
	status |= BT817_WriteDl(0x0D000000 | cellSize * 16 / 4); // POINT_SIZE()
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (game->map[y][x] == FieldState_Bonus) {
				status |= BT817_WriteDl(0x40000000 | (((offsetX + x * cellSize + cellSize / 2) * 16) << 15) | ((offsetY + y * cellSize + cellSize / 2) * 16)); // VERTEX2F()
			}
		}
	}

	uint8_t opacity = 255;
	if (game->bonusRemainingTicks > 0) {
		opacity = 50;
	}

	// change drawing order when bonus activated 
	if (game->bonusRemainingTicks > 0) {
		if (!game->isLost) {
			GameRenderer_DrawPacman(game, &game->pacman);
		}

		if (!game->isWin) {
			GameRenderer_DrawGhost(game->ghosts + 0, 0x0000FFFF, opacity);
			GameRenderer_DrawGhost(game->ghosts + 1, 0x0000BBBB, opacity);
			GameRenderer_DrawGhost(game->ghosts + 2, 0x00007777, opacity);
		}
	} else {
		if (!game->isWin) {
			GameRenderer_DrawGhost(game->ghosts + 0, 0x0000FFFF, opacity);
			GameRenderer_DrawGhost(game->ghosts + 1, 0x0000BBBB, opacity);
			GameRenderer_DrawGhost(game->ghosts + 2, 0x00007777, opacity);
		}

		if (!game->isLost) {
			GameRenderer_DrawPacman(game, &game->pacman);
		}
	}
	for (int i = 0; i < 3; i++) {
		uint32_t color = 0xFF0000;
		if (game->lives <= i) {
			color = 0x000000;
		}

		GameRenderer_DrawHeart(i * 20, 0, color);
	}

	status |= BT817_WriteDl(0x00000000); // DISPLAY()

	BT817_SwapDl();
}
