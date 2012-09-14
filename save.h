/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SAVE_H_
#define SAVE_H_

#include "event.h"

void saInit(char *file);
void saSaveEvent(ev_t *ev);
void saReplay(char *file);

#endif /* SAVE_H_ */
