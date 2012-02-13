/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef AI_H_
#define AI_H_

#include "list.h"

typedef struct ai_s {
	struct list_head list;
	struct ai_s * next;
	ship_t * ship;
	ship_t * target;
	int state;
} ai_t;

ai_t * aiCreate(ship_t * sh);
void aiThink(void);

#endif /* AI_H_ */
