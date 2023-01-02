#include "GestureDetect.h"
#include "MAX25405.h"
#include "BT817.h"

#include <string.h>
#include <stdio.h>

#define GESTURE_ACTIVE_THRESHOLD 			300
#define GESTURE_POINTS_MAX					50
#define GESTURE_ACTIVATION_STATUS_MEMORY 	5
#define GESTURE_MINIMUM_POINTS				6
#define SIMILAR_GESTURE_SCORE_THRESHOLD		0.9
#define LOW_PASS_COEF						0.9
#define HIGH_PASS_COEF						0.05

typedef struct {
	float x;
	float y;

	int32_t data_max;
	int32_t filtered_max;
	int32_t data_at_filtered_max;
} GesturePoint;

typedef struct {
	int16_t* original_data;

	int32_t data32[MAX25405_PIXELS];
	int32_t data32_filtered[MAX25405_PIXELS];

	int32_t data32_max;
	int32_t data32_filtered_max;
	int32_t data32_at_filtered_max;
} DetectionSession;

static GesturePoint gesturePoints[GESTURE_POINTS_MAX];
static size_t gesturePointsLength = 0;

static float low_pass_memory[MAX25405_PIXELS];
static float high_pass_memory[MAX25405_PIXELS];

static int wasGestureInProgress[GESTURE_ACTIVATION_STATUS_MEMORY];
static int isGestureInProgress = 0;

// original_data in lastDetectionSession is invalid!!!!!! Never use it.
static DetectionSession lastDetectionSession;
static int lastGesture = -1;
static int lastGestureShowTime = 0;

static void GestureDetect_ComputeCenterOfMass(int32_t* data, float* x, float* y) {
	float xTotal = 0;
	float yTotal = 0;
	float total = 0;

	for (int y = 0; y < MAX25405_ROWS; y++) {
		for (int x = 0; x < MAX25405_COLUMNS; x++) {
			int i = y * MAX25405_COLUMNS + x;

			int32_t val = data[i];
			if (val < 0) {
				val = -val;
			}

			xTotal += val * (x + 1);
			yTotal += val * (y + 1);
			total += val;
		}
	}

	if (total == 0) {
		total = 1;
	}

	*x = xTotal / total - 1;
	*y = yTotal / total - 1;
}

static int GestureDetect_IsGestureInProgress(DetectionSession* session) {
	for (int i = 0; i < MAX25405_PIXELS; i++) {
		if (session->data32_filtered[i] > GESTURE_ACTIVE_THRESHOLD ||
			session->data32_filtered[i] < -GESTURE_ACTIVE_THRESHOLD) {

			return 1;
		}
	}
	return 0;
}

void GestureDetect_Init() {
	for (int i = 0; i < MAX25405_PIXELS; i++) {
		low_pass_memory[i] = 0;
		high_pass_memory[i] = 0;
	}

	for (int i = 0; i < GESTURE_ACTIVATION_STATUS_MEMORY; i++) {
		wasGestureInProgress[i] = 0;
	}
}

static void GestureDetect_Preprocess(DetectionSession* session) {
	session->data32_max = -99999999;
	session->data32_filtered_max = -9999999;

	for (int i = 0; i < MAX25405_PIXELS; i++) {
		session->data32[i] = session->original_data[i];

		float after_low_pass = (1 - LOW_PASS_COEF) * low_pass_memory[i] + LOW_PASS_COEF * session->data32[i];
		float after_high_pass = (1 - HIGH_PASS_COEF) * high_pass_memory[i] + HIGH_PASS_COEF * after_low_pass;

		low_pass_memory[i] = after_low_pass;
		high_pass_memory[i] = after_high_pass;

		session->data32_filtered[i] = (int32_t)(after_low_pass - after_high_pass);

		if (session->data32[i] > session->data32_max) {
			session->data32_max = session->data32[i];
		}

		if (session->data32_filtered[i] > session->data32_filtered_max) {
			session->data32_filtered_max = session->data32_filtered[i];
			session->data32_at_filtered_max = session->data32[i];
		}
	}
}

static void GestureDetect_AddPoint(DetectionSession* session) {
	GesturePoint* point;

	if (gesturePointsLength == GESTURE_POINTS_MAX) {
		for (int i = 0; i < GESTURE_POINTS_MAX - 1; i++) {
			memcpy(gesturePoints + i, gesturePoints + i + 1, sizeof(GesturePoint));
		}
		point = gesturePoints + GESTURE_POINTS_MAX - 1;
	} else {
		point = gesturePoints + gesturePointsLength++;
	}

	GestureDetect_ComputeCenterOfMass(session->data32_filtered, &point->x, &point->y);

	point->data_max = session->data32_max;
	point->filtered_max = session->data32_filtered_max;
	point->data_at_filtered_max = session->data32_at_filtered_max;
}

static void GestureDetect_GetPointsValuesInfo(int32_t* gesture_data_min, int32_t* gesture_data_range) {
	*gesture_data_min = 99999999;
	int32_t gesture_data_max = -99999999;

	for (int i = 0; i < gesturePointsLength; i++) {
		GesturePoint* p = gesturePoints + i;

		if (p->data_at_filtered_max > gesture_data_max) {
			gesture_data_max = p->data_at_filtered_max;
		}

		if (p->data_at_filtered_max < *gesture_data_min) {
			*gesture_data_min = p->data_at_filtered_max;
		}
	}

	*gesture_data_range = gesture_data_max - *gesture_data_min;
}


static int GestureDetect_EvaluateGesture(DetectionSession* session) {
	if (gesturePointsLength < GESTURE_MINIMUM_POINTS) {
		return DIRECTION_UNKNOWN;
	}

	float directonScore[4];
	directonScore[DIRECTION_UP] = 0;
	directonScore[DIRECTION_DOWN] = 0;
	directonScore[DIRECTION_LEFT] = 0;
	directonScore[DIRECTION_RIGHT] = 0;

	int32_t gesture_data_min;
	int32_t gesture_data_range;

	GestureDetect_GetPointsValuesInfo(&gesture_data_min, &gesture_data_range);
	if (gesture_data_range == 0) {
		gesture_data_range = 1;
	}

	for (int i = 0; i < gesturePointsLength - 1; i++) {
		GesturePoint* p1 = gesturePoints + i;
		GesturePoint* p2 = gesturePoints + i + 1;

		int direction = -1;
		float distance;

		// 4 and 8 are compensations for non-rectangular sensor resolution (4 + 2) * (8 + 2) = 6 * 10 = 60px
		// 2 is compensation because border pxels are less sensitive
		float diffX = (p2->x - p1->x) * 4;
		float diffY = (p2->y - p1->y) * 8;

		float absDiffX = diffX;
		if (absDiffX < 0) {
			absDiffX = -absDiffX;
		}

		float absDiffY = diffY;
		if (absDiffY < 0) {
			absDiffY = -absDiffY;
		}

		if (absDiffX > absDiffY) { // horizontal movement
			if (diffX > 0) {
				direction = DIRECTION_RIGHT;
				distance = diffX;
			} else {
				direction = DIRECTION_LEFT;
				distance = -diffX;
			}
		} else { // vertical movement
			if (diffY > 0) {
				direction = DIRECTION_DOWN;
				distance = diffY;
			} else {
				direction = DIRECTION_UP;
				distance = -diffY;
			}
		}

		// remove unprobable large distances, often caused by initial noise before gesture starts
		if (distance > 3 || distance < -3) {
			distance = 0;
		}

		float point1_score = (p1->data_at_filtered_max - gesture_data_min) / (float)gesture_data_range; // between 0 and 1
		float point2_score = (p2->data_at_filtered_max - gesture_data_min) / (float)gesture_data_range; // between 0 and 1

		directonScore[direction] += distance * (point1_score) * (point2_score);
	}

	float maxScore = directonScore[0];
	int maxScoreDirection = 0;
	for (int i = 1; i  < 4; i++) {
		if (directonScore[i] > maxScore) {
			maxScore = directonScore[i];
			maxScoreDirection = i;
		}
	}

	for (int i = 1; i < 4; i++) {
		if (i != maxScoreDirection && directonScore[i] > maxScore * SIMILAR_GESTURE_SCORE_THRESHOLD) {
			return DIRECTION_UNKNOWN;
		}
	}

	return maxScoreDirection;
}


int GestureDetect_AddDataAndGetGesture(int16_t* data) {
	DetectionSession session;
	session.original_data = data;

	GestureDetect_Preprocess(&session);

	// copy session for furhter debug rendering
	memcpy(&lastDetectionSession, &session, sizeof(DetectionSession));

	int isCurrentFrameGesture = GestureDetect_IsGestureInProgress(&session);

	for (int i = 0; i < GESTURE_ACTIVATION_STATUS_MEMORY - 1; i++) {
		wasGestureInProgress[i] = wasGestureInProgress[i + 1];
	}

	wasGestureInProgress[GESTURE_ACTIVATION_STATUS_MEMORY - 1] = isCurrentFrameGesture;

	// check for gesture start
	if (!isGestureInProgress && isCurrentFrameGesture) {
		gesturePointsLength = 0;
		isGestureInProgress = 1;
	}

	// process gesture
	if (isCurrentFrameGesture) {
		GestureDetect_AddPoint(&session);
	}

	// check for gesture end
	int isEnd = 1;
	for (int i = 0; i < GESTURE_ACTIVATION_STATUS_MEMORY; i++) {
		if (wasGestureInProgress[i]) {
			isEnd = 0;
			break;
		}
	}
	if (isGestureInProgress && isEnd) {
		lastGesture = GestureDetect_EvaluateGesture(&session);
		lastGestureShowTime = 100;
		isGestureInProgress = 0;
		return lastGesture;
	} else {
		return -1;
	}
}

void GestureDetect_RenderDebugScreen() {
	BT817_WriteDl(0x02000000); // CLEAR_COLOR_RGB(R,G,B)
	BT817_WriteDl(0x26000007); // CLEAR(1,1,1)
	BT817_WriteDl(0x05000000 | 24); // BITMAP_HANDLE(16)

	int32_t* data = lastDetectionSession.data32_filtered;
	int32_t min = *data;
	int32_t max = *data;
	for (int i = 0; i < MAX25405_PIXELS; i++) {
		if (data[i] < min) {
			min = data[i];
		}
		if (data[i] > max) {
			max = data[i];
		}
	}

	int32_t intensityLimit = max;
	if (intensityLimit < 1000) {
		intensityLimit = 1000;
	}

	for (int y = 0; y < 6; y++) {
		for (int x = 0; x < 10; x++) {
			int value = (int)data[y * 10 + x] - (int)min;
			int16_t intensity;

			if (value < 0) {
				intensity = 0;
			} else {
				intensity = 255 * value / intensityLimit;
				if (intensity > 255) {
					intensity = 255;
				}
			}

			int x1 = x * 128;
			int y1 = y * 100;
			int x2 = x1 + 127;
			int y2 = y1 + 99;

			BT817_WriteDl(0x1F000009); // BEGIN(RECTS)
			BT817_WriteDl(0x04000000 | (intensity << 8)); // COLOR_RGB(0,0x44,0)
			BT817_WriteDl(0x40000000 | ((x1 * 16) << 15) | (y1 * 16)); // VERTEX2F()
			BT817_WriteDl(0x40000000 | ((x2 * 16) << 15) | (y2 * 16)); // VERTEX2F()

			char buffer[32];
			int xLetter = x1 + 10;
			int yLetter = y1 + 30;

			snprintf(buffer, sizeof(buffer), "%d", (int)data[y * 10 + x]);

			char* bufferRd = buffer;
			char current;
			BT817_WriteDl(0x1F000001); // BEGIN(BITMAPS)
			BT817_WriteDl(0x04FFFFFF); // COLOR_RGB()
			while ((current = *bufferRd++)) {
				BT817_WriteDl(0x06000000 | current); // CELL()
				BT817_WriteDl(0x40000000 | ((xLetter * 16) << 15) | (yLetter * 16)); // VERTEX2F()
				xLetter += 15;
			}
		}
	}

	if (gesturePointsLength > 1) {
		BT817_WriteDl(0x04FFFFFF); // COLOR_RGB(255,255,255)
		BT817_WriteDl(0x1F000003); // BEGIN(LINES)
		BT817_WriteDl(0x40000000 | (((int)(gesturePoints[0].x * 128 + 64) * 16) << 15) | ((int)(gesturePoints[0].y * 100 + 50) * 16)); // VERTEX2F()
		for (int i = 1; i < gesturePointsLength; i++) {
			BT817_WriteDl(0x40000000 | (((int)(gesturePoints[i].x * 128 + 64) * 16) << 15) | ((int)(gesturePoints[i].y * 100 + 50) * 16)); // VERTEX2F()
			BT817_WriteDl(0x40000000 | (((int)(gesturePoints[i].x * 128 + 64) * 16) << 15) | ((int)(gesturePoints[i].y * 100 + 50) * 16)); // VERTEX2F()
		}
	}

	if (gesturePointsLength > 0) {
		int32_t min;
		int32_t range;
		GestureDetect_GetPointsValuesInfo(&min, &range);
		if (range == 0) {
			range = 1;
		}

		BT817_WriteDl(0x1F000002); // BEGIN(POINTS)

		for (int i = 0; i < gesturePointsLength; i++) {
			//BT817_WriteDl(0x04000000 | (i * 255 / DECTION_FRAME_COUNT)); // COLOR_RGB(255,0,0)
			BT817_WriteDl(0x040000FF); // COLOR_RGB(255,0,0)
			BT817_WriteDl(0x0D000000 | (16 * 10 + (gesturePoints[i].data_at_filtered_max - min) * 300 / range)); // POINT_SIZE()
			BT817_WriteDl(0x40000000 | (((int)(gesturePoints[i].x * 128 + 64) * 16) << 15) | ((int)(gesturePoints[i].y * 100 + 50) * 16)); // VERTEX2F()
		}
	}

	BT817_WriteDl(0x04FF0000); // COLOR_RGB(255,0,0)

	char* gestureStr = "--";
	if (lastGestureShowTime > 0) {
		if (lastGesture == DIRECTION_UP) {
			gestureStr = "UP";
		} else if (lastGesture == DIRECTION_DOWN) {
			gestureStr = "DOWN";
		} else if (lastGesture == DIRECTION_LEFT) {
			gestureStr = "LEFT";
		} else if (lastGesture == DIRECTION_RIGHT) {
			gestureStr = "RIGHT";
		} else if (lastGesture == DIRECTION_UNKNOWN) {
			gestureStr = "UNK";
		}
		lastGestureShowTime--;
	}

	BT817_WriteDl(0x1F000001); // BEGIN(BITMAPS)
	BT817_WriteDl(0x05000000 | 31); // BITMAP_HANDLE()
	BT817_WriteDl(0x04FFFF00); // COLOR_RGB()

	char current;
	int xLetter = 600;
	while ((current = *gestureStr++)) {
		BT817_WriteDl(0x06000000 | current); // CELL()
		BT817_WriteDl(0x40000000 | ((xLetter * 16) << 15) | (550 * 16)); // VERTEX2F()
		xLetter += 25;
	}


	BT817_WriteDl(0x1F000002); // BEGIN(POINTS)
	if (isGestureInProgress) {
		BT817_WriteDl(0x0400FF00); // COLOR_RGB(255,0,0)
	} else {
		BT817_WriteDl(0x04FF0000); // COLOR_RGB(255,0,0)
	}
	BT817_WriteDl(0x0D000000 | (16 * 50));
	BT817_WriteDl(0x40000000 | (((int)(950) * 16) << 15) | ((int)(500) * 16)); // VERTEX2F()

	BT817_WriteDl(0x00000000); // DISPLAY()
	BT817_SwapDl();
}
