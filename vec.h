/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef VEC_H_
#define VEC_H_

typedef struct {
	float x;
	float y;
} vec_t;

typedef struct {
	vec_t p; //position
	vec_t v; //speed
	float r; //angle
} pos_t;

vec_t vadd(vec_t a, vec_t b);
vec_t vsub(vec_t a, vec_t b);
vec_t vmul(vec_t a, float b);
vec_t vangle(float len, float r);

#define vecn(x,y) { vec_t v = { .x = x, .y = y } }

static inline vec_t vec(float x, float y) {
	vec_t v;
	v.x = x;
	v.y = y;
	return v;
}

#endif
