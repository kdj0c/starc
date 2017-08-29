/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include "event.h"

int ntServerInit(void);
int ntClientInit(void);
void ntServerMain(void);

void ntSendPing(void);
void ntSendEvent(float time, void *data, int size, event_e type);
void ntHandleMessage(void);

#endif /* NETWORK_H_ */

