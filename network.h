/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef NETWORK_H_
#define NETWORK_H_

void ntInit(void);
ship_t * ntCreateLocalPlayer(char * type);
void ntSendShip(ship_t * sh);
void ntHandleMessage(void);


#endif /* NETWORK_H_ */
