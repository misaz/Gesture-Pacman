#include "MAX25405.h"

#include <string.h>
#include "GestureDetect.h"

float lastCentersX[DECTION_FRAME_COUNT];
float lastCentersY[DECTION_FRAME_COUNT];
int32_t lastMaximums[DECTION_FRAME_COUNT];

int lastGestures[OUTPUT_STATE_FILTERING_MEMORY];
int isDirectionBlocked[4];
int stuckAtDirection[4];

float low_pass_memory[MAX25405_PIXELS];
float high_pass_memory[MAX25405_PIXELS];


static void computeCenterOfMass(int32_t* data, float* x, float* y) {
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

static int detectGesture(float* lastCentersX, float* lastCentersY, int32_t* lastMaximums, int32_t data_processed_max) {
	if (data_processed_max < 300) {
		return -1;
	}

	int directions[DECTION_FRAME_COUNT - 1];
	float distances[DECTION_FRAME_COUNT - 1];

	int32_t minMax = *lastMaximums;
	int32_t maxMax = *lastMaximums;

	for (int i = 1; i < DECTION_FRAME_COUNT; i++) {
		if (lastMaximums[i] < minMax) {
			minMax = lastMaximums[i];
		}
		if (lastMaximums[i] > maxMax) {
			maxMax = lastMaximums[i];
		}
	}

	float rangeMax = maxMax - minMax;
	if (rangeMax == 0) {
		rangeMax = 1;
	}

	float directonScore[4];
	directonScore[DIRECTION_UP] = 0;
	directonScore[DIRECTION_DOWN] = 0;
	directonScore[DIRECTION_LEFT] = 0;
	directonScore[DIRECTION_RIGHT] = 0;

	for (int i = 0; i < DECTION_FRAME_COUNT - 1; i++) {
		// 4 and 8 are compensations for non-rectangular sensor resolution (4 + 2) * (8 + 2) = 6 * 10 = 60px
		float diffX = (lastCentersX[i + 1] - lastCentersX[i]) * 4;
		float diffY = (lastCentersY[i + 1] - lastCentersY[i]) * 8;

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
				directions[i] = DIRECTION_RIGHT;
				distances[i] = diffX;
			} else {
				directions[i] = DIRECTION_LEFT;
				distances[i] = -diffX;
			}
		} else { // vertical movement
			if (diffY > 0) {
				directions[i] = DIRECTION_DOWN;
				distances[i] = diffY;
			} else {
				directions[i] = DIRECTION_UP;
				distances[i] = -diffY;
			}
		}

		// remove unprobable large distances, often caused by initial noise before gesture starts
		if (distances[i] > 3 || distances[i] < -3) {
			distances[i] = 0;
		}
	}

	// compute direction score
	for (int i = 0; i < DECTION_FRAME_COUNT - 1; i++) {
		if (directions[i] == directions[i + 1]) {
			float point1_score = (lastMaximums[i] - minMax) / rangeMax; // between 0 and 1
			float point2_score = (lastMaximums[i + 1] - minMax) / rangeMax; // between 0 and 1

			directonScore[directions[i]] += distances[i] * distances[i] * (point1_score) * (point2_score);
		}
	}


	float maxScore = directonScore[0];
	int maxScoreDirection = 0;

	for (int i = 1; i  < 4; i++) {
		if (directonScore[i] > maxScore) {
			maxScore = directonScore[i];
			maxScoreDirection = i;
		}
	}


	for (int i = 1; i  < 4; i++) {
		if (i != maxScoreDirection && directonScore[i] > maxScore * 0.8) {
			return -1;
		}
	}


	return maxScoreDirection;
}

static int detectStatefullGesture(float* lastCentersX, float* lastCentersY, int32_t* lastMaximums, int32_t data_processed_max) {
	for (int i = 1; i < OUTPUT_STATE_FILTERING_MEMORY; i++) {
		lastGestures[i - 1] = lastGestures[i];
	}

	lastGestures[OUTPUT_STATE_FILTERING_MEMORY - 1] = detectGesture(lastCentersX, lastCentersY, lastMaximums, data_processed_max);

	int candidate = -1;
	int candidateSequence = 0;

	for (int i = 0; i < OUTPUT_STATE_FILTERING_MEMORY; i++) {
		if (lastGestures[i] != -1) {
			if (candidate == lastGestures[i]) {
				candidateSequence++;
			} else {
				candidate = lastGestures[i];
				candidateSequence = 1;
			}
		} else {
			candidate = -1;
			candidateSequence = 0;
		}
	}

	if (candidate != -1 && isDirectionBlocked[candidate]) {
		candidate = -1;
		candidateSequence = 0;
	}

	if (candidate != -1) {
		int threshold = 12;
		if (candidate == DIRECTION_UP || candidate == DIRECTION_DOWN) {
			threshold = 8;
		}

		if (candidateSequence < threshold) {
			candidate = -1;
		 	candidateSequence = 0;
		 }
	}

	if (candidate != -1) {
		if (candidate == DIRECTION_UP) {
			stuckAtDirection[DIRECTION_UP] = 60;

			isDirectionBlocked[DIRECTION_UP] = 30;
			isDirectionBlocked[DIRECTION_DOWN] = 30;
			isDirectionBlocked[DIRECTION_LEFT] = 20;
			isDirectionBlocked[DIRECTION_RIGHT] = 20;
		}

		if (candidate == DIRECTION_DOWN) {
			stuckAtDirection[DIRECTION_DOWN] = 60;

			isDirectionBlocked[DIRECTION_UP] = 30;
			isDirectionBlocked[DIRECTION_DOWN] = 30;
			isDirectionBlocked[DIRECTION_LEFT] = 20;
			isDirectionBlocked[DIRECTION_RIGHT] = 20;
		}

		if (candidate == DIRECTION_RIGHT) {
			stuckAtDirection[DIRECTION_RIGHT] = 60;

			isDirectionBlocked[DIRECTION_UP] = 20;
			isDirectionBlocked[DIRECTION_DOWN] = 20;
			isDirectionBlocked[DIRECTION_RIGHT] = 30;
			isDirectionBlocked[DIRECTION_LEFT] = 30;
		}

		if (candidate == DIRECTION_LEFT) {
			stuckAtDirection[DIRECTION_LEFT] = 60;

			isDirectionBlocked[DIRECTION_UP] = 20;
			isDirectionBlocked[DIRECTION_DOWN] = 20;
			isDirectionBlocked[DIRECTION_LEFT] = 30;
			isDirectionBlocked[DIRECTION_RIGHT] = 30;
		}
	}

	for (int i = 0; i < 4; i++) {
		if (isDirectionBlocked[i]) {
			isDirectionBlocked[i]--;
		}
		if (stuckAtDirection[i]) {
			candidate = i;
			stuckAtDirection[i]--;
		}
	}

	return candidate;
}

void GestureDetect_Init() {
	for (int i = 0; i < DECTION_FRAME_COUNT; i++) {
		lastCentersX[i] = 0;
		lastCentersY[i] = 0;
		lastMaximums[i] = 0;
	}

	for (int i = 0; i < OUTPUT_STATE_FILTERING_MEMORY; i++) {
		lastGestures[i] = -1;
	}

	for (int i = 0; i < MAX25405_PIXELS; i++) {
		low_pass_memory[i] = 0;
		high_pass_memory[i] = 0;
	}
}

int GestureDetect_AddDataAndGetGesture(int16_t* data) {
	float centerOfMassX = 0;
	float centerOfMassY = 0;

	int32_t data_processed[MAX25405_PIXELS];
	int32_t data_processed_max = -999999;

	for (int i = 1; i < DECTION_FRAME_COUNT; i++) {
		lastCentersX[i - 1] = lastCentersX[i];
		lastCentersY[i - 1] = lastCentersY[i];
		lastMaximums[i - 1] = lastMaximums[i];
	}

    float low_pass_coeff = 0.9;
    float high_pass_coeff = 0.05;

	for (int i = 0; i < MAX25405_PIXELS; i++) {
		float after_low_pass = (1 - low_pass_coeff) * low_pass_memory[i] + low_pass_coeff * data[i];
		float after_high_pass = (1 - high_pass_coeff) * high_pass_memory[i] + high_pass_coeff * after_low_pass;

		low_pass_memory[i] = after_low_pass;
		high_pass_memory[i] = after_high_pass;

		data_processed[i] = (int32_t)(after_low_pass - after_high_pass);

		if (data_processed[i] > data_processed_max) {
			data_processed_max = data_processed[i];
		}
	}

	computeCenterOfMass(data_processed, &centerOfMassX, &centerOfMassY);

	lastCentersX[DECTION_FRAME_COUNT - 1] = centerOfMassX;
	lastCentersY[DECTION_FRAME_COUNT - 1] = centerOfMassY;
	lastMaximums[DECTION_FRAME_COUNT - 1] = data_processed[0];

	for (int i = 0; i < MAX25405_PIXELS; i++) {
		if (data[i] > lastMaximums[DECTION_FRAME_COUNT - 1]) {
			lastMaximums[DECTION_FRAME_COUNT - 1] = data[i];
		}
	}

	return detectStatefullGesture(lastCentersX, lastCentersY, lastMaximums, data_processed_max);
}
