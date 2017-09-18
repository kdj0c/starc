/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SERVER_H_
#define SERVER_H_

void seStartServerLocal(void);
void serverMain(float time);
void sePostEventToServer(float time, void *data, int size, event_e type);

#endif
