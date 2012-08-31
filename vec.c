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

vec_t vmatrix(vec_t a, vec_t b, float r) {
    vec_t v;
    v.x = a.x + b.x * cos(r) + b.y * sin(r);
    v.y = a.y + b.x * sin(r) - b.y * cos(r);
    return v;
}

vec_t vmatrix1(vec_t a, float r) {
    vec_t v;
    v.x = a.x * cos(r) + a.y * sin(r);
    v.y = a.x * sin(r) - a.y * cos(r);
    return v;
}

vec_t vangle(float len, float r) {
	vec_t v;
	v.x = len * cos(r);
	v.y = len * sin(r);
	return v;
}

float sqdist(vec_t a, vec_t b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dy;
}


void get_pos(float time, traj_t *traj, pos_t *pos) {
    float dt = time - traj->basetime;
    vec_t tmp1;

    switch(traj->type) {
    case t_linear:
        pos->v = traj->base.v;
        pos->p = vadd(traj->base.p, vmul(pos->v, dt));
        pos->dr = traj->man;
        pos->r = traj->base.r + dt * traj->man;
    break;
    case t_linear_acc:
        pos->r = traj->base.r;
        pos->v = vadd(traj->base.v, vangle(dt * traj->thrust, pos->r));
        pos->p = vadd(traj->base.p, vmul(traj->base.v, dt));
        pos->p = vadd(pos->p, vangle(0.5 * dt * dt * traj->thrust, pos->r));
        pos->dr = traj->man;
    break;
    /* p(t) = p0 + t*v0 + t*thrust/man*unitvec(r0 + pi/2) + thrust/(man * man) * (unitvec(r0) - unitvec(r(t))) */
    case t_circle:
        pos->dr = traj->man;
        pos->r = traj->base.r + dt * traj->man;
        pos->v = vadd(traj->base.v, vangle(traj->thrust / traj->man, traj->base.r + M_PI/2.0));
        pos->v = vadd(pos->v, vangle(traj->thrust / traj->man, traj->base.r - M_PI/2.0 + traj->man * dt));
        pos->p = vadd(traj->base.p, vmul(traj->base.v, dt));
        pos->p = vadd(pos->p, vangle(dt * traj->thrust / traj->man, traj->base.r + M_PI/2.0));
        tmp1 = vsub(vangle(1., traj->base.r), vangle(1., pos->r));
        pos->p = vadd(pos->p, vmul(tmp1, traj->thrust / (traj->man * traj->man)));
    break;
    }
}
