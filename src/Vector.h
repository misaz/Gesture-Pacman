#ifndef __VECTOR_H
#define __VECTOR_H

typedef struct {
	int x;
	int y;
} Vector;

extern const Vector vectorUp;
extern const Vector vectorDown;
extern const Vector vectorLeft;
extern const Vector vectorRight;

Vector Vector_Add(Vector v1, Vector v2);
Vector Vector_Inv(Vector v);
int Vector_Cmp(Vector v1, Vector v2);

#endif