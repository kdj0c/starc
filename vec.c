/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>

#include "vec.h"

vec_t vadd(vec_t a, vec_t b) {
	vec_t r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	return r;
}

vec_t vsub(vec_t a, vec_t b) {
	vec_t r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	return r;
}

vec_t vmul(vec_t a, float b) {
	vec_t r;
	r.x = b * a.x;
	r.y = b * a.y;
	return r;
}

vec_t vangle(float len, float r) {
	vec_t v;
	v.x = len * cos(r);
	v.y = len * sin(r);
	return v;
}
