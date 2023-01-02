#ifndef __FIELD_STATE_H
#define __FIELD_STATE_H

typedef enum {
	FieldState_Empty = 0,
	FieldState_Wall = 1,
	FieldState_Bonus = 2,
	FieldState_Point = 3,
} FieldState;

#endif