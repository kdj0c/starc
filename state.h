/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef STATE_H_
#define STATE_H_

#include "vec.h"
#include "list.h"

typedef struct {
	struct list_head list;
	vec_t pos;
	vec_t vit;
	vec_t dir;
	float t;
	float dt;
	void *priv;
} state_t;

#endif /* MOTHERSHIP_H_ */
