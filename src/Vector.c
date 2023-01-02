#include "Vector.h"

const Vector vectorUp = { .x = 0, .y = -1 };
const Vector vectorDown = { .x = 0, .y = 1 };
const Vector vectorLeft = { .x = -1, .y = 0 };
const Vector vectorRight = { .x = 1, .y = 0 };

Vector Vector_Add(Vector v1, Vector v2) {
	Vector output = { v1.x + v2.x, v1.y + v2.y };
	return output;
}

Vector Vector_Inv(Vector v) {
	Vector output = { v.x * -1, v.y * -1 };
	return output;
}

int Vector_Cmp(Vector v1, Vector v2) {
	return v1.x == v2.x && v1.y == v2.y;
}
