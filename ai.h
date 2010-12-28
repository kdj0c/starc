/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef AI_H_
#define AI_H_

typedef struct {
	ship_t * ship;
	ship_t * target;
} ai_t;

ai_t * aiCreate(ship_t * sh, ship_t * target);
void aiThink(ai_t *ai);

#endif /* AI_H_ */
